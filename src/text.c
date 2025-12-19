#pragma once

#include "freetype/ftlcdfil.h"

FT_Library ft;
FT_Face face;

i32 font_size = 12;

void init_freetype() {
    FT_Init_FreeType(&ft);
    FT_Library_SetLcdFilter(ft, FT_LCD_FILTER_LIGHT);
    if (FT_New_Face(ft, "../Inter-VariableFont_opsz,wght.ttf", 0, &face) != 0) {
        printf("Font does not contains UNICODE characters?\n");
    }
    FT_Set_Pixel_Sizes(face, 0, font_size);
}

void free_font() {
    // Cleanup
    FT_Done_Face(face);
    // FT_Done_FreeType(ft);
}

static inline u32 decode_utf8(const char** ptr)
{
    const u8* s = (const u8*)*ptr;
    if (s[0] < 0x80) { *ptr += 1; return s[0]; }
    if ((s[0] & 0xE0) == 0xC0 && (s[1] & 0xC0) == 0x80) { *ptr += 2; return ((s[0]&0x1F)<<6) | (s[1]&0x3F); }
    if ((s[0] & 0xF0) == 0xE0 && (s[1] & 0xC0) == 0x80 && (s[2] & 0xC0) == 0x80) { *ptr += 3; return ((s[0]&0x0F)<<12) | ((s[1]&0x3F)<<6) | (s[2]&0x3F); }
    if ((s[0] & 0xF8) == 0xF0 && (s[1] & 0xC0) == 0x80 && (s[2] & 0xC0) == 0x80 && (s[3] & 0xC0) == 0x80) { *ptr += 4; return ((s[0]&0x07)<<18) | ((s[1]&0x3F)<<12) | ((s[2]&0x3F)<<6) | (s[3]&0x3F); }
    
    *ptr += 1; return 0xFFFD; // � replacement character
}

static inline u32 blend_grayscale_glyph(u32 bg, u32 color, u8 bitmap_alpha)
{
    if (bitmap_alpha == 0) return bg;
    if (bitmap_alpha == 0xFF) return 0xFF000000 | color;

    unsigned int alpha = bitmap_alpha;

    int r = ((color >> 16) & 0xFF) * alpha + ((bg >> 16) & 0xFF) * (0xFF - alpha);
    int g = ((color >>  8) & 0xFF) * alpha + ((bg >>  8) & 0xFF) * (0xFF - alpha);
    int b = ( color        & 0xFF) * alpha + ( bg        & 0xFF) * (0xFF - alpha);

    return 0xFF000000 | (r/0xFF << 16) | (g/0xFF << 8) | (b/0xFF);
}

Rect render_text_ft(const char *text, int x, int y, u32 color)
{
    Screen_Buffer *buffer = get_current_screen_buffer();

    int pen_x = x;
    int pen_y = y;
    
    Rect rect = {.x = x, .y = y};

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
        
        FT_GlyphSlot glyph = face->glyph;
        FT_Bitmap*   bitmap = &glyph->bitmap;
        
        if (codepoint == '\n') {
            pen_y += font_size * 1.4f;
            pen_x = x;
            continue;
        }
        if (codepoint == '\t') {
            pen_x += font_size * 4;
            continue;
        }
        
        int glyph_x = pen_x + glyph->bitmap_left;
        int glyph_y = pen_y - glyph->bitmap_top;
        
        for (int by = 0; by < bitmap->rows; ++by) {
            int sy = glyph_y + by;
            if (sy < 0 || sy >= buffer->height) continue;
        
            for (int bx = 0; bx < bitmap->width; ++bx) {
                int sx = glyph_x + bx;
                if (sx < 0 || sx >= buffer->width) continue;
        
                u8 bitmap_alpha = bitmap->buffer[by * bitmap->pitch + bx];
        
                if (bitmap_alpha == 0) continue;
        
                u32* dst = &buffer->pixels[sy * buffer->width + sx];
                u32  bg  = *dst;
        
                *dst = blend_grayscale_glyph(bg, color, bitmap_alpha);
            }
        }
        
        pen_x += glyph->advance.x >> 6;
    }
    
    rect.w = pen_x - x;
    rect.h = pen_y - y;
    
    return rect;
}