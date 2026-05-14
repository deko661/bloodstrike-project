#pragma once

#include <windows.h>
#include <TlHelp32.h>
#include <string>
#include <memory>
#include <cstdint>

class c_memory
{
public:
    c_memory() = default;
    ~c_memory() = default;

    std::uint32_t find_process_id(const std::wstring &process_name);
    std::uint64_t find_module_address(const std::wstring &module_name);
    bool attach_to_process(const std::wstring &process_name);

    std::string read_string(std::uint64_t address);

    template <typename T>
    T read(std::uint64_t address)
    {
        T buffer{};
        read_internal(address, &buffer, sizeof(T));
        return buffer;
    }

    template <typename T>
    void write(std::uint64_t address, T value)
    {
        write_internal(address, &value, sizeof(T));
    }

    std::string ReadCUtlSymbolLarge(uintptr_t address);

    std::uint32_t get_process_id() const { return process_id; }
    std::uint64_t get_module_address() const { return base_address; }

private:
    bool read_internal(std::uint64_t address, void* buffer, std::uint64_t size);
    bool write_internal(std::uint64_t address, const void* buffer, std::uint64_t size);

    std::uint32_t process_id{};
    std::uint64_t base_address{};
};

extern std::unique_ptr<c_memory> memory;
