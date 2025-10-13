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
} ScreenBuffer;
DEFINE_ARRAY(Array_i32, i32);

b32 should_run = true;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Global screen buffer
ScreenBuffer* g_screenBuffer = NULL;

void load_font() {
    FT_Library font_library;
    FT_Init_FreeType(&font_library);
    printf("real\n");
}

// Draw the screen buffer to a device context
void DrawScreenBuffer(HDC hdc, ScreenBuffer* buffer, int x, int y)
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

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    // Register the window class
    const wchar_t CLASS_NAME[] = L"Direct Draw Window";
    
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    
    AttachConsole(ATTACH_PARENT_PROCESS);
    //for printf to work
    freopen("CONOUT$", "w", stdout);
    
    RegisterClass(&wc);

    // Create the window
    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"Direct Drawing Example - Click or Press Keys!",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL, NULL, hInstance, NULL
    );
    
    // 3. Display the window
    ShowWindow(hwnd, nCmdShow);
    
    load_font();
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
        
        for (i32 i = 0; i < (i32)(g_screenBuffer->height * 0.5f); i++) {
            for (i32 j = 0; j < (i32)(g_screenBuffer->width * 0.5f); j++) {
                g_screenBuffer->pixels[i * g_screenBuffer->width + j] = 0xffff0000;
            }
        }
        
        // Draw our screen buffer
        if (g_screenBuffer) {
            DrawScreenBuffer(hdc, g_screenBuffer, 0, 0);
        }
        
        // Draw some info text
        // SetBkMode(hdc, TRANSPARENT);
        // TextOut(hdc, 10, 10, L"Press R=Random, G=Gradient, C=Clear, Click=Draw", 50);
        
        ReleaseDC(hwnd, hdc);
    }
    
    return 0;
}


// Destroy screen buffer
void DestroyScreenBuffer(ScreenBuffer* buffer)
{
    if (buffer) {
        if (buffer->pixels) {
            free(buffer->pixels);
        }
        free(buffer);
    }
}

// Clear screen buffer with specified color
void ClearScreenBuffer(ScreenBuffer* buffer, u32 color)
{
    if (!buffer || !buffer->pixels) return;
    
    for (int i = 0; i < buffer->width * buffer->height; i++) {
        buffer->pixels[i] = color;
    }
}

// Create a screen buffer
ScreenBuffer* CreateScreenBuffer(int width, int height)
{
    ScreenBuffer* buffer = (ScreenBuffer*)malloc(sizeof(ScreenBuffer));
    if (!buffer) return NULL;
    
    buffer->width = width;
    buffer->height = height;
    buffer->pixels = (u32*)malloc(width * height * sizeof(u32));
    
    if (!buffer->pixels) {
        free(buffer);
        return NULL;
    }
    
    // Initialize to black
    ClearScreenBuffer(buffer, RGB(0, 0, 0));
    return buffer;
}

// Set a pixel in the screen buffer
void DrawPixel(ScreenBuffer* buffer, int x, int y, u32 color)
{
    if (!buffer || !buffer->pixels) return;
    if (x < 0 || x >= buffer->width || y < 0 || y >= buffer->height) return;
    
    buffer->pixels[y * buffer->width + x] = color;
}

// Fill buffer with random pixels
void DrawRandomPixels(ScreenBuffer* buffer)
{
    if (!buffer) return;
    
    for (int i = 0; i < 10000; i++) {
        int x = rand() % buffer->width;
        int y = rand() % buffer->height;
        u32 color = RGB(rand() % 256, rand() % 256, rand() % 256);
        DrawPixel(buffer, x, y, color);
    }
}

// Draw a color gradient
void DrawGradient(ScreenBuffer* buffer)
{
    if (!buffer) return;
    
    for (int y = 0; y < buffer->height; y++) {
        for (int x = 0; x < buffer->width; x++) {
            BYTE r = (BYTE)((x * 255) / buffer->width);
            BYTE g = (BYTE)((y * 255) / buffer->height);
            BYTE b = (BYTE)(((x + y) * 255) / (buffer->width + buffer->height));
            DrawPixel(buffer, x, y, RGB(r, g, b));
        }
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CREATE:
        {
            // Create screen buffer when window is created
            RECT rect;
            GetClientRect(hwnd, &rect);
            g_screenBuffer = CreateScreenBuffer(rect.right, rect.bottom);
            
            // Draw initial content
            if (g_screenBuffer) {
                DrawGradient(g_screenBuffer);
            }
        }
        break;
        
        case WM_SIZE:
        {
            // Resize screen buffer when window is resized
            if (g_screenBuffer) {
                DestroyScreenBuffer(g_screenBuffer);
            }
            
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);
            g_screenBuffer = CreateScreenBuffer(width, height);
            
            if (g_screenBuffer) {
                DrawGradient(g_screenBuffer);
                InvalidateRect(hwnd, NULL, FALSE);
            }
        }
        break;
        
        case WM_PAINT:
        {
            
        }
        break;
        
        case WM_LBUTTONDOWN:
        {
            if (!g_screenBuffer) break;
            
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            
            // Draw a circle at click position
            for (int dy = -20; dy <= 20; dy++) {
                for (int dx = -20; dx <= 20; dx++) {
                    if (dx*dx + dy*dy <= 20*20) {
                        int px = x + dx;
                        int py = y + dy;
                        if (px >= 0 && px < g_screenBuffer->width && 
                            py >= 0 && py < g_screenBuffer->height) {
                            DrawPixel(g_screenBuffer, px, py, RGB(255, 0, 0));
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
            if (!g_screenBuffer) break;
            
            switch (wParam)
            {
                case 'R':
                    DrawRandomPixels(g_screenBuffer);
                    break;
                    
                case 'G':
                    DrawGradient(g_screenBuffer);
                    break;
                    
                case 'C':
                    ClearScreenBuffer(g_screenBuffer, RGB(0, 0, 0));
                    break;
                    
                case 'B':
                    // Draw bouncing ball animation
                    for (int frame = 0; frame < 100; frame++) {
                        ClearScreenBuffer(g_screenBuffer, RGB(0, 0, 0));
                        
                        int ballX = (frame * 10) % g_screenBuffer->width;
                        int ballY = (g_screenBuffer->height / 2) + 
                                   (int)(sin(frame * 0.1) * 100);
                        
                        // Draw ball
                        for (int dy = -10; dy <= 10; dy++) {
                            for (int dx = -10; dx <= 10; dx++) {
                                if (dx*dx + dy*dy <= 10*10) {
                                    int px = ballX + dx;
                                    int py = ballY + dy;
                                    if (px >= 0 && px < g_screenBuffer->width && 
                                        py >= 0 && py < g_screenBuffer->height) {
                                        DrawPixel(g_screenBuffer, px, py, RGB(0, 255, 0));
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
            if (g_screenBuffer) {
                DestroyScreenBuffer(g_screenBuffer);
                g_screenBuffer = NULL;
            }
            PostQuitMessage(0);
            
            should_run = false;
            return 0;
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}