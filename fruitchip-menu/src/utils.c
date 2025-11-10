#include <stdarg.h>
#include <wchar.h>

#include <png.h>

#include "utils.h"

int gsKit_texture_png_from_memory(GSGLOBAL *gsGlobal, GSTEXTURE *Texture, void *buf, size_t size)
{
    FILE *File = fmemopen(buf, size, "rb");
    if (File == NULL)
    {
        return -1;
    }

    png_structp png_ptr;
    png_infop info_ptr;
    png_uint_32 width, height;
    png_bytep *row_pointers;

    u32 sig_read = 0;
    png_uint_32 row, i, j;
    int  k=0, bit_depth, color_type, interlace_type;

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, (png_voidp) NULL, NULL, NULL);

    if (!png_ptr)
    {
        printf("PNG Read Struct Init Failed\n");
        fclose(File);
        return -1;
    }

    info_ptr = png_create_info_struct(png_ptr);

    if (!info_ptr)
    {
        printf("PNG Info Struct Init Failed\n");
        fclose(File);
        png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
        return -1;
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        printf("Got PNG Error!\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
        fclose(File);
        return -1;
    }

    png_init_io(png_ptr, File);

    png_set_sig_bytes(png_ptr, sig_read);

    png_read_info(png_ptr, info_ptr);

    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,&interlace_type, NULL, NULL);

    if (bit_depth == 16)
        png_set_strip_16(png_ptr);

    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 4)
        png_set_expand(png_ptr);

    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png_ptr);

    png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);

    png_read_update_info(png_ptr, info_ptr);

    Texture->Width = width;
    Texture->Height = height;

    Texture->VramClut = 0;
    Texture->Clut = NULL;

    if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_RGB_ALPHA)
    {
        int row_bytes = png_get_rowbytes(png_ptr, info_ptr);
        Texture->PSM = GS_PSM_CT32;
        Texture->Mem = memalign(128, gsKit_texture_size_ee(Texture->Width, Texture->Height, Texture->PSM));

        row_pointers = calloc(height, sizeof(png_bytep));

        for (row = 0; row < height; row++) row_pointers[row] = malloc(row_bytes);

        png_read_image(png_ptr, row_pointers);

        struct pixel { u8 r,g,b,a; };
        struct pixel *Pixels = (struct pixel *) Texture->Mem;

        for (i = 0; i < height; i++)
        {
            for (j = 0; j < width; j++)
            {
                Pixels[k].r = row_pointers[i][4*j];
                Pixels[k].g = row_pointers[i][4*j+1];
                Pixels[k].b = row_pointers[i][4*j+2];
                Pixels[k++].a = 128-((int) row_pointers[i][4*j+3] * 128 / 255);
            }
        }

        for(row = 0; row < height; row++) free(row_pointers[row]);

        free(row_pointers);
    }
    else if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_RGB)
    {
        int row_bytes = png_get_rowbytes(png_ptr, info_ptr);
        Texture->PSM = GS_PSM_CT24;
        Texture->Mem = memalign(128, gsKit_texture_size_ee(Texture->Width, Texture->Height, Texture->PSM));

        row_pointers = calloc(height, sizeof(png_bytep));

        for(row = 0; row < height; row++) row_pointers[row] = malloc(row_bytes);

        png_read_image(png_ptr, row_pointers);

        struct pixel3 { u8 r,g,b; };
        struct pixel3 *Pixels = (struct pixel3 *) Texture->Mem;

        for (i=0;i<height;i++)
        {
            for (j=0;j<width;j++)
            {
                Pixels[k].r = row_pointers[i][4*j];
                Pixels[k].g = row_pointers[i][4*j+1];
                Pixels[k++].b = row_pointers[i][4*j+2];
            }
        }

        for(row = 0; row < height; row++) free(row_pointers[row]);

        free(row_pointers);
    }
    else if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_PALETTE)
    {
        png_colorp palette = NULL;
        int num_pallete = 0;
        png_bytep trans = NULL;
        int num_trans = 0;

        png_get_PLTE(png_ptr, info_ptr, &palette, &num_pallete);
        png_get_tRNS(png_ptr, info_ptr, &trans, &num_trans, NULL);
        Texture->ClutPSM = GS_PSM_CT32;

        struct png_clut { u8 r, g, b, a; };

        if (bit_depth == 4)
        {

            int row_bytes = png_get_rowbytes(png_ptr, info_ptr);
            Texture->PSM = GS_PSM_T4;
            Texture->Mem = memalign(128, gsKit_texture_size_ee(Texture->Width, Texture->Height, Texture->PSM));

            row_pointers = calloc(height, sizeof(png_bytep));

            for(row = 0; row < height; row++) row_pointers[row] = malloc(row_bytes);

            png_read_image(png_ptr, row_pointers);

            Texture->Clut = memalign(128, gsKit_texture_size_ee(8, 2, GS_PSM_CT32));
            memset(Texture->Clut, 0, gsKit_texture_size_ee(8, 2, GS_PSM_CT32));

            unsigned char *pixel = (unsigned char *)Texture->Mem;
            struct png_clut *clut = (struct png_clut *)Texture->Clut;

            int k = 0;

            for (int i = num_pallete; i < 16; i++)
                memset(&clut[i], 0, sizeof(clut[i]));


            for (int i = 0; i < num_pallete; i++)
            {
                clut[i].r = palette[i].red;
                clut[i].g = palette[i].green;
                clut[i].b = palette[i].blue;
                clut[i].a = 0x80;
            }

            for (int i = 0; i < num_trans; i++)
                clut[i].a = trans[i] >> 1;

            for (u32 i = 0; i < Texture->Height; i++)
            {
                for (u32 j = 0; j < Texture->Width / 2; j++)
                    memcpy(&pixel[k++], &row_pointers[i][1 * j], 1);
            }

            u32 byte;
            unsigned char *tmpdst = (unsigned char *)Texture->Mem;
            unsigned char *tmpsrc = (unsigned char *)pixel;

            for (byte = 0; byte < gsKit_texture_size_ee(Texture->Width, Texture->Height, Texture->PSM); byte++) tmpdst[byte] = (tmpsrc[byte] << 4) | (tmpsrc[byte] >> 4);

            for(row = 0; row < height; row++) free(row_pointers[row]);

            free(row_pointers);

        }
        else if (bit_depth == 8)
        {
            int row_bytes = png_get_rowbytes(png_ptr, info_ptr);
            Texture->PSM = GS_PSM_T8;
            Texture->Mem = memalign(128, gsKit_texture_size_ee(Texture->Width, Texture->Height, Texture->PSM));

            row_pointers = calloc(height, sizeof(png_bytep));

            for(row = 0; row < height; row++) row_pointers[row] = malloc(row_bytes);

            png_read_image(png_ptr, row_pointers);

            Texture->Clut = memalign(128, gsKit_texture_size_ee(16, 16, GS_PSM_CT32));
            memset(Texture->Clut, 0, gsKit_texture_size_ee(16, 16, GS_PSM_CT32));

            unsigned char *pixel = (unsigned char *)Texture->Mem;
            struct png_clut *clut = (struct png_clut *)Texture->Clut;

            int k = 0;

            for (int i = num_pallete; i < 256; i++)
                memset(&clut[i], 0, sizeof(clut[i]));


            for (int i = 0; i < num_pallete; i++)
            {
                clut[i].r = palette[i].red;
                clut[i].g = palette[i].green;
                clut[i].b = palette[i].blue;
                clut[i].a = 0x80;
            }

            for (int i = 0; i < num_trans; i++)
                clut[i].a = trans[i] >> 1;

            // rotate clut
            for (int i = 0; i < num_pallete; i++)
            {
                if ((i & 0x18) == 8)
                {
                    struct png_clut tmp = clut[i];
                    clut[i] = clut[i + 8];
                    clut[i + 8] = tmp;
                }
            }

            for (u32 i = 0; i < Texture->Height; i++)
            {
                for (u32 j = 0; j < Texture->Width; j++)
                    memcpy(&pixel[k++], &row_pointers[i][1 * j], 1);
            }

            for(row = 0; row < height; row++) free(row_pointers[row]);

            free(row_pointers);
        }
    }
    else
    {
        printf("This texture depth (%i) is not supported yet!\n", png_get_color_type(png_ptr, info_ptr));
        return -1;
    }

    Texture->Filter = GS_FILTER_NEAREST;
    png_read_end(png_ptr, NULL);
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
    fclose(File);

    return gsKit_TexManager_bind(gsGlobal, Texture);
}

int snwprintf(wchar_t *buffer, size_t n, const wchar_t * format, ...)
{
    va_list args;
    va_start(args, format);
    int ret = vswprintf(buffer, n, format, args);
    va_end(args);
    return ret;
}

u64 clock_us(void)
{
    struct timespec ts = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000 +  ts.tv_nsec / 1000;
}

u32 div_ceil_u32(u32 x, u32 y)
{
    return (x % y) ? x / y + 1 : x / y;
}
