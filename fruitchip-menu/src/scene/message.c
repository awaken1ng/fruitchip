
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "libpad.h"

#include "superscene.h"
#include "message.h"
#include "../components/font.h"
#include "../constants.h"

static struct scene_state_message
{
    wchar_t *text;
    float y;
} scene_state;

static void scene_input_handler_message(struct state *state, int input)
{
    if (input & PAD_CIRCLE)
    {
        free(scene_state.text);
        superscene_pop_scene();
        state->repaint = true;
    }
}

static void scene_paint_handler_message(struct state *state)
{
    font_print_centered(state->gs, scene_state.y, 1.0, FG, scene_state.text);
    superscene_clear_button_guide(state);
    state->button_guide.circle = L"Return";
    state->repaint = true;
}

void scene_switch_to_message(struct state *state, wchar_t *text)
{
    scene_state.text = wcsdup(text);
    scene_state.y = (state->gs->Height  / 2.0) - font_text_height(scene_state.text);

    struct scene scene = {
        .input_handler = scene_input_handler_message,
        .paint_handler = scene_paint_handler_message,
    };
    superscene_push_scene(&scene);
    state->repaint = true;
}
