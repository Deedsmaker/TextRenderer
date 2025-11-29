#pragma once

#include "freetype/ftlcdfil.h"

FT_Library ft;
FT_Face face;

i32 font_size = 22;

void init_freetype() {
    FT_Init_FreeType(&ft);
    FT_Library_SetLcdFilter(ft, FT_LCD_FILTER_DEFAULT);
    if (FT_New_Face(ft, "../Nunito-Light.ttf", 0, &face) != 0) {
        printf("Font does not contains UNICODE characters?\n");
    }
    FT_Set_Pixel_Sizes(face, 0, font_size);
}

void free_font() {
    // Cleanup
    FT_Done_Face(face);
    // FT_Done_FreeType(ft);
}

static u32 decode_utf8(const char** ptr)
{
    const unsigned char* s = (const unsigned char*)*ptr;
    if (s[0] < 0x80) { *ptr += 1; return s[0]; }
    if ((s[0] & 0xE0) == 0xC0 && (s[1] & 0xC0) == 0x80) { *ptr += 2; return ((s[0]&0x1F)<<6) | (s[1]&0x3F); }
    if ((s[0] & 0xF0) == 0xE0 && (s[1] & 0xC0) == 0x80 && (s[2] & 0xC0) == 0x80) { *ptr += 3; return ((s[0]&0x0F)<<12) | ((s[1]&0x3F)<<6) | (s[2]&0x3F); }
    if ((s[0] & 0xF8) == 0xF0 && (s[1] & 0xC0) == 0x80 && (s[2] & 0xC0) == 0x80 && (s[3] & 0xC0) == 0x80) { *ptr += 4; return ((s[0]&0x07)<<18) | ((s[1]&0x3F)<<12) | ((s[2]&0x3F)<<6) | (s[3]&0x3F); }
    
    *ptr += 1; return 0xFFFD; // � replacement character
}

void render_text_ft(Screen_Buffer* sb, const char *text, int x, int y, u32 color)
{
    int pen_x = x;
    int pen_y = y;

    const char *p = text;
    while (*text) {
        u32 codepoint = decode_utf8(&text);
        
        if (codepoint < 32 && codepoint != '\n' && codepoint != '\t') continue;
        
        FT_UInt glyph_index = FT_Get_Char_Index(face, codepoint);
        if (glyph_index == 0) { 
            glyph_index = FT_Get_Char_Index(face, 0xFFFD);
        }
        
        i32 flags = FT_LOAD_RENDER | FT_LOAD_TARGET_LIGHT | FT_LOAD_FORCE_AUTOHINT;
        
        if (FT_Load_Glyph(face, glyph_index, flags) != 0)
        {
            FT_Load_Glyph(face, FT_Get_Char_Index(face, 0xFFFD), flags); // Fallback.
        }
        
        FT_GlyphSlot g = face->glyph;
        FT_Bitmap*   bmp = &g->bitmap;
        
        // Skip newlines/tabs (do this BEFORE loading glyph!)
        if (codepoint == '\n') {
            pen_y += font_size * 1.4f;
            pen_x = x;
            continue;
        }
        if (codepoint == '\t') {
            pen_x += font_size * 4;
            continue;
        }
        
        int gx = pen_x + g->bitmap_left;
        int gy = pen_y - g->bitmap_top;
        
        for (int py = 0; py < bmp->rows; ++py) {
            int sy = gy + py;
            if (sy < 0 || sy >= sb->height) continue;
        
            for (int px = 0; px < bmp->width; ++px) {
                int sx = gx + px;
                if (sx < 0 || sx >= sb->width) continue;
        
                unsigned char alpha = bmp->buffer[py * bmp->pitch + px];
        
                if (alpha == 0) continue;
        
                u32* dst = &sb->pixels[sy * sb->width + sx];
                u32  bg  = *dst;
        
                int r = ((color >> 16) & 255) * alpha + ((bg >> 16) & 255) * (255 - alpha);
                int g = ((color >>  8) & 255) * alpha + ((bg >>  8) & 255) * (255 - alpha);
                int b = ( color        & 255) * alpha + ( bg        & 255) * (255 - alpha);
        
                *dst = 0xFF000000 | (r/255 << 16) | (g/255 << 8) | (b/255);
            }
        }
        
        pen_x += g->advance.x >> 6;
    }
}

void render_text_lcd(Screen_Buffer* sb, const char *text, int x, int y, u32 color)
{
    int pen_x = x;
    int pen_y = y;

    const char *p = text;
    while (*text) {
        u32 codepoint = decode_utf8(&text);
        
        if (codepoint < 32 && codepoint != '\n' && codepoint != '\t') continue;
        
        FT_UInt glyph_index = FT_Get_Char_Index(face, codepoint);
        if (glyph_index == 0) { 
            glyph_index = FT_Get_Char_Index(face, 0xFFFD);
        }
        
        i32 flags = FT_LOAD_RENDER | FT_LOAD_TARGET_LCD | FT_LOAD_TARGET_LIGHT | FT_LOAD_FORCE_AUTOHINT;
        
        if (FT_Load_Glyph(face, glyph_index, flags) != 0) {
            FT_Load_Glyph(face, 0xFFFD, flags);
        }

        FT_GlyphSlot g = face->glyph;
        FT_Bitmap* bmp = &g->bitmap;

        if (codepoint == '\n') {
            pen_y += font_size * 1.4f;
            pen_x = x;
        }
        if (codepoint == '\t') {
            pen_x += font_size;
        }
        
        if (codepoint == '\n' || codepoint == '\t') continue;

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
