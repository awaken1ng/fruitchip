#pragma once

#include <stdbool.h>

void input_pad_start();
bool input_pad_is_ready();
int input_pad_poll();
void input_pad_stop();
