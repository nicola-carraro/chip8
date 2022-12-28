
#ifndef C8_WIN_C
#define C8_WIN_C
#include "c8_win.h"

D3DPRESENT_PARAMETERS c8_win_init_d3d_params(HWND window)
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
	result.EnableAutoDepthStencil = 0;
	result.AutoDepthStencilFormat = D3DFMT_UNKNOWN;
	result.Flags = 0;
	result.FullScreen_RefreshRateInHz = 0;
	result.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

	return result;
}

bool c8_win_query_perf_count(C8_Win_Timer* timer, LARGE_INTEGER* perf_count)
{
	bool result = false;
	if (QueryPerformanceCounter(perf_count)) {
		result = true;
	}
	else {
		timer->has_timer = false;
		OutputDebugStringA("Failed to get perfCount\n");
	}
	return result;
}

C8_Win_Timer c8_win_init_timer()
{
	C8_Win_Timer result;
	c8_clear_struct(result);
	result.has_timer = false;

	LARGE_INTEGER perf_freq;

	if (QueryPerformanceFrequency(&perf_freq)) {
		LARGE_INTEGER perf_count;

		if (c8_win_query_perf_count(&result, &perf_count)) {
			result.has_timer = true;
			result.perf_freq = perf_freq;
			result.perf_count = perf_count;
		}

	}
	else {
		OutputDebugStringA("Could not get performance frequency\n");
	}

	return result;
}

float c8_win_compute_millis(C8_Win_Timer timer, LARGE_INTEGER new_perf_count)
{
	float secs_elapsed = ((float)(new_perf_count.QuadPart - timer.perf_count.QuadPart)) /
		((float)(timer.perf_freq.QuadPart));

	float result = secs_elapsed * 1000.0f;

	return result;
}

float c8_win_millis_elapsed(C8_Win_Timer* timer, bool reset_timer)
{
	float millis_elapsed = -1.0f;

	if (timer->has_timer)
	{
		LARGE_INTEGER perf_count;

		if (c8_win_query_perf_count(timer, &perf_count)) {

			millis_elapsed = c8_win_compute_millis(*timer, perf_count);

			if (reset_timer)
			{
				timer->perf_count = perf_count;
			}
		}
	}

	return millis_elapsed;
}

void c8_win_render(C8_Win_State* state) {

	bool result = false;

	HRESULT cleared = IDirect3DDevice9_Clear(state->d3d_dev,
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

	if (IDirect3DDevice9_BeginScene(state->d3d_dev) != D3D_OK) {
		OutputDebugStringA("BeginScene failed\n");
	}

	VOID* vp;
	HRESULT locked = IDirect3DVertexBuffer9_Lock(
		state->vb,
		0,
		0,
		&vp,
		0);
	if (locked != D3D_OK)
	{
		OutputDebugStringA("Failed to lock vertex buffer\n");
	}

	memcpy(vp, state->vertices, state->vertex_count * sizeof(C8_Win_D3d_Vertex));

	HRESULT unlocked = IDirect3DVertexBuffer9_Unlock(state->vb);
	if (unlocked != D3D_OK)
	{
		OutputDebugStringA("Failed to unlock vertex buffer\n");
	}

	HRESULT fvf_set = IDirect3DDevice9_SetFVF(
		state->d3d_dev,
		C8_WIN_D3D_FVF
	);

	if (fvf_set != D3D_OK)
	{
		OutputDebugStringA("SetFVF failed\n");
	}

	HRESULT stream_src_set = IDirect3DDevice9_SetStreamSource(
		state->d3d_dev,
		0,
		state->vb,
		0,
		sizeof(C8_Win_D3d_Vertex)
	);

	if (stream_src_set != D3D_OK)
	{
		OutputDebugStringA("SetStreamSource failed\n");
	}

	HRESULT drawn = IDirect3DDevice9_DrawPrimitive(
		state->d3d_dev,
		D3DPT_TRIANGLELIST,
		0,
		state->vertex_count / 3
	);

	if (drawn != D3D_OK) {
		OutputDebugStringA("DrawPrimitive failed\n");
	}

	if (IDirect3DDevice9_EndScene(state->d3d_dev) != D3D_OK) {
		OutputDebugStringA("EndScene failed\n");
	}

	if (IDirect3DDevice9_Present(state->d3d_dev, 0, 0, 0, 0) != D3D_OK) {
		OutputDebugStringA("Present failed\n");
	}

	state->vertex_count = 0;
}

bool c8_win_initd3d(C8_Win_State* state, HWND window)
{
	bool result = false;
	state->d3d = Direct3DCreate9(DIRECT3D_VERSION);
	if (state->d3d != 0)
	{
		D3DPRESENT_PARAMETERS d3dpp = c8_win_init_d3d_params(window);
		HRESULT device_created = IDirect3D9_CreateDevice(
			state->d3d,
			D3DADAPTER_DEFAULT,
			D3DDEVTYPE_HAL,
			window,
			D3DCREATE_SOFTWARE_VERTEXPROCESSING,
			&d3dpp,
			&state->d3d_dev
		);

		if (device_created == D3D_OK) {
			c8_clear_struct(state->vertices);

			HRESULT vb_created = IDirect3DDevice9_CreateVertexBuffer(
				state->d3d_dev,
				sizeof(state->vertices),
				0,
				C8_WIN_D3D_FVF,
				D3DPOOL_MANAGED,
				&state->vb,
				0
			);

			if (vb_created == D3D_OK) {
				result = true;
			}
			else {
				OutputDebugStringA("Failed to create vertex buffer");
			}
		}
		else {
			OutputDebugStringA("Could not create d3d device\n");
		}

	}

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
		global_state.app_state.running = false;

	}break;
	case WM_QUIT:
	{
		global_state.app_state.running = false;
	} break;
	case WM_SIZE:
	{

		if (global_state.d3d_dev != 0)
		{
			D3DPRESENT_PARAMETERS d3dpp = c8_win_init_d3d_params(window);
			IDirect3DDevice9_Reset(global_state.d3d_dev, &d3dpp);
		}

	}break;

	}

	return DefWindowProcA(window, msg, wparam, lparam);
}

