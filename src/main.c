#ifndef UNICODE
    #define UNICODE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <Windows.h>

#include "ft2build.h"
#include FT_FREETYPE_H

#include "CArray.c"

typedef struct {
    i32 width;
    i32 height;
    u32* pixels;
} Screen_Buffer;
DEFINE_ARRAY(Array_i32, i32);

b32 should_run = true;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Global screen buffer
Screen_Buffer screen_buffer = {0};


void draw_pixel(Screen_Buffer* buffer, i32 x, i32 y, u32 color)
{
    if (!buffer || !buffer->pixels) return;
    if (x < 0 || x >= buffer->width || y < 0 || y >= buffer->height) return;
    
    buffer->pixels[y * buffer->width + x] = color;
}

void draw_bitmap(Screen_Buffer *buffer, FT_Bitmap* bitmap, FT_Int x, FT_Int y)
{
  FT_Int  i, j, p, q;
  FT_Int  x_max = x + bitmap->width;
  FT_Int  y_max = y + bitmap->rows;

  /* for simplicity, we assume that `bitmap->pixel_mode' */
  /* is `FT_PIXEL_MODE_GRAY' (i.e., not a bitmap font)   */

  for ( i = x, p = 0; i < x_max; i++, p++ )
  {
    for ( j = y, q = 0; j < y_max; j++, q++ )
    {
      if ( i < 0      || j < 0       ||
           i >= buffer->width || j >= buffer->height )
        continue;
        
        // buffer->pixels[j][i] |= bitmap->buffer[q * bitmap->width + p];
        draw_pixel(buffer, i, j, bitmap->buffer[q * bitmap->width + p]);
    }
  }
}

// Draw the screen buffer to a device context
void DrawScreenBuffer(HDC hdc, Screen_Buffer* buffer, i32 x, i32 y)
{
    if (!buffer || !buffer->pixels) return;
    
    // Create a bitmap from our pixel data
    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = buffer->width;
    bmi.bmiHeader.biHeight = -buffer->height; // Negative for top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    
    // Use SetDIBitsToDevice for direct drawing
    SetDIBitsToDevice(
        hdc,
        x, y,                    // Destination coordinates
        buffer->width,           // Source width
        buffer->height,          // Source height
        0, 0,                    // Source start coordinates
        0,                       // First scan line
        buffer->height,          // Number of scan lines
        buffer->pixels,          // Pixel data
        &bmi,                    // Bitmap info
        DIB_RGB_COLORS           // Color table type
    );
}

void draw_text() {
    // Load font
    FT_Library ft;
    FT_Face face;
    FT_Init_FreeType(&ft);
    FT_New_Face(ft, "../SpaceMono-Regular.ttf", 0, &face);
    FT_Set_Pixel_Sizes(face, 0, 36);  // 48px height
    
    // For each character in your string
    const char* text = "Whereas disregard and ";
    i32 pen_x = 100, pen_y = 200;  // baseline position
        
    for (; *text; text++) {
        FT_Load_Char(face, *text, FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT);
        FT_GlyphSlot glyph = face->glyph;
    
        FT_Bitmap* bmp = &glyph->bitmap;
    
        for (i32 y = 0; y < bmp->rows; y++) {
            for (i32 x = 0; x < bmp->width; x++) {
                i32 px = pen_x + glyph->bitmap_left + x;
                i32 py = pen_y - glyph->bitmap_top + y;
    
                if (px >= 0 && px < screen_buffer.width &&
                    py >= 0 && py < screen_buffer.height) {
                    unsigned char alpha = bmp->buffer[y * bmp->pitch + x];
                    u32* dst = &screen_buffer.pixels[py * screen_buffer.width + px];
                    // Simple alpha blend over black background
                    if (alpha) {
                        // *dst = 0xFFFFFF | (alpha << 24);
                        // Color blending.
                        u32 bg = *dst;
                        u32 fg = 0xFFFFFF;
                        i32 a = alpha;
                        i32 inv = 255 - a;
                        *dst = (((fg & 0xFF) * a + (bg & 0xFF) * inv) >> 8) |
                               (((fg >> 8 & 0xFF) * a + (bg >> 8 & 0xFF) * inv) >> 8) << 8 |
                               (((fg >> 16 & 0xFF) * a + (bg >> 16 & 0xFF) * inv) >> 8) << 16 |
                               a << 24;
                    }
                }
            }
        }
    
        pen_x += glyph->advance.x >> 6;  // advance is in 1/64px
    }
    
    // Cleanup
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

// i32 WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, i32 nCmdShow)
i32 main()
{
    // Register the window class
    const wchar_t CLASS_NAME[] = L"Direct Draw Window";
    
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = 0;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    
    AttachConsole(ATTACH_PARENT_PROCESS); // With that console outputs work like an icecream.
    
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"Direct Drawing Example - Click or Press Keys!",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL, NULL, 0, NULL
    );
    
    ShowWindow(hwnd, 1);
    
    MSG msg = {0};
    while (should_run) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        UpdateWindow(hwnd);
    
        // Initial direct drawing
        HDC hdc = GetDC(hwnd);
        
        // for (i32 i = 0; i < (i32)(screen_buffer.height * 0.5f); i++) {
        //     for (i32 j = 0; j < (i32)(screen_buffer.width * 0.5f); j++) {
        //         screen_buffer.pixels[i * screen_buffer.width + j] = 0xffff0000;
        //     }
        // }
        
        // do_the_thing1(&screen_buffer);
        draw_text();
        
        // Draw our screen buffer
        DrawScreenBuffer(hdc, &screen_buffer, 0, 0);
        
        // Draw some info text
        // SetBkMode(hdc, TRANSPARENT);
        // TextOut(hdc, 10, 10, L"Press R=Random, G=Gradient, C=Clear, Click=Draw", 50);
        
        ReleaseDC(hwnd, hdc);
    }
    
    return 0;
}

