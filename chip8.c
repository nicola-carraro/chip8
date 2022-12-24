#include <Windows.h>
#include<stdbool.h>

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, PSTR cmd_line, int cmd_show) {

	const char* class_name = "chip8";

	WNDCLASSA wc;
    memset(&wc, 0, sizeof(wc));

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
            NULL,
            NULL,
            instance,
            NULL
        );

        if (window != 0) {
            ShowWindow(window, cmd_show);

            while(true){
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
