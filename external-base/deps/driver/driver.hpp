#pragma once

#include <chrono>
#include <cstdio>
#include <cstdarg>
#include <string>

#include <Windows.h>
#include <tlhelp32.h>
#include <winternl.h>

#include "communications.hpp"
#include "../ext/xorstr.hpp"

inline void log_print(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

namespace driver
{
    class c_driver
    {
    public:
        using nt_device_io_control_file_t = NTSTATUS(NTAPI*)(
            HANDLE,
            HANDLE,
            PIO_APC_ROUTINE,
            PVOID,
            PIO_STATUS_BLOCK,
            ULONG,
            PVOID,
            ULONG,
            PVOID,
            ULONG);

        HANDLE m_handle = INVALID_HANDLE_VALUE;
        bool m_kernel_available = false;
        u32 m_pid{ 0 };
        u64 m_base_addr{ 0 };
        u64 m_dtb{ 0 };
        u64 m_peb{ 0 };
        nt_device_io_control_file_t m_nt_device_io_control_file = nullptr;

        static bool nt_success(NTSTATUS status) {
            return status >= 0;
        }

        bool ioctl_native(u32 code, void* in_buf, u32 in_size, void* out_buf, u32 out_size, DWORD* bytes = nullptr) {
            if (!m_nt_device_io_control_file) {
                DWORD ret = 0;
                BOOL ok = DeviceIoControl(m_handle, code, in_buf, in_size, out_buf, out_size, &ret, nullptr);
                if (bytes) *bytes = ret;
                return !!ok;
            }

            IO_STATUS_BLOCK io_status{};
            HANDLE complete_event = CreateEventW(nullptr, TRUE, FALSE, nullptr);
            if (!complete_event) {
                DWORD ret = 0;
                BOOL ok = DeviceIoControl(m_handle, code, in_buf, in_size, out_buf, out_size, &ret, nullptr);
                if (bytes) *bytes = ret;
                return !!ok;
            }

            NTSTATUS status = m_nt_device_io_control_file(
                m_handle,
                complete_event,
                nullptr,
                nullptr,
                &io_status,
                code,
                in_buf,
                in_size,
                out_buf,
                out_size);

            if (status == STATUS_PENDING) {
                WaitForSingleObject(complete_event, INFINITE);
                status = (NTSTATUS)io_status.Status;
            }

            CloseHandle(complete_event);

            if (bytes) *bytes = (DWORD)io_status.Information;
            return nt_success(status);
        }

        bool init() {
            m_handle = CreateFileW(
                xorstr_(L"\\\\.\\{8EE1AF3F-3FBF-4CA9-9232-4B9BE8B8B8B8}"),
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                nullptr,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                nullptr
            );

            m_kernel_available = (m_handle != INVALID_HANDLE_VALUE);

            if (!m_kernel_available) {
                log_print(xorstr_("[-] failed to open driver handle\n"));
                return false;
            }

            log_print(xorstr_("[+] driver handle opened\n"));

            HMODULE ntdll = GetModuleHandleW(xorstr_(L"ntdll.dll"));
            if (!ntdll)
                ntdll = LoadLibraryW(xorstr_(L"ntdll.dll"));

            if (ntdll)
                m_nt_device_io_control_file = (nt_device_io_control_file_t)GetProcAddress(ntdll, xorstr_("NtDeviceIoControlFile"));

            return true;
        }

        void attach(const wchar_t* procname) {
            HANDLE h_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            if (h_snapshot == INVALID_HANDLE_VALUE)
                return;

            PROCESSENTRY32W pe{};
            pe.dwSize = sizeof(pe);

            if (Process32FirstW(h_snapshot, &pe)) {
                do {
                    if (_wcsicmp(procname, pe.szExeFile) == 0) {
                        m_pid = pe.th32ProcessID;
                        break;
                    }
                } while (Process32NextW(h_snapshot, &pe));
            }

            CloseHandle(h_snapshot);

            if (!m_pid) return;

            get_base(m_base_addr);
            if (!m_base_addr) return;

            get_dtb(m_dtb);
        }

        bool get_base(u64& out_base) {
            if (m_kernel_available) {
                ioctl::base_request req{};
                req.m_pid = m_pid;

                DWORD ret = 0;
                BOOL ok = ioctl_native(ioctl::get_base, &req, sizeof(req), &req, sizeof(req), &ret);

                out_base = req.m_base;
                return ok && out_base;
            }

            // fallback: toolhelp
            HANDLE mod_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, m_pid);
            if (mod_snapshot == INVALID_HANDLE_VALUE) {
                out_base = 0;
                return false;
            }

            MODULEENTRY32W me{};
            me.dwSize = sizeof(me);

            bool ok = !!Module32FirstW(mod_snapshot, &me);
            out_base = ok ? (u64)me.modBaseAddr : 0;
            CloseHandle(mod_snapshot);
            return ok && out_base;
        }

        bool get_dtb(u64& out_dtb) {
            if (!m_kernel_available) {
                out_dtb = 0;
                return false;
            }

            ioctl::pml4::dtb_invoke req{};
            req.pid = m_pid;

            DWORD ret = 0;
            BOOL ok = ioctl_native(ioctl::get_dtb, &req, sizeof(req), &req, sizeof(req), &ret);

            out_dtb = req.dtb;
            return ok && out_dtb;
        }

        bool read(u64 address, void* buffer, u64 size) {
            if (!m_kernel_available)
                return false;

            ioctl::read_request req{};
            req.m_security = driver_security_code;
            req.m_pid = m_pid;
            req.m_address = address;
            req.m_buffer = (u64)buffer;
            req.m_size = size;

            DWORD ret = 0;
            return ioctl_native(ioctl::read_memory, &req, sizeof(req), nullptr, 0, &ret);
        }

        bool write(u64 address, const void* buffer, u64 size) {
            if (!m_kernel_available)
                return false;

            ioctl::write_request req{};
            req.m_security = driver_security_code;
            req.m_pid = m_pid;
            req.m_address = address;
            req.m_buffer = (u64)buffer;
            req.m_size = size;

            DWORD ret = 0;
            return ioctl_native(ioctl::write_memory, &req, sizeof(req), nullptr, 0, &ret);
        }

        bool mouse_move(i32 dx, i32 dy) {
            if (!m_kernel_available) return false;
            ioctl::mouse_move_request req{};
            req.dx = dx;
            req.dy = dy;
            DWORD ret = 0;
            return ioctl_native(ioctl::mouse_move, &req, sizeof(req), nullptr, 0, &ret);
        }

        template<typename T>
        bool read(u64 address, T& out) {
            return read(address, &out, sizeof(T));
        }

        template<typename T>
        T read(u64 address) {
            T out{};
            read(address, out);
            return out;
        }

        template<typename T>
        bool write(u64 address, const T& value) {
            return write(address, (void*)&value, sizeof(T));
        }
    };

    inline c_driver* km = new c_driver();
}
