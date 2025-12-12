#pragma once

typedef struct Rect {
    i32 x, y;
    i32 w, h;
} Rect;

typedef struct Color {
    u8 r, g, b, a;
} Color;

#include <emmintrin.h>  // SSE2

static inline void draw_pixel_blend_4x(Screen_Buffer* buf, i32 x, i32 y, Color c)
{
    if ((u32)y >= (u32)buf->height) return;

    u32* dst = buf->pixels + y * buf->width + x;

    // Broadcast color
    __m128i fg = _mm_set1_epi32((c.a << 24) | (c.r << 16) | (c.g << 8) | c.b);
    __m128i bg = _mm_loadu_si128((__m128i*)dst);

    if (c.a == 255) {
        _mm_storeu_si128((__m128i*)dst, fg);
        return;
    }
    if (c.a == 0) return;

    // Unpack to 16-bit for accurate blending
    __m128i fg_lo = _mm_unpacklo_epi8(fg, _mm_setzero_si128());
    __m128i fg_hi = _mm_unpackhi_epi8(fg, _mm_setzero_si128());
    __m128i bg_lo = _mm_unpacklo_epi8(bg, _mm_setzero_si128());
    __m128i bg_hi = _mm_unpackhi_epi8(bg, _mm_setzero_si128());

    // alpha = fg.a, 1-alpha = 255-a
    u32 a = c.a;
    __m128i alpha    = _mm_set1_epi16(a);
    __m128i invalpha = _mm_set1_epi16(255 - a);

    // (fg * a + bg * (255-a) + 128) >> 8
    __m128i lo = _mm_add_epi16(_mm_mullo_epi16(fg_lo, alpha),
                               _mm_mullo_epi16(bg_lo, invalpha));
    __m128i hi = _mm_add_epi16(_mm_mullo_epi16(fg_hi, alpha),
                               _mm_mullo_epi16(bg_hi, invalpha));

    lo = _mm_srli_epi16(_mm_add_epi16(lo, _mm_set1_epi16(128)), 8);
    hi = _mm_srli_epi16(_mm_add_epi16(hi, _mm_set1_epi16(128)), 8);

    __m128i result = _mm_packus_epi16(lo, hi);  // back to 8-bit
    _mm_storeu_si128((__m128i*)dst, result);
}

static inline void draw_pixel_blend_solid_4x(Screen_Buffer* buf, i32 x, i32 y, u32 color)
{
    u32* p = buf->pixels + y * buf->width + x;
    __m128i c = _mm_set1_epi32(color);
    _mm_storeu_si128((__m128i*)p, c);
}

static inline void blend_constant_alpha_4x(u32* dst, u32 fg_color, u32 alpha)
{
    __m128i fg = _mm_set1_epi32(fg_color);
    __m128i bg = _mm_loadu_si128((__m128i*)dst);
    __m128i a  = _mm_set1_epi16(alpha);
    __m128i ia = _mm_set1_epi16(255 - alpha);

    __m128i lo = _mm_unpacklo_epi8(bg, _mm_setzero_si128());
    __m128i hi = _mm_unpackhi_epi8(bg, _mm_setzero_si128());
    __m128i f  = _mm_unpacklo_epi8(fg, _mm_setzero_si128());  // reuse fg

    lo = _mm_add_epi16(_mm_mullo_epi16(f, a), _mm_mullo_epi16(lo, ia));
    hi = _mm_add_epi16(_mm_mullo_epi16(f, a), _mm_mullo_epi16(hi, ia));
    lo = _mm_srli_epi16(_mm_add_epi16(lo, _mm_set1_epi16(128)), 8);
    hi = _mm_srli_epi16(_mm_add_epi16(hi, _mm_set1_epi16(128)), 8);

    _mm_storeu_si128((__m128i*)dst, _mm_packus_epi16(lo, hi));
}

inline void draw_pixel(Screen_Buffer* buffer, i32 x, i32 y, u32 color)
{
    buffer->pixels[y * buffer->width + x] = color;
}

