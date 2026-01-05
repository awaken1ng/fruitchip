#include <pico/platform/sections.h>

#include <boot_rom/read/idle.h>
#include <boot_rom/read/osdsys.h>
#include <boot_rom/handler.h>

void __time_critical_func(handle_read_idle)(uint8_t r)
{
    switch (r)
    {
        case 0x7F: read_handler = handle_read_find_osdsys_elf; break;
    }
}
