#pragma once
/*
 * FREE OBFUSCATION INTEGRATION
 * 
 * Comment utiliser dans ton code:
 * 
 * 1. Strings: remplace "BloodStrike.exe" par OBF("BloodStrike.exe")
 * 2. Fonctions critiques: ajoute JUNK() au début
 * 3. Anti-debug: appelle ANTI_DBG() dans la boucle principale
 * 
 */

#include <windows.h>
#include <cstdint>
#include <cmath>

// ==================== STRING OBFUSCATION ====================
// Compile-time XOR encryption
template<size_t N>
class ObfStr {
    char data[N];
    static constexpr char KEY = 0x42 + N % 8; // Key varie par string
public:
    constexpr ObfStr(const char(&str)[N]) {
        for (size_t i = 0; i < N; i++)
            data[i] = str[i] ^ (KEY + i);
    }
    char* decrypt() {
        for (size_t i = 0; i < N; i++)
            data[i] ^= (KEY + i);
        return data;
    }
};

// Macro principale pour strings
#define OBF(str) obf::ObfStr<sizeof(str)>(str).decrypt()

namespace obf {

// ==================== JUNK CODE ====================
// Insert à des endroits critiques pour brouiller le control flow

#define JUNK_1() \
    do { \
        volatile int __a = GetTickCount() % 100; \
        volatile int __b = __a * 3 + 7; \
        if (__b > 200) { \
            volatile double __c = sqrt((double)__b); \
            (void)__c; \
        } \
    } while(0)

#define JUNK_2() \
    do { \
        SYSTEMTIME __st; \
        GetSystemTime(&__st); \
        volatile DWORD __t = __st.wMilliseconds; \
        if (__t % 2) { \
            volatile DWORD __s = __t ^ 0xDEADBEEF; \
            (void)__s; \
        } \
    } while(0)

#define JUNK_3() \
    do { \
        MEMORYSTATUSEX __mem; \
        __mem.dwLength = sizeof(__mem); \
        GlobalMemoryStatusEx(&__mem); \
        volatile DWORDLONG __m = __mem.ullTotalPhys % 0xFFFF; \
        (void)__m; \
    } while(0)

// Macro générique qui choisit aléatoirement
#define JUNK() do { JUNK_1(); JUNK_2(); } while(0)

// ==================== ANTI-DEBUG ====================
inline bool IsDebugged() {
    // Check RemoteDebugger
    BOOL dbg = FALSE;
    CheckRemoteDebuggerPresent(GetCurrentProcess(), &dbg);
    if (dbg) return true;
    
    // Check PEB BeingDebugged
#ifdef _WIN64
    if (*(BYTE*)(__readgsqword(0x60) + 0x2)) return true;
#else
    if (*(BYTE*)(__readfsdword(0x30) + 0x2)) return true;
#endif
    
    return false;
}

#define ANTI_DBG() \
    do { \
        if (obf::IsDebugged()) { \
            ExitProcess(rand()); \
        } \
    } while(0)

// ==================== TIMING CHECKS ====================
class Timer {
    uint64_t start;
public:
    Timer() : start(__rdtsc()) {}
    bool Check(uint64_t max) {
        return (__rdtsc() - start) < max;
    }
};

#define TIME_START() obf::Timer __timer
#define TIME_CHECK(max_cycles) \
    do { \
        if (!__timer.Check(max_cycles)) { \
            ExitProcess(rand()); \
        } \
    } while(0)

} // namespace obf

// ==================== API HASHING (Dynamic) ====================
// Evite d'avoir les noms d'API en clair dans l'IAT

typedef uint32_t hash_t;

constexpr hash_t HASH_CT(const char* str, hash_t h = 0x811c9dc5) {
    return *str ? HASH_CT(str + 1, (h ^ *str) * 0x01000193) : h;
}

inline hash_t HASH_RT(const char* str) {
    hash_t h = 0x811c9dc5;
    while (*str) {
        h = (h ^ *str++) * 0x01000193;
    }
    return h;
}

// Macro pour API hashing
#define HASH_API(name) constexpr auto api_##name##_hash = HASH_CT(#name)

// ==================== UTILISATION EXEMPLE ====================
/*

DANS TON CODE:

#include "obfuscate_integration.hpp"

void Example() {
    JUNK(); // Insert junk code
    
    // String obfuscation
    const char* procName = OBF("BloodStrike.exe");
    
    // Anti-debug check
    ANTI_DBG();
    
    // Timing check
    TIME_START();
    ReadProcessMemory(...);
    TIME_CHECK(1000000); // Max 1M cycles
}

*/
