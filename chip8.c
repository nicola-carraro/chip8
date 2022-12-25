#include <Windows.h>
#include<stdbool.h>

#define clear_struct(obj)(memset(&obj, 0, sizeof(obj)))

boolean global_running = true;

LRESULT CALLBACK WindowProc(HWND window, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch(msg) {
    case WM_QUIT: global_running = false;

    }

    DefWindowProcA(window, msg, wparam, lparam);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, PSTR cmd_line, int cmd_show) {

	const char* class_name = "chip8";

	WNDCLASSA wc;
    clear_struct(wc);

	wc.lpfnWndProc = DefWindowProcA;
	wc.hInstance = instance;
	wc.lpszClassName = class_name;
    if (RegisterClassA(&wc) != 0) {

        HWND window = CreateWindowExA(
            0,
            class_name,
            "Chip 8",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
            0,
            0,
            instance,
            0
        );

        if (window != 0) {
            ShowWindow(window, cmd_show);
            MSG msg;
            clear_struct(msg);

            while (global_running)
            {
                while (PeekMessage(&msg, window, 0, 0, PM_REMOVE)) {
                    TranslateMessage(&msg);
                    DispatchMessageA(&msg);
                }
            }
         
        }
        else {
            OutputDebugStringA("Could not open window");
        }

    }
    else {
        OutputDebugStringA("Could not register window class");
    }
}
