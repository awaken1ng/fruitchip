#pragma once

#include <stdbool.h>

#include "state.h"

bool apps_list_populate(struct state *state);

bool apps_attr_is_configurable(u32 attr);
