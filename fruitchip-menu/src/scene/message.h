#pragma once

#include <wchar.h>

#include "superscene.h"
#include "../state.h"

void scene_switch_to_message(struct state *state, wchar_t *text);
