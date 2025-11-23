#pragma once

#include <stdbool.h>

#include "state.h"

bool apps_list_populate(struct state *state);

u32 apps_list_push_item(struct state *state, list_item_t item, u32 attr);

void apps_list_remove_item(struct state *state, u32 idx);

bool apps_attr_is_configurable(u32 attr);
