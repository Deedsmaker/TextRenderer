#pragma once

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


Screen_Buffer *get_current_screen_buffer();
inline Screen_Buffer *make_screen_buffer(i32 width, i32 height, Memory_Arena *arena);
void free_screen_buffer_if_need(Screen_Buffer **buffer);
void init_main_screen_buffer(i32 width, i32 height);
inline void draw_line(Vector2_i32 start, Vector2_i32 end, f32 thick, u32 color);
void draw_line_raw(Vector2_i32 start, Vector2_i32 end, f32 thick);
void draw_rect_lines(Rect rect);
void draw_gradient();
void clear_screen_buffer(u32 color);


