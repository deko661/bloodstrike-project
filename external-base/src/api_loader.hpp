#pragma once
/*
 * DYNAMIC API LOADER
 * Charge les APIs sensibles à l'exécution pour éviter les imports statiques
 */

#include <windows.h>
#include <cstdint>

namespace api {

    // Fonctions cachées - pas dans l'IAT
    inline NtReadVirtualMemory_t NtReadVirtualMemory = nullptr;
    inline NtWriteVirtualMemory_t NtWriteVirtualMemory = nullptr;
    inline NtOpenProcess_t NtOpenProcess = nullptr;
    inline NtQueryVirtualMemory_t NtQueryVirtualMemory = nullptr;

    // Hash simple pour résoudre les noms
    constexpr uint32_t hash(const char* str, uint32_t h = 0x811c9dc5) {
        return *str ? hash(str + 1, (h ^ *str) * 0x01000193) : h;
    }

    inline void* GetProcByHash(HMODULE hMod, uint32_t targetHash) {
        auto dos = (PIMAGE_DOS_HEADER)hMod;
        auto nt = (PIMAGE_NT_HEADERS)((uint8_t*)hMod + dos->e_lfanew);
        auto exp = &nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
        auto expDir = (PIMAGE_EXPORT_DIRECTORY)((uint8_t*)hMod + exp->VirtualAddress);
        
        auto names = (uint32_t*)((uint8_t*)hMod + expDir->AddressOfNames);
        auto funcs = (uint32_t*)((uint8_t*)hMod + expDir->AddressOfFunctions);
        auto ords = (uint16_t*)((uint8_t*)hMod + expDir->AddressOfNameOrdinals);
        
        for (uint32_t i = 0; i < expDir->NumberOfNames; i++) {
            const char* name = (const char*)((uint8_t*)hMod + names[i]);
            if (hash(name) == targetHash) {
                return (void*)((uint8_t*)hMod + funcs[ords[i]]);
            }
        }
        return nullptr;
    }

    inline bool Init() {
        HMODULE ntdll = GetModuleHandleA("ntdll.dll");
        if (!ntdll) return false;

        // Résoudre par hash (noms pas visibles dans le binaire)
        NtReadVirtualMemory = (NtReadVirtualMemory_t)GetProcByHash(ntdll, hash("NtReadVirtualMemory"));
        NtWriteVirtualMemory = (NtWriteVirtualMemory_t)GetProcByHash(ntdll, hash("NtWriteVirtualMemory"));
        NtOpenProcess = (NtOpenProcess_t)GetProcByHash(ntdll, hash("NtOpenProcess"));
        NtQueryVirtualMemory = (NtQueryVirtualMemory_t)GetProcByHash(ntdll, hash("NtQueryVirtualMemory"));

        return NtReadVirtualMemory && NtOpenProcess;
    }

} // namespace api

// Redéfinir les fonctions pour utiliser les pointers
#define DYN_READ(addr, buf, size) api::NtReadVirtualMemory(process_handle, (void*)(addr), (buf), (size), nullptr)
#define DYN_WRITE(addr, buf, size) api::NtWriteVirtualMemory(process_handle, (void*)(addr), (void*)(buf), (size), nullptr)