HWND c8_win_create_window(HINSTANCE instance)
{
	const char* class_name = "chip8";

	WNDCLASSA wc;
	c8_clear_struct(wc);

	wc.lpfnWndProc = WindowProc;
	wc.hInstance = instance;
	wc.lpszClassName = class_name;
	HWND window = 0;
	if (RegisterClassA(&wc) != 0) {

		window = CreateWindowExA(
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

	}
	else {
		OutputDebugStringA("Could not register window class\n");
	}

	return window;
}

bool c8_win_process_msgs(C8_Win_State* state, HWND window) {
	MSG msg;
	while (PeekMessage(&msg, window, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}

	if (!state->app_state.running)
	{
		return false;
	}

	return true;
}

bool c8_win_push_vertex(C8_Win_State* state, float x, float y, u8 r, u8 g, u8 b) {
	bool result = false;

	assert(state->vertex_count < c8_arr_count(state->vertices));

	if (state->vertex_count < c8_arr_count(state->vertices))
	{
		state->vertices[state->vertex_count].x = x;
		state->vertices[state->vertex_count].y = y;
		state->vertices[state->vertex_count].color = D3DCOLOR_XRGB(r, g, b);
		state->vertex_count++;
		result = true;
	}
	else {
		OutputDebugStringA("Vertex buffer size exceeded");
	}

	return result;
}

bool c8_win_push_triangle(C8_Win_State* state, C8_V2 p1, C8_V2 p2, C8_V2 p3, C8_Rgb rgb) {
	bool push1 = c8_win_push_vertex(state, p1.x, p1.y, rgb.r, rgb.g, rgb.b);
	bool push2 = c8_win_push_vertex(state, p2.x, p2.y, rgb.r, rgb.g, rgb.b);
	bool push3 = c8_win_push_vertex(state, p3.x, p3.y, rgb.r, rgb.g, rgb.b);

	return push1 && push2 && push3;
}

bool c8_plat_push_rect(float x, float y, float width, float height, C8_Rgb rgb) {
	C8_V2 p1 = { x, y };
	C8_V2 p2 = { x + width, y };
	C8_V2 p3 = { x + width, y + height };
	C8_V2 p4 = { x, y + height };

	bool push1 = c8_win_push_triangle(&global_state, p1, p2, p3, rgb);
	bool push2 = c8_win_push_triangle(&global_state, p1, p3, p4, rgb);

	return push1 && push2;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, PSTR cmd_line, int cmd_show) {
	c8_clear_struct(global_state);

	HWND window = c8_win_create_window(instance);

	C8_Win_Timer timer = c8_win_init_timer();

	if (window != 0) {
		if (c8_win_initd3d(&global_state, window)) {
			ShowWindow(window, cmd_show);
			global_state.app_state.running = true;
			while (global_state.app_state.running)
			{

				if (!c8_win_process_msgs(&global_state, window))
				{
					break;
				}

				c8_app_update();

				c8_win_render(&global_state);

				float milli_elapsed = c8_win_millis_elapsed(&timer, true);

				char str[255];
				sprintf(str, "Milliseconds: %f\n", milli_elapsed);
				OutputDebugStringA(str);
			}
		}
		else {
			OutputDebugStringA("Could not initialize Direct3D\n");
		}
	}
	else {
		OutputDebugStringA("Could not open window\n");
	}
}

void* c8_plat_allocate(psz size) {
	void* result = VirtualAlloc(
		0,
		size,
		MEM_COMMIT,
		PAGE_READWRITE
	);

	return result;
}
#endif // !C8_WIN_C