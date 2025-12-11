#include <libpad.h>

#include <components/font.h>
#include <scene/settings/update.h>
#include <scene/superscene.h>
#include <constants.h>

static list_state_t list;
static u32 item_idx_no;
static u32 item_idx_yes;

static void pop_scene(struct state *state)
{
    list_clear(&list);
    superscene_pop_scene();
    state->repaint = true;
}

static void scene_input_handler_file_found(struct state *state, int input)
{
    if (list_handle_input(&list, input))
    {
        state->repaint = true;
    }
    else if (input & PAD_CIRCLE)
    {
        pop_scene(state);
    }
    else if (input & PAD_CROSS)
    {
        if (list.hilite_idx == item_idx_no)
        {
            pop_scene(state);
        }
        else if (list.hilite_idx == item_idx_yes)
        {
            pop_scene(state);
            scene_switch_to_update_checking();
        }
    }
}

static void scene_paint_handler_file_found(struct state *state)
{
    wchar_t *line1 = L"Do not reset the console while update is in progress";
    font_print(state->gs, ITEM_TEXT_X_START, ITEM_TEXT_Y(0), 1, FG, line1);

    wchar_t *line2 = L"If firmware fails to start after update, you'll need to dissamble";
    wchar_t *line3 = L"the console to manually flash working firmware";
    font_print(state->gs, ITEM_TEXT_X_START, ITEM_TEXT_Y(2), 1, FG, line2);
    font_print(state->gs, ITEM_TEXT_X_START, ITEM_TEXT_Y(3), 1, FG, line3);

    wchar_t *line4 = L"Start the update?";
    font_print(state->gs, ITEM_TEXT_X_START, ITEM_TEXT_Y(5), 1, FG, line4);
    list_draw_items(state->gs, &list);

    superscene_clear_button_guide(state);
    state->button_guide.cross = L"Select";
    state->button_guide.circle = L"Return";
}

void scene_switch_to_update_found()
{
    scene_t scene;
    scene_init(&scene);
    scene.input_handler = scene_input_handler_file_found;
    scene.paint_handler = scene_paint_handler_file_found;
    superscene_push_scene(scene);

    array_list_item_init(list.items);
    list.hilite_idx = 0;
    list.start_item_idx = 6;
    list.max_items = MAX_LIST_ITEMS_ON_SCREEN;

    list_item_t item;
    list_item_init(&item);

    item.left_text = wstring_new_static(L"No");
    item.right_text = NULL;
    item_idx_no = list_push_item(&list, item);

    item.left_text = wstring_new_static(L"Yes");
    item.right_text = NULL;
    item_idx_yes = list_push_item(&list, item);
}