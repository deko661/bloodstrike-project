#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <deps/memory/memory.h>

namespace scanner
{
  inline std::vector<uint8_t> parse_pattern(const std::string& pattern, std::vector<bool>& mask)
  {
    std::vector<uint8_t> bytes;
    mask.clear();

    for (size_t i = 0; i < pattern.size(); i++)
    {
      if (pattern[i] == ' ' || pattern[i] == '?')
        continue;

      if (i + 1 < pattern.size() && pattern[i + 1] == '?')
      {
        bytes.push_back(0);
        mask.push_back(false);
        if (pattern[i] == '?')
          i++;
        else
          i += 2;
      }
      else
      {
        std::string byte_str = pattern.substr(i, 2);
        bytes.push_back(static_cast<uint8_t>(std::stoul(byte_str, nullptr, 16)));
        mask.push_back(true);
        i++;
      }
    }
    return bytes;
  }

  inline uint64_t find_in_buffer(const std::vector<uint8_t>& buffer, const std::vector<uint8_t>& pattern,
                                    const std::vector<bool>& mask)
  {
    if (buffer.size() < pattern.size())
      return 0;

    for (size_t i = 0; i <= buffer.size() - pattern.size(); i++)
    {
      bool found = true;
      for (size_t j = 0; j < pattern.size(); j++)
      {
        if (mask[j] && buffer[i + j] != pattern[j])
        {
          found = false;
          break;
        }
      }
      if (found)
        return i;
    }
    return 0;
  }

  inline std::vector<uint8_t> read_module_section(uint64_t module_base, const char* section_name)
  {
    std::vector<uint8_t> buffer;

    IMAGE_DOS_HEADER dos_header = memory->read<IMAGE_DOS_HEADER>(module_base);
    if (dos_header.e_magic != IMAGE_DOS_SIGNATURE)
      return buffer;

    IMAGE_NT_HEADERS64 nt_headers = memory->read<IMAGE_NT_HEADERS64>(module_base + dos_header.e_lfanew);
    if (nt_headers.Signature != IMAGE_NT_SIGNATURE)
      return buffer;

    IMAGE_SECTION_HEADER section;
    uint64_t section_header = module_base + dos_header.e_lfanew + sizeof(IMAGE_NT_HEADERS64);

    for (int i = 0; i < nt_headers.FileHeader.NumberOfSections; i++)
    {
      section = memory->read<IMAGE_SECTION_HEADER>(section_header + i * sizeof(IMAGE_SECTION_HEADER));
      if (memcmp(section.Name, section_name, strlen(section_name)) == 0)
      {
        buffer.resize(section.Misc.VirtualSize);
        SIZE_T bytes_read = 0;
        ReadProcessMemory(memory->get_process_handle(),
                          reinterpret_cast<LPCVOID>(module_base + section.VirtualAddress),
                          buffer.data(), section.Misc.VirtualSize, &bytes_read);
        buffer.resize(bytes_read);
        break;
      }
    }
    return buffer;
  }

  inline uint64_t resolve_rip(uint64_t address, int offset_idx, int instruction_size)
  {
    int32_t rel_offset = memory->read<int32_t>(address + offset_idx);
    return address + instruction_size + rel_offset;
  }

  inline uint64_t find_pattern(const char* pattern_str, const char* section = ".text")
  {
    std::vector<bool> mask;
    std::vector<uint8_t> pattern = parse_pattern(pattern_str, mask);

    if (pattern.empty())
      return 0;

    uint64_t module_base = memory->get_module_address();
    std::vector<uint8_t> buffer = read_module_section(module_base, section);

    if (buffer.empty())
      return 0;

    uint64_t result = find_in_buffer(buffer, pattern, mask);
    if (result == 0)
      return 0;

    return module_base + result;
  }

  inline uint64_t find_entitylist()
  {
    uint64_t addr = find_pattern("48 8B 0D ? ? ? ? 48 8B D9");
    if (addr != 0)
      return resolve_rip(addr, 3, 7);
    
    addr = find_pattern("48 8B 05 ? ? ? ? 48 8B D9");
    if (addr != 0)
      return resolve_rip(addr, 3, 7);
    
    addr = find_pattern("48 8B D9 48 8D 05 ? ? ? ? 48 8B 0D");
    if (addr != 0)
      return resolve_rip(addr + 11, 3, 7);
    
    return 0;
  }

  inline uint64_t find_clientengine()
  {
    uint64_t addr = find_pattern("48 8B 0D ? ? ? ? E8 ? ? ? ? 48 8B D8");
    if (addr != 0)
      return resolve_rip(addr, 3, 7);
    
    addr = find_pattern("48 8B 05 ? ? ? ? 48 8B D0");
    if (addr != 0)
      return resolve_rip(addr, 3, 7);
    
    addr = find_pattern("49 8B D6 48 8B CF E8 ? ? ? ? 48 83 3D");
    if (addr != 0)
      return resolve_rip(addr + 13, 3, 7);
    
    return 0;
  }

  inline uint64_t find_health_offset()
  {
    uint64_t addr = find_pattern("F3 0F 10 81 ? ? ? ?");
    if (addr != 0)
    {
      uint32_t offset = memory->read<uint32_t>(addr + 4);
      if (offset > 0x100 && offset < 0x800)
        return offset;
    }
    
    addr = find_pattern("F3 0F 10 83 ? ? ? ?");
    if (addr != 0)
    {
      uint32_t offset = memory->read<uint32_t>(addr + 4);
      if (offset > 0x100 && offset < 0x800)
        return offset;
    }
    
    addr = find_pattern("89 81 ? ? ? ?");
    if (addr != 0)
    {
      uint32_t offset = memory->read<uint32_t>(addr + 2);
      if (offset > 0x100 && offset < 0x800)
        return offset;
    }
    
    addr = find_pattern("F3 0F 10 89 ? ? ? ?");
    if (addr != 0)
    {
      uint32_t offset = memory->read<uint32_t>(addr + 4);
      if (offset > 0x100 && offset < 0x800)
        return offset;
    }
    
    return 0;
  }

} // namespace scanner
