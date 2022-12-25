#include <Windows.h>
#include<stdbool.h>
#include <d3d9.h>

#define clear_struct(obj)(memset(&obj, 0, sizeof(obj)))

typedef struct  {
	boolean running;
	LPDIRECT3D9 direct3d;   
	LPDIRECT3DDEVICE9 direct3d_dev;
} State;

State global_state;

LRESULT CALLBACK WindowProc(HWND window, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch (msg) {
	case WM_QUIT: {global_state.running = false; } break;

	}

	DefWindowProcA(window, msg, wparam, lparam);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, PSTR cmd_line, int cmd_show) {

	clear_struct(global_state);
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

			global_state.running = true;
			while (global_state.running)
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
