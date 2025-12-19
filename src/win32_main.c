#pragma once

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

typedef struct Screen_Buffer;

HWND main_window = {0};
HDC main_drawing_context = {0};

HWND win32_init_window() {
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);

    // Register the window class
    const wchar_t CLASS_NAME[] = L"Real window class";
    
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
        L"Real Note Taker",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL, NULL, 0, NULL
    );
    
    ShowWindow(hwnd, 1);
    
    main_window = hwnd;
    
    return hwnd;
}

// Draw the screen buffer to a device context
void win32_draw_screen_buffer(Screen_Buffer* buffer, i32 x, i32 y)
{
    if (!buffer || !buffer->pixels) return;
    
    // Creating a bitmap from our pixel data.
    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = buffer->width;
    bmi.bmiHeader.biHeight = -buffer->height; // Negative for top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    
    // Use SetDIBitsToDevice for direct drawing
    SetDIBitsToDevice(
        main_drawing_context,
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

inline HDC win32_start_drawing() { 
    main_drawing_context = GetDC(main_window);
    return main_drawing_context;
}

inline void win32_finish_drawing(Screen_Buffer *screen_buffer) { 
    win32_draw_screen_buffer(screen_buffer, 0, 0);
    ReleaseDC(main_window, main_drawing_context);
    UpdateWindow(main_window);
    
    InvalidateRect(main_window, NULL, FALSE);
}

