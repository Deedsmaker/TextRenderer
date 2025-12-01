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

#include "CArray.c"
#include "String.c"

typedef struct {
    u32* pixels;
    i32 width;
    i32 height;
} Screen_Buffer;
DEFINE_ARRAY(Array_i32, i32);

typedef struct Vector2_int { 
    i32 x, y;
} Vector2_int;

b32 should_run = true;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Global screen buffer
Screen_Buffer screen_buffer = {0};

#include "text.c"

void draw_pixel(Screen_Buffer* buffer, i32 x, i32 y, u32 color)
{
    if (!buffer || !buffer->pixels) return;
    if (x < 0 || x >= buffer->width || y < 0 || y >= buffer->height) return;
    
    buffer->pixels[y * buffer->width + x] = color;
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

    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);
    // This is the killer line → makes all char* literals real UTF-8
    // _setmode(_fileno(stdout), _O_U8TEXT);

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
    
    init_freetype();
      
    MSG msg = {0};
    while (should_run) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        UpdateWindow(hwnd);
    
        // Initial direct drawing
        HDC hdc = GetDC(hwnd);
        
        render_text_ft(&screen_buffer, "Привет 228\nHello my man normal!", 100, 100, (i32)(0x00FFFFFF)); 
        
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
            // u8 r = (u8)((x * 255) / buffer->width);
            // u8 g = (u8)((y * 255) / buffer->height);
            // u8 b = (u8)(((x + y) * 255) / (buffer->width + buffer->height));
            // draw_pixel(buffer, x, y, RGB(r, g, b));
            draw_pixel(buffer, x, y, 0xFF242731);
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