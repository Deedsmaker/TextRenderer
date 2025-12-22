#ifndef UNICODE
    #define UNICODE
#endif

#if defined(_MSC_VER)
    // This is the ONLY combo that actually works in 2025
    // #pragma execution_character_set("utf-8")
    // Force Windows to use UTF-8 codepage system-wide for this process
    #include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <Windows.h>

#include "ft2build.h"
#include FT_FREETYPE_H

#include "my_defines.h"
#include "CMath.c"
#include "CArray.c"
#include "String.c"

#include "win32_main.c"

#include "render.c"

inline b32 compare_i32(i32 a, i32 b) { return a == b; }
DEFINE_ARRAY(Array_i32, i32, compare_i32);

b32 should_run = true;

String_Builder input_text = {0};

// Project includes.
#include "draw.c"
#include "text.c"

static inline f32 smoothstep(f32 e0, f32 e1, f32 x) {
    x = x < e0 ? 0 : (x > e1 ? 1 : (x - e0)/(e1 - e0));
    
    return x * x * (3 - 2 * x);
}

void init() {
    
}

void update() {
    
}

void draw() {
    
}

// i32 WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, i32 nCmdShow)
i32 main()
{
    init_arena(temp, Megabytes(16));
    
    win32_init_window();
    
    init_freetype();
    
    MSG msg = {0};
    while (should_run) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    
        begin_drawing();
        
        clear_background(BACKGROUND_COLOR);
        
        render_text_ft("Привет 1234567890\nHello my man normal!", 100, 100, (i32)(0x00EAC38F)); 
        
        if (input_text.data) {
            render_text_ft(input_text.data, 100, 200, (i32)(0x00EAC38F)); 
        }
        
        draw_line_raw((Vector2_i32){30, 80}, (Vector2_i32){1500, 750}, 1);
        draw_line((Vector2_i32){30, 30}, (Vector2_i32){1500, 700}, 1, 0xFFFFFFFF);
        
        end_drawing();
    }
    
    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CHAR:
        {
            WCHAR wide_char = (WCHAR)wParam;
    
            // Convert UTF-16 to UTF-8
            int buffer_size = WideCharToMultiByte(CP_UTF8, 0, &wide_char, 1, NULL, 0, NULL, NULL);
            if (buffer_size > 0 && buffer_size <= 4)
            {
                static char buf[5];
                // std::vector<char> utf8Buffer(buffer_size);
                WideCharToMultiByte(CP_UTF8, 0, &wide_char, 1, buf, buffer_size, NULL, NULL);
                
                builder_append_str(&input_text, buf);
            } else if (buffer_size > 4) {
                printf("WATAHEWLL\n");
            }
        } break;
        case WM_CREATE:
        {
            RECT rect;
            GetClientRect(hwnd, &rect);
            init_main_screen_buffer(rect.right, rect.bottom);
            
            clear_background(BACKGROUND_COLOR);
        }
        break;
        
        case WM_SIZE:
        {
            i32 width = LOWORD(lParam);
            i32 height = HIWORD(lParam);
            init_main_screen_buffer(width, height);
            
            clear_background(BACKGROUND_COLOR);
            InvalidateRect(hwnd, NULL, FALSE);
        }
        break;
        
        case WM_PAINT:
        {
            
        }
        break;
        
        case WM_LBUTTONDOWN: 
        {
            i32 x = LOWORD(lParam);
            i32 y = HIWORD(lParam);
            
            // Draw a circle at click position
            // for (i32 dy = -20; dy <= 20; dy++) {
            //     for (i32 dx = -20; dx <= 20; dx++) {
            //         if (dx*dx + dy*dy <= 20*20) {
            //             i32 px = x + dx;
            //             i32 py = y + dy;
            //             if (px >= 0 && px < screen_buffer.width && 
            //                 py >= 0 && py < screen_buffer.height) {
            //                 draw_pixel(&screen_buffer, px, py, RGB(255, 0, 0));
            //             }
            //         }
            //     }
            // }
            
            // Force redraw
            InvalidateRect(hwnd, NULL, FALSE);
        }
        break;
        
        case WM_KEYDOWN:
        {
            switch (wParam)
            {
                case VK_RETURN:
                    builder_append_char(&input_text, '\n');
                break;
                case VK_BACK:
                break;
            }
        }
        break;
        
        case WM_DESTROY:
            should_run = false;
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

