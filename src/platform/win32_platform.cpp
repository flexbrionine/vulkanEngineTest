#include <windows.h>

#include "renderer/vk_renderer.cpp"

static bool running = true;

LRESULT CALLBACK platform_window_callback(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg)
    {
    case WM_CLOSE:
        running = false;
        break;
    
    default:
        break;
    }
    return DefWindowProcA(window, msg, wParam, lParam);
    
}

bool platform_create_window(HWND *window) {
    HINSTANCE inst = GetModuleHandleA(0);

    WNDCLASS wc = {};
    wc.lpfnWndProc = platform_window_callback;
    wc.hInstance = inst;
    wc.lpszClassName = "vulkan_engine_class";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClassA(&wc)) {
        MessageBox(0, "Window class is not regitered!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return false;
    }
    
    *window = CreateWindowExA(
        WS_EX_APPWINDOW,
        "vulkan_engine_class",
        "Refractor",
        WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_OVERLAPPED,
        100, 100, 800, 720, 0, 0, inst, 0);

    if (window == 0)
    {
        MessageBoxA(0, "Faild creating window!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return false;
    } 
    
    ShowWindow(*window, SW_SHOW);

    return true;
}

void platform_update_window(HWND window) {
    MSG msg;

    while (PeekMessageA(&msg, window, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

int main() {
    VkContext vkContext = {};

    HWND window = 0;

    if (!platform_create_window(&window)) {
        return -1;
    }
    

    if (!vk_init(&vkContext, &window)) {
        return -1;
    }


    while (running) {
        platform_update_window(window);
        // vk_render(&vkContext);
    }
    
    return 0;
}