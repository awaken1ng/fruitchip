#pragma once

#include "stdbool.h"

#include "tamtypes.h"

typedef void (connect_event_callback_t)(u8 is_mount_event, void *arg);

void masswatch_init();

void masswatch_set_connect_callback(connect_event_callback_t *cb, void *arg);
