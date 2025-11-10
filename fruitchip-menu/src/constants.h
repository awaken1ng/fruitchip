#pragma once

#include "tamtypes.h"
#include "gsKit.h"

// region: gui

// static const u64 FG = GS_SETREG_RGBA(0x00,0x00,0x00,0x80); // black
// static const u64 BG = GS_SETREG_RGBA(0xFF,0xFF,0xFF,0x80); // white

static const u64 FG = GS_SETREG_RGBA(0xFF, 0xFF, 0xFF, 0x80); // #ffffff
static const u64 BG = GS_SETREG_RGBA(0x00, 0x00, 0x00, 0x80); // #000000

#define MARGIN_X 4.0f

#define ITEM_H 28.0f
#define ITEM_TEXT_X_START 32.0f
#define ITEM_TEXT_Y_OFFSET 6.0f

#define HEADER_Y_START (ITEM_H * 1)
#define HEADER_H (2.0)

// draw items with 4px gap to fit scrollbar on the side
#define ITEMS_Y_START (ITEM_H * 2)

#define ITEMS_SCROLLBAR_W (2.0)

#define ITEM_Y(i) (ITEMS_Y_START + (ITEM_H * i))
#define ITEM_TEXT_Y(i) (ITEM_Y(i) + ITEM_TEXT_Y_OFFSET)

#define BUTTON_GUIDE_START_Y (ITEM_H * 14)

// endregion: gui

#define BOOT_ITEM_OSDSYS 0
#define BOOT_ITEM_FWFS 1
