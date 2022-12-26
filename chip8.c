#include <Windows.h>
#include<stdbool.h>
#include <stdio.h>
#include <assert.h>

#include <d3d9.h>

#define C8_D3D_FVF (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)

typedef struct {
	FLOAT x;
	FLOAT y;
	FLOAT z;
	FLOAT rhw;
	D3DCOLOR color;
} D3d_Vertex;

#define clear_struct(obj)(memset(&obj, 0, sizeof(obj)))

typedef struct {
	boolean running;
	LPDIRECT3D9 d3d;
	LPDIRECT3DDEVICE9 d3d_dev;
	float cli_width;
	float cli_height;
} State;

State global_state;

D3DPRESENT_PARAMETERS c8_d3d_init_pp(HWND window)
{
	D3DPRESENT_PARAMETERS result;

	result.BackBufferWidth = 0;
	result.BackBufferHeight = 0;
	result.BackBufferFormat = D3DFMT_UNKNOWN;
	result.BackBufferCount = 0;
	result.MultiSampleType = D3DMULTISAMPLE_NONE;
	result.MultiSampleQuality = 0;
	result.SwapEffect = D3DSWAPEFFECT_DISCARD;
	result.hDeviceWindow = window;
	result.Windowed = true;
	result.EnableAutoDepthStencil =0;
	result.AutoDepthStencilFormat = D3DFMT_UNKNOWN;
	result.Flags = 0;
	result.FullScreen_RefreshRateInHz = 0;
	result.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

	return result;
}

LRESULT CALLBACK WindowProc(HWND window, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch (msg) {
	case WM_CLOSE:
	{
		if (!DestroyWindow(window))
		{
			OutputDebugStringA("Could not destroy window\n");
		}
	}break;

	case WM_DESTROY:
	{
		PostQuitMessage(0);
		global_state.running = false;

	}break;
	case WM_QUIT:
	{
		global_state.running = false;
	} break;
	case WM_SIZE:
	{

		if (global_state.d3d_dev != 0)
		{
			D3DPRESENT_PARAMETERS d3dpp = c8_d3d_init_pp(window);
			IDirect3DDevice9_Reset(global_state.d3d_dev, &d3dpp);
		}

	}break;

	}

	return DefWindowProcA(window, msg, wparam, lparam);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, PSTR cmd_line, int cmd_show) {
	clear_struct(global_state);
	const char* class_name = "chip8";

	WNDCLASSA wc;
	clear_struct(wc);

	wc.lpfnWndProc = WindowProc;
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

		LARGE_INTEGER perf_freq;

		if (QueryPerformanceFrequency(&perf_freq)) {
			LARGE_INTEGER perf_count;
			if (!QueryPerformanceCounter(&perf_count))
			{
				"Failed to get perfCount\n";
			}
			global_state.d3d = Direct3DCreate9(DIRECT3D_VERSION);
			if (window != 0) {
				if (global_state.d3d != 0)
				{
					D3DPRESENT_PARAMETERS d3dpp = c8_d3d_init_pp(window);
					HRESULT device_created = IDirect3D9_CreateDevice(
						global_state.d3d,
						D3DADAPTER_DEFAULT,
						D3DDEVTYPE_HAL,
						window,
						D3DCREATE_SOFTWARE_VERTEXPROCESSING,
						&d3dpp,
						&global_state.d3d_dev
					);

					if (device_created == D3D_OK) {
						D3d_Vertex vertices[3];
						memset(vertices, 0, sizeof(vertices));

						LPDIRECT3DVERTEXBUFFER9 vb;
						HRESULT vb_created = IDirect3DDevice9_CreateVertexBuffer(
							global_state.d3d_dev,
							sizeof(vertices),
							0,
							C8_D3D_FVF,
							D3DPOOL_MANAGED,
							&vb,
							0
						);

						if (vb_created == D3D_OK) {
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

								if (!global_state.running)
								{
									break;
								}

								vertices[0].x = 0.0f;
								vertices[0].y = 0.0f;
								vertices[0].color = D3DCOLOR_XRGB(100, 0, 0);

								vertices[1].x = 200.0f;
								vertices[1].y = 0.0f;
								vertices[1].color = D3DCOLOR_XRGB(100, 0, 0);

								vertices[2].x = 200.0f;
								vertices[2].y = 200.0f;
								vertices[2].color = D3DCOLOR_XRGB(100, 0, 0);

								HRESULT cleared = IDirect3DDevice9_Clear(global_state.d3d_dev,
									0,
									0,
									D3DCLEAR_TARGET,
									D3DCOLOR_XRGB(0, 40, 100),
									1.0f,
									0
								);

								if (cleared != D3D_OK) {
									OutputDebugStringA("Could not clear screen\n");
								}

								if (IDirect3DDevice9_BeginScene(global_state.d3d_dev) != D3D_OK) {
									OutputDebugStringA("BeginScene failed\n");
								}

								VOID* vp;
								HRESULT locked = IDirect3DVertexBuffer9_Lock(
									vb,
									0,
									0,
									&vp,
									0);
								if (locked != D3D_OK)
								{
									OutputDebugStringA("Failed to lock vertex buffer\n");
								}

								memcpy(vp, vertices, sizeof(vertices));

								HRESULT unlocked = IDirect3DVertexBuffer9_Unlock(vb);
								if (unlocked != D3D_OK)
								{
									OutputDebugStringA("Failed to unlock vertex buffer\n");
								}

								HRESULT fvf_set = IDirect3DDevice9_SetFVF(
									global_state.d3d_dev,
									C8_D3D_FVF
								);

								if (fvf_set != D3D_OK)
								{
									OutputDebugStringA("SetFVF failed\n");
								}

								HRESULT stream_src_set = IDirect3DDevice9_SetStreamSource(
									global_state.d3d_dev,
									0,
									vb,
									0,
									sizeof(D3d_Vertex)
								);

								if (stream_src_set != D3D_OK)
								{
									OutputDebugStringA("SetStreamSource failed\n");
								}

								HRESULT drawn = IDirect3DDevice9_DrawPrimitive(
									global_state.d3d_dev,
									D3DPT_TRIANGLELIST,
									0,
									1);

								if (drawn != D3D_OK) {
									OutputDebugStringA("DrawPrimitive failed\n");
								}

								if (IDirect3DDevice9_EndScene(global_state.d3d_dev) != D3D_OK) {
									OutputDebugStringA("EndScene failed\n");
								}

								if (IDirect3DDevice9_Present(global_state.d3d_dev, 0, 0, 0, 0) != D3D_OK) {
									OutputDebugStringA("Present failed\n");
								}

								LARGE_INTEGER old_perf_count = perf_count;
								if (!QueryPerformanceCounter(&perf_count))
								{
									"Failed to get perfCount\n";
								}

								float secs_elapsed = ((float)(perf_count.QuadPart - old_perf_count.QuadPart)) / ((float)(perf_freq.QuadPart));

								float milli_elapsed = secs_elapsed * 1000.0f;

								char str[255];
								sprintf(str, "Milliseconds: %f\n", milli_elapsed);
								OutputDebugStringA(str);
							}
						}
						else {
							OutputDebugStringA("Failed to create vertex buffer");
						}
					}
					else {
						OutputDebugStringA("Could not create d3d device\n");
					}

				}
				else {
					OutputDebugStringA("Could not initialize d3d\n");
				}

			}
			else {
				OutputDebugStringA("Could not open window\n");
			}
		}
		else {
			OutputDebugStringA("Could not get performance frequency\n");
		}
	}
	else {
		OutputDebugStringA("Could not register window class\n");
	}
}
