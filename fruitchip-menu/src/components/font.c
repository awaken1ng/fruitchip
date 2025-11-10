#include <stdio.h>
#include <stdlib.h>

#include "sio.h"

#include "minmax.h"

#include <font/dejavu_sans_mono.h>
#include "../components/font.h"
#include "../utils.h"

const struct BMFont font = DEJAVU_SANS_MONO;

static GSTEXTURE **fontPages;

// Returns pointer to the glyph or NULL if the font doesn't have a glyph for this character
static const BMFontChar *font_get_glyph(wchar_t character)
{
    for (int i = 0; i < font.bucketCount; i++)
    {
        if ((font.buckets[i].startChar <= character) &&
            (font.buckets[i].endChar >= character))
        {
            return &font.buckets[i].chars[character - font.buckets[i].startChar];
        }
    }

    return NULL;
}

// Draws glyph at specified coordinates
static void font_draw_glyph(
    GSGLOBAL *gsGlobal,
    GSTEXTURE **fontPages,
    const BMFontChar *glyph,
    float x,
    float y,
    int z,
    uint64_t color
)
{
    gsKit_prim_sprite_texture(
        gsGlobal,
        fontPages[glyph->page],                 // font page
        x + glyph->xoffset,                        // x1 (destination)
        -0.5f + y + glyph->yoffset,                 // y1
        glyph->x,                                   // u1 (source texture)
        glyph->y,                                   // v1
        x + glyph->xoffset + glyph->width,         // x2 (destination)
        -0.5f + y + glyph->yoffset + glyph->height, // y2
        glyph->x + glyph->width + 1,                // u2 (source texture, without +1 all characters are cut off on real hardware)
        glyph->y + glyph->height + 1,               // v2
        z,
        color
    );
}

int font_init(GSGLOBAL *gsGlobal)
{
    fontPages = calloc(sizeof(GSTEXTURE *), font.pageCount);

    for (int i = 0; i < font.pageCount; i ++)
    {
        fontPages[i] = calloc(sizeof(GSTEXTURE), 1);
        int ret = gsKit_texture_png_from_memory(
            gsGlobal,
            fontPages[i],
            DEJAVU_SANS_MONO_PAGES[i].data,
            DEJAVU_SANS_MONO_PAGES[i].size
        );
        if (ret < 0)
            return ret;
    }

    return 0;
}

void font_free()
{
    if (!fontPages) return;

    for (int i = 0; i < font.pageCount; i ++)
        if (fontPages[i]) free(fontPages[i]);

    free(fontPages);
}

// Gets the line width for the first line in text
float font_text_width(const wchar_t *text)
{
    float lineWidth = 0;
    const BMFontChar *glyph;

    for (int i = 0; text[i] != '\0'; i++)
    {
        if (text[i] == '\n')
            return lineWidth;

        glyph = font_get_glyph(text[i]);
        if (glyph == NULL)
            continue;

        lineWidth += glyph->xadvance;
    }

    return lineWidth;
}

float font_text_height(const wchar_t *text)
{
    float glyph_height = 0;
    const BMFontChar *glyph;

    for (int i = 0; text[i] != '\0'; i++)
    {
        glyph = font_get_glyph(text[i]);
        if (glyph == NULL)
            continue;

        glyph_height = MAX(glyph->height, glyph_height);
    }

    return glyph_height;
}


// Draws the text with specified max dimensions relative to x and y
// Returns the bottom Y coordinate of the last line that can be used to draw the next text
int font_print(GSGLOBAL *gsGlobal, float x, float y, int z, uint64_t color, const wchar_t *text)
{
    int curX = x;
    const BMFontChar *glyph;

    // Set alpha
    gsKit_set_primalpha(gsGlobal, GS_BLEND_BACK2FRONT, 0);
    // gsKit_set_test(gsGlobal, GS_ATEST_OFF);


    int curHeight = 0;
    for (int i = 0; text[i] != '\0'; i++)
    {
        if (text[i] == '\n')
        {
            curX = x;
            curHeight += font.lineHeight;
            continue;
        }

        glyph = font_get_glyph(text[i]);
        if (glyph == NULL)
            continue;

        font_draw_glyph(gsGlobal, fontPages, glyph, curX, y + curHeight, z, color);
        curX += glyph->xadvance;
    }

    gsKit_set_test(gsGlobal, GS_ATEST_ON);
    gsKit_set_primalpha(gsGlobal, GS_SETREG_ALPHA(0, 1, 0, 1, 0), 0);

    return (y + curHeight + font.lineHeight);
}

int font_print_centered(GSGLOBAL *gsGlobal, float y, int z, uint64_t color, const wchar_t *text)
{
    float w = font_text_width(text);
    float x = (gsGlobal->Width - w) / 2;

    return font_print(gsGlobal, x, y, z, color, text);
}

int font_print_aligned_right(GSGLOBAL *gsGlobal, float xoff, float y, int z, uint64_t color, const wchar_t *text)
{
    float w = font_text_width(text);
    float x = gsGlobal->Width - xoff - w;

    return font_print(gsGlobal, x, y, z, color, text);
}

