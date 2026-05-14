#pragma once

using i8 = char;
using i16 = short;
using i32 = int;
using i64 = long long;

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;
using u64 = unsigned long long;

#define ioctl_code_buffered( i ) CTL_CODE( FILE_DEVICE_UNKNOWN , i , METHOD_BUFFERED , FILE_SPECIAL_ACCESS )
#define ioctl_code_neither( i ) CTL_CODE( FILE_DEVICE_UNKNOWN , i , METHOD_NEITHER , FILE_SPECIAL_ACCESS )
#define driver_security_code 0x8B3F9A2C

namespace ioctl
{
    constexpr u32 read_memory = ioctl_code_neither( 0x801 );
    constexpr u32 write_memory = ioctl_code_neither( 0x802 );
    constexpr u32 get_base = ioctl_code_buffered( 0x803 );
    constexpr u32 translate_pml4 = ioctl_code_buffered( 0x804 );
    constexpr u32 get_dtb = ioctl_code_buffered( 0x805 );
    constexpr u32 unload_driver = ioctl_code_buffered( 0x806 );
    constexpr u32 protect_window = ioctl_code_buffered( 0x807 );
    constexpr u32 attach = ioctl_code_buffered( 0x808 );
    constexpr u32 detach = ioctl_code_buffered( 0x809 );
    constexpr u32 resolve = ioctl_code_buffered( 0x80A );
    constexpr u32 eac_call = ioctl_code_buffered( 0x80C );
    constexpr u32 allocate_virtual = ioctl_code_buffered( 0x80D );
    constexpr u32 protect_virtual = ioctl_code_buffered( 0x80E );
    constexpr u32 get_peb = ioctl_code_buffered( 0x80F );
    constexpr u32 mouse_move = ioctl_code_buffered( 0x810 );

    struct mouse_move_request
    {
        i32 dx;
        i32 dy;
    };

    struct read_request
    {
        u32 m_security;
        u32 m_pid;
        u64 m_address;
        u64 m_buffer;
        u64 m_size;
    };

    struct write_request
    {
        u32 m_security;
        u32 m_pid;
        u64 m_address;
        u64 m_buffer;
        u64 m_size;
    };

    struct base_request
    {
        u32 m_pid;
        u64 m_base;
    };

    struct peb_request
    {
        u32 m_pid;
        u32 m_padding;
        u64 m_peb;
    };

    struct protect_window_request
    {
        u64 m_window_handle;
        u32 m_value;
        u32 m_padding;
    };

    enum resolve_kind : u32
    {
        resolve_base = 1,
        resolve_dtb = 2
    };

    struct attach_request
    {
        u32 m_pid;
        u32 m_padding;
        u64 m_base;
        u64 m_dtb;
    };

    struct detach_request
    {
        u32 m_pid;
        u32 m_padding;
    };

    struct resolve_request
    {
        u32 m_pid;
        u32 m_type;
        u64 m_argument;
        u64 m_value;
    };

    struct allocate_virtual_request
    {
        u32 m_pid;
        u32 m_padding;
        u64 m_address;
        u64 m_size;
        u32 m_alloc_type;
        u32 m_protection;
    };

    struct protect_virtual_request
    {
        u32 m_pid;
        u32 m_padding;
        u64 m_address;
        u64 m_size;
        u32 m_protection;
        u32 m_old_protection;
    };

    constexpr u32 eac_max_sig_len = 192;
    constexpr u32 eac_max_args = 6;

    struct eac_call_request
    {
        char m_signature[ eac_max_sig_len ];
        u32 m_argc;
        u32 m_padding;
        u64 m_args[ eac_max_args ];
        u64 m_return;
        u64 m_target;
    };

    namespace pml4
    {
        struct translate_invoke
        {
            u64 virtual_address;
            u64 directory_base;
            u64 physical_address;
        };

        struct dtb_invoke
        {
            u32 pid;
            u64 dtb;
        };
    }
}
