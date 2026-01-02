#pragma once

#include <stdint.h>

extern bool disable_next_osdsys_hook;
extern bool disable_next_iopboot_hook;

void handle_write_disable_next_osdsys_hook(uint8_t w);
