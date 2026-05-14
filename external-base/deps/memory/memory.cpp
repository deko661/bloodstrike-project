#include "memory.h"
#include "../driver/driver.hpp"

std::uint32_t c_memory::find_process_id(const std::wstring &process_name)
{
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
        return 0;

    PROCESSENTRY32W entry{};
    entry.dwSize = sizeof(entry);

    if (!Process32FirstW(snapshot, &entry))
    {
        CloseHandle(snapshot);
        return 0;
    }

    do
    {
        if (_wcsicmp(entry.szExeFile, process_name.c_str()) == 0)
        {
            CloseHandle(snapshot);
            return entry.th32ProcessID;
        }
    } while (Process32NextW(snapshot, &entry));

    CloseHandle(snapshot);
    return 0;
}

std::uint64_t c_memory::find_module_address(const std::wstring &module_name)
{
    if (!process_id)
        return 0;

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, process_id);
    if (snapshot == INVALID_HANDLE_VALUE)
        return 0;

    MODULEENTRY32W entry{};
    entry.dwSize = sizeof(entry);

    if (!Module32FirstW(snapshot, &entry))
    {
        CloseHandle(snapshot);
        return 0;
    }

    do
    {
        if (_wcsicmp(entry.szModule, module_name.c_str()) == 0)
        {
            CloseHandle(snapshot);
            return reinterpret_cast<std::uint64_t>(entry.modBaseAddr);
        }
    } while (Module32NextW(snapshot, &entry));

    CloseHandle(snapshot);
    return 0;
}

bool c_memory::attach_to_process(const std::wstring &process_name)
{
    // Initialize driver
    if (!driver::km->init())
    {
        printf("[-] Failed to initialize driver\n");
        return false;
    }

    // Find process ID
    process_id = find_process_id(process_name);
    if (!process_id)
    {
        printf("[-] Failed to find process: %ws\n", process_name.c_str());
        return false;
    }

    printf("[+] Found process PID: %d\n", process_id);

    // Attach driver to process
    driver::km->attach(process_name.c_str());

    // Get base address from driver
    base_address = driver::km->m_base_addr;
    if (!base_address)
    {
        printf("[-] Failed to get base address\n");
        return false;
    }

    printf("[+] Base address: 0x%llX\n", base_address);
    printf("[+] DTB: 0x%llX\n", driver::km->m_dtb);

    return true;
}

bool c_memory::read_internal(std::uint64_t address, void* buffer, std::uint64_t size)
{
    return driver::km->read(address, buffer, size);
}

bool c_memory::write_internal(std::uint64_t address, const void* buffer, std::uint64_t size)
{
    return driver::km->write(address, buffer, size);
}

std::string c_memory::read_string(std::uint64_t address)
{
    std::string result;
    char buffer[256]{};

    for (int i = 0; i < 255; i += sizeof(buffer) - 1)
    {
        if (!read_internal(address + i, buffer, sizeof(buffer) - 1))
            break;

        buffer[sizeof(buffer) - 1] = '\0';
        result += buffer;

        size_t len = strlen(buffer);
        if (len < sizeof(buffer) - 1)
            break;
    }

    return result;
}

std::unique_ptr<c_memory> memory = std::make_unique<c_memory>();