static inline void draw_pixel_blend(Screen_Buffer* buffer, i32 x, i32 y, Color color)
{
    if (x < 0 || y < 0 || x >= buffer->width || y >= buffer->height) return;
    if (color.a == 255) {
        buffer->pixels[y * buffer->width + x] = 0xFF000000 | (color.r << 16) | (color.g << 8) | color.b;
        return;
    }
    if (color.a == 0) return;

    u32 bg = buffer->pixels[y * buffer->width + x];
    u32 a = color.a;
                u32 inv_a = 255 - a;

                u32 r_out = ((a * ((color.r) ) + inv_a * ((bg >> 16) & 0xFF)) >> 8) << 16;
                u32 g_out = ((a * ((color.g) ) + inv_a * ((bg >>  8) & 0xFF)) >> 8) <<  8;
                u32 b_out =  (a * ( color.b) + inv_a * ( bg        & 0xFF)) >> 8;

                buffer->pixels[y * buffer->width + x] = 0xFF000000 | r_out | g_out | b_out;
    // u32 rb = bg & 0xFF00FF, g = bg & 0x00FF00;

    // rb += ((color.r - (rb & 0xFF)) * color.a + 128) >> 8;
    // rb &= 0xFF00FF;

    // g += ((color.g - ((g >> 8) & 0xFF)) * color.a + 128) >> 8;
    // g &= 0x00FF00;

    // buffer->pixels[y * buffer->width + x] = 0xFF000000 | rb | g | ((color.b * color.a + (bg & 0xFF) * (255 - color.a) + 128) >> 8);
}

void draw_line_aa(Screen_Buffer *buffer, Vector2_i32 start, Vector2_i32 end, f32 thick, u32 color)
{
    f32 r = thick * 0.5f;
    f32 r_outer = r + 1.0f;

    i32 x0 = start.x, y0 = start.y;
    i32 x1 = end.x,   y1 = end.y;

    i32 dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
    i32 dy = -abs(y1-y0), sy = y0<y1 ? 1 : -1;
    i32 err = dx + dy, e2;

    while (1) {
        for (i32 yy = (i32)(y0 - r_outer); yy <= (i32)(y0 + r_outer); ++yy) {
            if (yy < 0 || yy >= buffer->height) continue;
            for (i32 xx = (i32)(x0 - r_outer); xx <= (i32)(x0 + r_outer); ++xx) {
                if (xx < 0 || xx >= buffer->width) continue;

                f32 dx = xx - x0, dy = yy - y0;
                f32 dist = sqrtf(dx*dx + dy*dy);

                u32 alpha = 0;
                if      (dist <= r)       alpha = 255;
                else if (dist <= r_outer) alpha = (u32)(255 * (r_outer - dist));

                // u32 bg = buffer->pixels[yy * buffer->width + xx];
                // u32 a = alpha;
                // u32 inv_a = 255 - a;

                // u32 r_out = ((a * ((color >> 16) & 0xFF) + inv_a * ((bg >> 16) & 0xFF)) >> 8) << 16;
                // u32 g_out = ((a * ((color >>  8) & 0xFF) + inv_a * ((bg >>  8) & 0xFF)) >> 8) <<  8;
                // u32 b_out =  (a * ( color        & 0xFF) + inv_a * ( bg        & 0xFF)) >> 8;

                // buffer->pixels[yy * buffer->width + xx] = 0xFF000000 | r_out | g_out | b_out;
                draw_pixel_blend_4x(buffer, xx, yy, (Color){255, 255, 255, alpha});
            }
        }

        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void draw_line_1(Screen_Buffer *buffer, Vector2_i32 start, Vector2_i32 end, f32 thick)
{
    if (thick < 1.0f) thick = 1.0f;
    f32 r = thick * 0.5f;

    i32 x0 = start.x, y0 = start.y;
    i32 x1 = end.x,   y1 = end.y;

    i32 dx = abs(x1 - x0);
    i32 dy = abs(y1 - y0);
    i32 sx = x0 < x1 ? 1 : -1;
    i32 sy = y0 < y1 ? 1 : -1;
    i32 err = dx - dy;

    while (1) {
        // draw thick pixel (fast circle fill)
        i32 ir = (i32)(r + 0.5f);
        for (i32 yy = -ir; yy <= ir; ++yy) {
            i32 xx = (i32)sqrtf(r*r - yy*yy + 0.5f);
            for (i32 x = x0 - xx; x <= x0 + xx; ++x) {
                i32 y = y0 + yy;
                if (x >= 0 && x < buffer->width && y >= 0 && y < buffer->height)
                    buffer->pixels[y * buffer->width + x] = 0xFFFFFFFF;
            }
        }

        if (x0 == x1 && y0 == y1) break;

        i32 e2 = 2 * err;
        if (e2 >= -dy) { err -= dy; x0 += sx; }
        if (e2 <=  dx) { err += dx; y0 += sy; }
    }
}

void draw_rect_lines(Screen_Buffer *buffer, Rect rect) {
    i32 end_x = i32_clamp(rect.x + rect.w, 0, buffer->width - 1);
    i32 end_y = i32_clamp(rect.y + rect.h, 0, buffer->height - 1);
    
    for (i32 y = rect.y; y < end_y; y++) {
        for (i32 x = rect.x; x < end_x; x++) {
            
        }
    }
}
