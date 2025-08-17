#include "boot_rom_read_handler.h"
#include "dma.h"

// OSDSYS patch that launches stage 2
const uint8_t __not_in_flash("ee_stage1") LOADER_EE_STAGE_1[] = {
    #embed "../loader/ee-stage1/bin/ee-stage1.bin"
};

static_assert(sizeof(LOADER_EE_STAGE_1) <= 0x62C);

void handle_read_idle(uint8_t r);
void handle_read_osdsys_0_48h_seen(uint8_t r);
void handle_read_osdsys_1_8Ah_seen(uint8_t r);
void handle_read_osdsys_2_05h_seen(uint8_t r);
void handle_read_osdsys_3_00h_seen(uint8_t r);
void handle_read_osdsys_4_07h_seen(uint8_t r);
void handle_read_osdsys_5_found(uint8_t r);

void (*read_handler)(uint8_t) = &handle_read_idle;

void __time_critical_func(handle_read_idle)(uint8_t r)
{
    switch (r)
    {
        case 0x48: read_handler = &handle_read_osdsys_0_48h_seen; break;
    }
}

void __time_critical_func(handle_read_osdsys_0_48h_seen)(uint8_t r)
{
    switch (r)
    {
        case 0x8A: read_handler = &handle_read_osdsys_1_8Ah_seen; break;
        default: read_handler = &handle_read_idle; break;
    }
}

void __time_critical_func(handle_read_osdsys_1_8Ah_seen)(uint8_t r)
{
    switch (r)
    {
        case 0x05: read_handler = &handle_read_osdsys_2_05h_seen; break;
        default: read_handler = &handle_read_idle; break;
    }
}

void __time_critical_func(handle_read_osdsys_2_05h_seen)(uint8_t r)
{
    switch (r)
    {
        case 0x00: read_handler = &handle_read_osdsys_3_00h_seen; break;
        default: read_handler = &handle_read_idle; break;
    }
}

void __time_critical_func(handle_read_osdsys_3_00h_seen)(uint8_t r)
{
    switch (r)
    {
        case 0x48: read_handler = &handle_read_osdsys_0_48h_seen; break;
        case 0x07: read_handler = &handle_read_osdsys_4_07h_seen; break;
        default: read_handler = &handle_read_idle; break;
    }
}

void __time_critical_func(handle_read_osdsys_4_07h_seen)(uint8_t r) 
{
    static uint16_t counter = 0;

    if (counter == 486 && r == 0x24)
    {
        // next byte is the injection window
        dma_data_out_start_transfer(LOADER_EE_STAGE_1, sizeof(LOADER_EE_STAGE_1));
        read_handler = &handle_read_idle;
        counter = 0;
        return;
    }
    else if (counter > 486)
    {
        read_handler = &handle_read_idle;
        counter = 0;
        return;
    }

    counter += 1;
}
