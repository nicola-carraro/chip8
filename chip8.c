#include <Windows.h>
#include<stdbool.h>
#include <d3d9.h>

#define clear_struct(obj)(memset(&obj, 0, sizeof(obj)))

typedef struct  {
	boolean running;
	LPDIRECT3D9 d3d;   
	LPDIRECT3DDEVICE9 d3d_dev;
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
			global_state.d3d = Direct3DCreate9(DIRECT3D_VERSION);

			if (global_state.d3d != 0)
			{
				D3DPRESENT_PARAMETERS d3dpp;
				clear_struct(d3dpp);
				d3dpp.Windowed = true;
				d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
				d3dpp.hDeviceWindow = window;
				HRESULT device_created = IDirect3D9_CreateDevice(
					global_state.d3d, 
					D3DADAPTER_DEFAULT,
					D3DDEVTYPE_HAL,
					window,
					D3DCREATE_SOFTWARE_VERTEXPROCESSING,
					&d3dpp,
					&global_state.d3d_dev);

				if (device_created == D3D_OK){
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

						HRESULT cleared = IDirect3DDevice9_Clear(global_state.d3d_dev,
							0, 
							0,
							D3DCLEAR_TARGET,
							D3DCOLOR_XRGB(0, 40, 100),
							1.0f, 
							0
						);

						if (cleared != D3D_OK) {
							OutputDebugStringA("Could not clear screen");
						}

						if (IDirect3DDevice9_BeginScene(global_state.d3d_dev) != D3D_OK) {
							OutputDebugStringA("BeginScene failed");
						}

						if (IDirect3DDevice9_EndScene(global_state.d3d_dev) != D3D_OK) {
							OutputDebugStringA("EndScene failed");
						}

						if (IDirect3DDevice9_Present(global_state.d3d_dev, 0, 0, 0, 0) != D3D_OK) {
							OutputDebugStringA("Present failed");
						}
					}
				}
				else {
					OutputDebugStringA("Could not create d3d device");
				}

			}
			else {
				OutputDebugStringA("Could not initialize d3d");
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
