#pragma once
#include <windows.h>
#include <cstdint>
#include <random>

namespace obf {

    template<size_t N>
    class XorString {
        char data[N];
        static constexpr char KEY = 0x55;
    public:
        constexpr XorString(const char(&str)[N]) {
            for (size_t i = 0; i < N; i++)
                data[i] = str[i] ^ KEY;
        }
        char* get() {
            for (size_t i = 0; i < N; i++)
                data[i] ^= KEY;
            return data;
        }
    };

    #define XOR(str) obf::XorString<sizeof(str)>(str).get()

    constexpr uint32_t HASH_CT(const char* str, uint32_t h = 0) {
        return *str ? HASH_CT(str + 1, (h >> 13) | (h << 19)) + *str : h;
    }

    inline uint32_t HASH_RT(const char* str) {
        uint32_t h = 0;
        while (*str) {
            h = ((h >> 13) | (h << 19)) + *str++;
        }
        return h;
    }

    #define HASH(str) constexpr auto str##_hash = obf::HASH_CT(str)

    #define JUNK_CODE() \
        do { \
            volatile int _j1 = rand() % 1000; \
            volatile int _j2 = _j1 * 42; \
            if (_j1 > 500) { \
                volatile double _j3 = sqrt((double)_j2); \
                (void)_j3; \
            } \
        } while(0)

    #define JUNK_CODE_2() \
        do { \
            volatile uint64_t _j4 = __rdtsc(); \
            volatile uint64_t _j5 = _j4 ^ 0xDEADBEEF; \
            if (_j5 < _j4) { \
                volatile int _j6 = (int)(_j5 % 100); \
                (void)_j6; \
            } \
        } while(0)

    #define JUNK_CODE_3() \
        do { \
            SYSTEMTIME _st; \
            GetSystemTime(&_st); \
            volatile int _j7 = _st.wSecond + _st.wMilliseconds; \
            if (_j7 % 2 == 0) { \
                volatile int _j8 = _j7 * 3; \
                (void)_j8; \
            } \
        } while(0)

    inline bool IsDebugged() {
        BOOL debugger = FALSE;
        CheckRemoteDebuggerPresent(GetCurrentProcess(), &debugger);
        return debugger;
    }

    inline void DebugCheck() {
        if (IsDebugged()) {
            ExitProcess(0);
        }
    }

    inline void HidePEHeaders() {
        // Erase PE header from memory
        DWORD oldProtect;
        VirtualProtect((PVOID)GetModuleHandle(NULL), 0x1000, PAGE_EXECUTE_READWRITE, &oldProtect);
        memset((PVOID)GetModuleHandle(NULL), 0, 0x1000);
    }

    // ==================== TIMING CHECKS (Anti-Timing) ====================
    #define TIME_CHECK_START() auto _tstart = __rdtsc()
    #define TIME_CHECK_END(max) \
        do { \
            auto _tend = __rdtsc(); \
            if ((_tend - _tstart) > (max)) { \
                ExitProcess(0); \
            } \
        } while(0)

}