void ClearScreenBuffer(Screen_Buffer* buffer, u32 color)
{
    if (!buffer || !buffer->pixels) return;
    
    for (i32 i = 0; i < buffer->width * buffer->height; i++) {
        buffer->pixels[i] = color;
    }
}

void alloc_screen_buffer(Screen_Buffer *buffer, i32 width, i32 height)
{
    if (!buffer) return;
    if (buffer->pixels) {
        free(buffer->pixels);
    }
    
    buffer->width = width;
    buffer->height = height;
    buffer->pixels = (u32*)calloc(1, width * height * sizeof(u32));
}

void draw_random_pixel(Screen_Buffer* buffer)
{
    if (!buffer) return;
    
    for (i32 i = 0; i < 10000; i++) {
        i32 x = rand() % buffer->width;
        i32 y = rand() % buffer->height;
        u32 color = RGB(rand() % 256, rand() % 256, rand() % 256);
        draw_pixel(buffer, x, y, color);
    }
}

void draw_gradient(Screen_Buffer* buffer)
{
    if (!buffer) return;
    
    for (i32 y = 0; y < buffer->height; y++) {
        for (i32 x = 0; x < buffer->width; x++) {
            u8 r = (u8)((x * 255) / buffer->width);
            u8 g = (u8)((y * 255) / buffer->height);
            u8 b = (u8)(((x + y) * 255) / (buffer->width + buffer->height));
            draw_pixel(buffer, x, y, RGB(r, g, b));
        }
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CREATE:
        {
            RECT rect;
            GetClientRect(hwnd, &rect);
            alloc_screen_buffer(&screen_buffer, rect.right, rect.bottom);
            
            draw_gradient(&screen_buffer);
        }
        break;
        
        case WM_SIZE:
        {
            i32 width = LOWORD(lParam);
            i32 height = HIWORD(lParam);
            alloc_screen_buffer(&screen_buffer, width, height);
            
            draw_gradient(&screen_buffer);
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
            for (i32 dy = -20; dy <= 20; dy++) {
                for (i32 dx = -20; dx <= 20; dx++) {
                    if (dx*dx + dy*dy <= 20*20) {
                        i32 px = x + dx;
                        i32 py = y + dy;
                        if (px >= 0 && px < screen_buffer.width && 
                            py >= 0 && py < screen_buffer.height) {
                            draw_pixel(&screen_buffer, px, py, RGB(255, 0, 0));
                        }
                    }
                }
            }
            
            // Force redraw
            InvalidateRect(hwnd, NULL, FALSE);
        }
        break;
        
        case WM_KEYDOWN:
        {
            switch (wParam)
            {
                case 'R':
                    draw_random_pixel(&screen_buffer);
                    break;
                    
                case 'G':
                    draw_gradient(&screen_buffer);
                    break;
                    
                case 'C':
                    ClearScreenBuffer(&screen_buffer, RGB(0, 0, 0));
                    break;
                    
                case 'B':
                    for (i32 frame = 0; frame < 100; frame++) {
                        ClearScreenBuffer(&screen_buffer, RGB(0, 0, 0));
                        
                        i32 ballX = (frame * 10) % screen_buffer.width;
                        i32 ballY = (screen_buffer.height / 2) + 
                                   (i32)(sin(frame * 0.1) * 100);
                        
                        for (i32 dy = -10; dy <= 10; dy++) {
                            for (i32 dx = -10; dx <= 10; dx++) {
                                if (dx*dx + dy*dy <= 10*10) {
                                    i32 px = ballX + dx;
                                    i32 py = ballY + dy;
                                    if (px >= 0 && px < screen_buffer.width && 
                                        py >= 0 && py < screen_buffer.height) {
                                        draw_pixel(&screen_buffer, px, py, RGB(0, 255, 0));
                                    }
                                }
                            }
                        }
                        
                        InvalidateRect(hwnd, NULL, FALSE);
                        UpdateWindow(hwnd);
                        Sleep(16); // ~60 FPS
                    }
                    break;
            }
            
            InvalidateRect(hwnd, NULL, FALSE);
        }
        break;
        
        case WM_DESTROY:
            should_run = false;
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}