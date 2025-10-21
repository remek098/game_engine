#include "platform/platform.h"
#include "core/logger.h"
#include "core/input.h"

#include "core/event.h"


#if KPLATFORM_WINDOWS

#include <windows.h>
#include <windowsx.h> // param input extraction
#include <stdlib.h>

#include "renderer/vulkan/vulkan_types.inl"
#include <vulkan/vulkan_win32.h>

typedef struct internal_state {
    HINSTANCE hInstance;
    HWND hWnd;
    VkSurfaceKHR surface;
} internal_state;

// Clock
static f64 clock_frequency;
static LARGE_INTEGER start_time;

LRESULT CALLBACK win32_process_messages(HWND hWnd, u32 msg, WPARAM w_param, LPARAM l_param);

platform::platform() {
    // plat_state = nullptr;
    plat_state = new platform_state();
    plat_state->internal_state = nullptr;
    KDEBUG("Called platform::platform()");
}

platform::~platform() {
    // delete plat_state;
    if (plat_state) {
        if (plat_state->internal_state) {
            std::free(plat_state->internal_state);
            plat_state->internal_state = nullptr;
        }
        delete plat_state;
        plat_state = nullptr;

        KDEBUG("Platform destroyed successfully");
    }
}

b8 platform::startup(const std::string& applicationName, int32_t x, int32_t y, int32_t width, int32_t height) {
    // convert our void* to the format we can work with
    plat_state->internal_state = malloc(sizeof(internal_state));
    // No need to be concerned with malloc here, this code is part of runtime routine, so we don't have to really bother tracking this allocation
    // in our memory subsystem
    if (!plat_state->internal_state) {
        // Handle malloc failure
        return false;
    }
    internal_state* state = (internal_state*)plat_state->internal_state;

    state->hInstance = GetModuleHandleA(0);

    // Setup and register window class.
    HICON icon = LoadIcon(state->hInstance, IDI_APPLICATION);
    WNDCLASSA wc;
    memset(&wc, 0, sizeof(wc));
    wc.style = CS_DBLCLKS; // Get double-clicks
    wc.lpfnWndProc = win32_process_messages;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = state->hInstance;
    wc.hIcon = icon;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszClassName = m_class_name.c_str();

    if(!RegisterClassA(&wc)) {
        MessageBoxA(0, "Window registration failed", "Error", MB_ICONEXCLAMATION | MB_OK);
        return false;
    }

    // Create window
    u32 client_x = x;
    u32 client_y = y;
    u32 client_width = width;
    u32 client_height = height;

    u32 window_x = client_x;
    u32 window_y = client_y;
    u32 window_width = client_width;
    u32 window_height = client_height;

    // WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX
    u32 window_style =  WS_OVERLAPPEDWINDOW;
    u32 window_ex_style = WS_EX_APPWINDOW;

    // Obtain size of border
    RECT border_rect = {0, 0, 0, 0};
    AdjustWindowRectEx(&border_rect, window_style, 0, window_ex_style);

    // In this case, border rectangle is negative
    window_x += border_rect.left;
    window_y += border_rect.top;
    // grow by the size of OS border.
    window_width += border_rect.right - border_rect.left;
    window_height += border_rect.bottom - border_rect.top;

    HWND handle = CreateWindowExA(
        window_ex_style, m_class_name.c_str(), applicationName.c_str(), 
        window_style, window_x, window_y, window_width, window_height,
        0, 0, state->hInstance, 0
    );

    if(handle == 0) {
        MessageBoxA(NULL, "Window creation failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return false;
    }
    else {
        state->hWnd = handle;
    }

    // Show the window
    b32 should_activate = 1;
    i32 show_window_command_flags = should_activate ? SW_SHOW : SW_SHOWNOACTIVATE;
    // can use SW_SHOWMAXIMIZED OR SW_MINIMIZE if we want to have it initially maximized or minimized
    ShowWindow(state->hWnd, show_window_command_flags);
    
    // Clock setup
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    clock_frequency = 1.0/(f64)frequency.QuadPart;
    QueryPerformanceCounter(&start_time);


    KDEBUG("Window initialized successfully.");
    return true;
}

void platform::shutdown() {
    internal_state* state = (internal_state*) plat_state->internal_state;

    if(state->hWnd) {
        DestroyWindow(state->hWnd);
        state->hWnd = nullptr;
    }

    KDEBUG("Windows platform shut down successfully.");
}

b8 platform::pump_messages() {
    MSG message;
    while(PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }
    return true;
}

void* platform::allocate(u64 size, b8 aligned) {
    return malloc(size);
}

void  platform::free(void* block, b8 aligned) {
    std::free(block);
}

void* platform::zero_memory(void* block, u64 size) {
    return memset(block, 0, size);
}

void* platform::copy_memory(void* dest, const void* source, u64 size) {
    return memcpy(dest, source, size);
}

void* platform::set_memory(void* dest, i32  value, u64 size) {
    return memset(dest, value, size);
}

void platform::console_write(const char* message, u8 color) {
    HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    // FATAL, ERROR, WARN, INFO, DEBUG, TRACE
    static u8 levels[6] = {64, 4, 6, 2, 1, 8};
    SetConsoleTextAttribute(console_handle, levels[color]);
    
    OutputDebugStringA(message);
    u64 length = strlen(message);
    LPDWORD number_written = 0;
    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), message, (DWORD)length, number_written, 0);
}

