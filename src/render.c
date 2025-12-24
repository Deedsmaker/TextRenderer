#pragma once

#include "Memory_Arena.c"

typedef struct {
    Memory_Arena *arena;
    u32* pixels;
    i32 width;
    i32 height;
} Screen_Buffer;

typedef struct Rect {
    i32 x, y;
    i32 w, h;
} Rect;

typedef struct Color {
    u8 r, g, b, a;
} Color;

static Screen_Buffer *main_buffer = NULL;
static Screen_Buffer *current_buffer = NULL;

inline u32 color_to_u32(Color color) {
    u32 result = color.b | (color.g << 8) | (color.r << 16) | (color.a << 24);
    return result;
}

inline Color u32_to_color(u32 number) {
    Color result = {.r = (number << 16) & 0xFF, .g = (number << 8) & 0xFF, .b = number & 0xFF, .a = (number << 24) & 0xFF};
    return result;
}

Screen_Buffer *get_current_screen_buffer() {
    return current_buffer;
}

inline Screen_Buffer *make_screen_buffer(i32 width, i32 height, Memory_Arena *arena) {
    Screen_Buffer *buffer = alloc(arena, sizeof(Screen_Buffer));
    buffer->pixels = (u32 *)alloc(arena, width * height * sizeof(width));
    buffer->arena = arena;
    buffer->width = width;
    buffer->height = height;
    return buffer;
}

void free_screen_buffer_if_need(Screen_Buffer **buffer) {
    if (!(*buffer)) return;
    
    free_data_in_arena((*buffer)->arena, (*buffer)->pixels);
    free_data_in_arena((*buffer)->arena, *buffer);
}

// This will be called on window create and on window resize, so we will check if it already exists.
void init_main_screen_buffer(i32 width, i32 height) {
    free_screen_buffer_if_need(&main_buffer);

    main_buffer = make_screen_buffer(width, height, HEAP_ALLOCATOR);
    current_buffer = main_buffer;
}   

void begin_drawing() {
    win32_start_drawing();
}

void end_drawing() {
    Screen_Buffer *buffer = get_current_screen_buffer();
    win32_finish_drawing(buffer->pixels, buffer->width, buffer->height);
}

#include <emmintrin.h>  // SSE2

static inline void draw_pixel_blend_4x(i32 x, i32 y, Color c)
{
    if ((u32)y >= (u32)current_buffer->height) return;

    u32* dst = current_buffer->pixels + y * current_buffer->width + x;

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

static inline void draw_pixel_solid_4x(i32 x, i32 y, u32 color) {
    u32* p = current_buffer->pixels + y * current_buffer->width + x;
    __m128i c = _mm_set1_epi32(color);
    _mm_storeu_si128((__m128i*)p, c);
}

static inline void blend_constant_alpha_4x(u32* dst, u32 fg_color, u32 alpha) {
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

// inline void draw_pixel_raw(Screen_Buffer* buffer, i32 x, i32 y, u32 color)
// {
//     buffer->pixels[y * buffer->width + x] = color;
// }

static inline void draw_pixel(i32 x, i32 y, Color color)
{
    // Don't check for buffer bounds just so it would be a little faster.
    
    if (color.a == 0xFF) {
        current_buffer->pixels[y * current_buffer->width + x] = 0xFF000000 | (color.r << 16) | (color.g << 8) | color.b;
        return;
    }
    if (color.a == 0) return;

    u32 bg = current_buffer->pixels[y * current_buffer->width + x];
    u32 a = color.a;
    u32 inv_a = 255 - a;

    u32 r_out = ((a * ((color.r) ) + inv_a * ((bg >> 16) & 0xFF)) >> 8) << 16;
    u32 g_out = ((a * ((color.g) ) + inv_a * ((bg >>  8) & 0xFF)) >> 8) <<  8;
    u32 b_out =  (a * ( color.b) + inv_a * ( bg        & 0xFF)) >> 8;

    current_buffer->pixels[y * current_buffer->width + x] = 0xFF000000 | r_out | g_out | b_out;
}

inline void draw_line(Vector2_i32 start, Vector2_i32 end, f32 thick, u32 color)
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
            if (yy < 0 || yy >= current_buffer->height) continue;
            for (i32 xx = (i32)(x0 - r_outer); xx <= (i32)(x0 + r_outer); ++xx) {
                if (xx < 0 || xx >= current_buffer->width) continue;

                f32 dx = xx - x0, dy = yy - y0;
                f32 dist = sqrtf(dx*dx + dy*dy);

                u32 alpha = 0;
                if      (dist <= r)       alpha = 255;
                else if (dist <= r_outer) alpha = (u32)(255 * (r_outer - dist));

                draw_pixel(xx, yy, (Color){255, 255, 255, alpha});
            }
        }

        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void draw_line_raw(Vector2_i32 start, Vector2_i32 end, f32 thick)
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
                if (x >= 0 && x < current_buffer->width && y >= 0 && y < current_buffer->height)
                    current_buffer->pixels[y * current_buffer->width + x] = 0xFFFFFFFF;
            }
        }

        if (x0 == x1 && y0 == y1) break;

        i32 e2 = 2 * err;
        if (e2 >= -dy) { err -= dy; x0 += sx; }
        if (e2 <=  dx) { err += dx; y0 += sy; }
    }
}

void clear_background(Color color)
{
    u32 num = color_to_u32(color);
    for (i32 i = 0; i < current_buffer->width * current_buffer->height; i++) {
        current_buffer->pixels[i] = num;
    }
}

