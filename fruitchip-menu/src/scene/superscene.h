#pragma once

#include "../state.h"

struct scene {
    void (*input_handler)(struct state *state, int input);
    void (*paint_handler)(struct state *state);
};

struct superscene {
    struct scene **scenes;
    u32 scenes_len;
    u64 last_paint;
};

void scene_input_handler_superscene(struct state *state, int input);
void scene_paint_handler_superscene(struct state *state);
void superscene_paint_after_vsync(struct state *state);
void superscene_push_scene(struct scene *scene);
void superscene_pop_scene();
void superscene_clear_button_guide(struct state *state);

