#pragma once

FT_Library ft;
FT_Face face;

i32 font_size = 24;

void init_freetype() {
    FT_Init_FreeType(&ft);
    FT_New_Face(ft, "../SpaceMono-Regular.ttf", 0, &face);
    FT_Set_Pixel_Sizes(face, 0, font_size);
}

void free_font() {
    // Cleanup
    FT_Done_Face(face);
    // FT_Done_FreeType(ft);
}

void render_text_ft(Screen_Buffer* sb, const char *text, int x, int y, u32 color)
{
    int pen_x = x;
    int pen_y = y;

    for (; *text; ++text) {
        if (FT_Load_Char(face, *text, FT_LOAD_RENDER | FT_LOAD_TARGET_LCD | FT_LOAD_TARGET_LIGHT | FT_LOAD_FORCE_AUTOHINT) != 0)
            continue;

        FT_GlyphSlot g = face->glyph;
        FT_Bitmap* bmp = &g->bitmap;

        int gx = pen_x + g->bitmap_left;
        int gy = pen_y - g->bitmap_top;

        for (int py = 0; py < bmp->rows; ++py) {
            int sy = gy + py;
            if (sy < 0 || sy >= sb->height) continue;

            for (int px = 0; px < bmp->width / 3; ++px) {
                int sx = gx + px;
                if (sx < 0 || sx >= sb->width) continue;

                unsigned char* src = bmp->buffer + py * bmp->pitch + px * 3;
                int r = src[0];
                int g = src[1];
                int b = src[2];

                if (r == 0 && g == 0 && b == 0) continue;

                u32* dst = &sb->pixels[sy * sb->width + sx];
                u32 bg = *dst;

                int fg_r = (color >> 16) & 0xFF;
                int fg_g = (color >>  8) & 0xFF;
                int fg_b = (color      ) & 0xFF;

                int out_r = (fg_r * r + ((bg >> 16) & 0xFF) * (255 - r)) / 255;
                int out_g = (fg_g * g + ((bg >>  8) & 0xFF) * (255 - g)) / 255;
                int out_b = (fg_b * b + ((bg      ) & 0xFF) * (255 - b)) / 255;

                *dst = 0xFF000000 | (out_r << 16) | (out_g << 8) | out_b;
            }
        }

        pen_x += g->advance.x >> 6;
    }
}
