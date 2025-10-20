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
    int width;
    int height;
    u32* pixels;
} Screen_Buffer;
DEFINE_ARRAY(Array_i32, i32);

b32 should_run = true;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Global screen buffer
Screen_Buffer screen_buffer = {0};

void load_font() {
    FT_Library font_library;
    FT_Init_FreeType(&font_library);
    printf("real\n");
}

// Draw the screen buffer to a device context
void DrawScreenBuffer(HDC hdc, Screen_Buffer* buffer, int x, int y)
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

// int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
int main()
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

    // Create the window
    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"Direct Drawing Example - Click or Press Keys!",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL, NULL, 0, NULL
    );
    
    // 3. Display the window
    ShowWindow(hwnd, 1);
    
    // load_font();
    printf("SFD\n");
    
    // 4. Run the message loop
    MSG msg = {0};
    while (should_run) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        UpdateWindow(hwnd);
    
        // Initial direct drawing
        HDC hdc = GetDC(hwnd);
        
        for (i32 i = 0; i < (i32)(screen_buffer.height * 0.5f); i++) {
            for (i32 j = 0; j < (i32)(screen_buffer.width * 0.5f); j++) {
                screen_buffer.pixels[i * screen_buffer.width + j] = 0xffff0000;
            }
        }
        
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
    
    for (int i = 0; i < buffer->width * buffer->height; i++) {
        buffer->pixels[i] = color;
    }
}

void alloc_screen_buffer(Screen_Buffer *buffer, int width, int height)
{
    if (!buffer) return;
    if (buffer->pixels) {
        free(buffer->pixels);
    }
    
    buffer->width = width;
    buffer->height = height;
    buffer->pixels = (u32*)calloc(1, width * height * sizeof(u32));
}

void draw_pixel(Screen_Buffer* buffer, int x, int y, u32 color)
{
    if (!buffer || !buffer->pixels) return;
    if (x < 0 || x >= buffer->width || y < 0 || y >= buffer->height) return;
    
    buffer->pixels[y * buffer->width + x] = color;
}

void draw_random_pixel(Screen_Buffer* buffer)
{
    if (!buffer) return;
    
    for (int i = 0; i < 10000; i++) {
        int x = rand() % buffer->width;
        int y = rand() % buffer->height;
        u32 color = RGB(rand() % 256, rand() % 256, rand() % 256);
        draw_pixel(buffer, x, y, color);
    }
}

void draw_gradient(Screen_Buffer* buffer)
{
    if (!buffer) return;
    
    for (int y = 0; y < buffer->height; y++) {
        for (int x = 0; x < buffer->width; x++) {
            BYTE r = (BYTE)((x * 255) / buffer->width);
            BYTE g = (BYTE)((y * 255) / buffer->height);
            BYTE b = (BYTE)(((x + y) * 255) / (buffer->width + buffer->height));
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
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);
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
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            
            // Draw a circle at click position
            for (int dy = -20; dy <= 20; dy++) {
                for (int dx = -20; dx <= 20; dx++) {
                    if (dx*dx + dy*dy <= 20*20) {
                        int px = x + dx;
                        int py = y + dy;
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
                    for (int frame = 0; frame < 100; frame++) {
                        ClearScreenBuffer(&screen_buffer, RGB(0, 0, 0));
                        
                        int ballX = (frame * 10) % screen_buffer.width;
                        int ballY = (screen_buffer.height / 2) + 
                                   (int)(sin(frame * 0.1) * 100);
                        
                        for (int dy = -10; dy <= 10; dy++) {
                            for (int dx = -10; dx <= 10; dx++) {
                                if (dx*dx + dy*dy <= 10*10) {
                                    int px = ballX + dx;
                                    int py = ballY + dy;
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