#pragma once

#define BACKGROUND_COLOR (Color){0x24, 0x27, 0x31, 0xFF}

void draw_rect_lines(Rect rect, f32 thick) {
    // bool thing = true;
    i32 end_x = i32_clamp(rect.x + rect.w, 0, current_buffer->width - 1);
    i32 end_y = i32_clamp(rect.y + rect.h, 0, current_buffer->height - 1);
    
    for (i32 y = rect.y; y < end_y; y++) {
        for (i32 x = rect.x; x < end_x; x++) {
            
        }
    }
}

