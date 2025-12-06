
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "libpad.h"

#include "components/font.h"
#include "scene/superscene.h"
#include "scene/message.h"
#include "constants.h"

typedef struct message_state {
    wchar_t *text;
    float y;
} message_state_t;

void message_state_init(message_state_t *obj)
{
    obj->text = NULL;
    obj->y = 0;
}

void message_state_copy(message_state_t *obj, const message_state_t *src)
{
    message_state_init(obj);

    if (src->text)
        obj->text = wcsdup(src->text);

    obj->y = src->y;
}

void message_state_free(message_state_t *obj)
{
    free(obj->text);
}

ARRAY_DEF(
    array_message_state,
    message_state_t,
    (
        INIT(API_2(message_state_init)),
        INIT_SET(API_6(message_state_copy)),
        SET(API_6(message_state_copy)),
        CLEAR(API_2(message_state_free)),
    )
)

static array_message_state_t scene_state;

static void scene_input_handler_message(struct state *state, int input)
{
    if (input & PAD_CIRCLE)
    {
        array_message_state_pop_back(NULL, scene_state);
        superscene_pop_scene();
        state->repaint = true;
    }
}

static void scene_paint_handler_message(struct state *state)
{
    message_state_t *msg_state = array_message_state_back(scene_state);
    if (!msg_state)
        return;

    font_print_centered(state->gs, msg_state->y, 1.0, FG, msg_state->text);
    superscene_clear_button_guide(state);
    state->button_guide.circle = L"Return";
    state->repaint = true;
}

void scene_switch_to_message(struct state *state, wchar_t *text)
{
    message_state_t msg_state = {
        .text = wcsdup(text),
        .y = (state->gs->Height  / 2.0) - font_text_height(text)
    };

    array_message_state_push_back(scene_state, msg_state);

    struct scene scene = {
        .input_handler = scene_input_handler_message,
        .paint_handler = scene_paint_handler_message,
    };
    superscene_push_scene(&scene);
    state->repaint = true;
}