void platform::console_writeError(const char* message, u8 color) {
    HANDLE console_handle = GetStdHandle(STD_ERROR_HANDLE);
    // FATAL, ERROR, WARN, INFO, DEBUG, TRACE
    static u8 levels[6] = {64, 4, 6, 2, 1, 8};
    SetConsoleTextAttribute(console_handle, levels[color]);
    
    OutputDebugStringA(message);
    u64 length = strlen(message);
    LPDWORD number_written = 0;
    WriteConsoleA(GetStdHandle(STD_ERROR_HANDLE), message, (DWORD)length, number_written, 0);
}

f64 platform::get_absolute_time() {
    LARGE_INTEGER now_time;
    QueryPerformanceCounter(&now_time);
    return (f64)now_time.QuadPart * clock_frequency;
}

void platform::sleep(f64 ms) {
    Sleep(ms);
}

// surface creation for vulkan
b8 platform::create_vulkan_surface(struct platform_state* plat_state, struct vulkan_context* context) {
    // cold cast to get HINSTANCE and HWND
    internal_state* state = (internal_state*)(plat_state->internal_state);

    VkWin32SurfaceCreateInfoKHR create_info = {VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
    create_info.hinstance = state->hInstance;
    create_info.hwnd = state->hWnd;
    
    VkResult result = vkCreateWin32SurfaceKHR(context->instance, &create_info, context->allocator, &state->surface);
    if(result != VK_SUCCESS) {
        KFATAL("Vulkan surface creation failed");
        return false;
    }

    context->surface = state->surface;
    return true;
}

void platform::destroy_vulkan_surface(struct vulkan_context* context) {
    vkDestroySurfaceKHR(context->instance, context->surface, context->allocator);
}

LRESULT CALLBACK win32_process_messages(HWND hWnd, u32 msg, WPARAM w_param, LPARAM l_param) {
    switch (msg) {
        case WM_ERASEBKGND:
            // Notify OS that erasing will be handled by the application to prevent flicker.
            return 1;
        case WM_CLOSE: {
            event_context data = {};
            event_system::fire_event(EVENT_CODE_APPLICATION_QUIT, 0, data);
            // TODO: Fire an event for the application to Quit.
            return 0;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_SIZE: {
            // Get the updated size of window
            // RECT r;
            // GetClientRect(hWnd, &r);
            // u32 width = r.right - r.left;
            // u32 height = r.bottom - r.top;

            // TODO: Fire an event for window resize.
        } break;

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP: {
            // key pressed/released
            b8 pressed = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
            keyboard_keys key = (keyboard_keys)w_param;
            
            input_system::process_key(key, pressed); // pass to process input
        } break;

        case WM_MOUSEMOVE: {
            // Mouse move
            i32 x_position = GET_X_LPARAM(l_param);
            i32 y_position = GET_Y_LPARAM(l_param);
            
            input_system::process_mouse_move(x_position, y_position); // pass to process input
        } break;

        case WM_MOUSEWHEEL: {
            i32 z_delta = GET_WHEEL_DELTA_WPARAM(w_param);
            if(z_delta != 0) {
                // Flattten the input to an OS-independent (-1, 1)
                z_delta = (z_delta < 0) ? -1 : 1;
                input_system::process_mouse_wheel(z_delta); // pass to process input
            }
        } break;

        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP: {
            b8 pressed = msg == WM_LBUTTONDOWN || msg == WM_MBUTTONDOWN || msg == WM_RBUTTONDOWN;
            mouse_buttons button = MOUSE_BUTTON_MAX_BUTTONS;
            switch (msg) {
                case WM_LBUTTONDOWN:
                case WM_LBUTTONUP:
                    button = MOUSE_BUTTON_LEFT;
                    break;
            
                case WM_MBUTTONDOWN:
                case WM_MBUTTONUP:
                    button = MOUSE_BUTTON_MIDDLE;
                    break;

                case WM_RBUTTONDOWN:
                case WM_RBUTTONUP:
                    button = MOUSE_BUTTON_RIGHT;
                    break;

                default:
                    break;
            } // end of finding out what mouse button was pressed
            
            // pass over to input subsystem
            if(button != MOUSE_BUTTON_MAX_BUTTONS) {
                input_system::process_button(button, pressed);
            }
        } break;

        default:
            return DefWindowProcA(hWnd, msg, w_param, l_param);
            // break;
    }
    return DefWindowProcA(hWnd, msg, w_param, l_param);
}

#endif // KPLATFORM_WINDOWS
