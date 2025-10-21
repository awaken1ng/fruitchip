#include "handler.h"
#include <boot_rom/read/idle.h>
#include <boot_rom/write/idle.h>

uint32_t cmd_byte_counter;

void (*read_handler)(uint8_t) = handle_read_idle;

void (*write_handler)(uint8_t) = handle_write_idle;
