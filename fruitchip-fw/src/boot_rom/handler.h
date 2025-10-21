#pragma once

#include <stdint.h>

extern uint32_t cmd_byte_counter;

#define GET_BYTE(v, idx) ((unsigned char)(v >> (idx * 8)))

extern void (*read_handler)(uint8_t);

extern void (*write_handler)(uint8_t);
