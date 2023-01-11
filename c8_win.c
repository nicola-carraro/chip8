
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

bool c8_win_render(C8_Win_State* state) {

	bool result = true;

	HRESULT cleared = IDirect3DDevice9_Clear(state->d3d_dev,
		0,
		0,
		D3DCLEAR_TARGET,
		D3DCOLOR_XRGB(255, 255, 255),
		0.0f,
		0
	);

	if (cleared != D3D_OK) {
		OutputDebugStringA("Could not clear screen\n");

		if (cleared == D3DERR_INVALIDCALL)
		{
			OutputDebugStringA("Invalid call\n");
		}
		else {
			OutputDebugStringA("Something else");

		}
		result = false;
	}

	if (IDirect3DDevice9_BeginScene(state->d3d_dev) != D3D_OK) {
		OutputDebugStringA("BeginScene failed\n");
		result = false;
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
		result = false;
	}

	memcpy(vp, state->vertices, state->vertex_count * sizeof(C8_Win_D3d_Vertex));

	HRESULT unlocked = IDirect3DVertexBuffer9_Unlock(state->vb);
	if (unlocked != D3D_OK)
	{
		OutputDebugStringA("Failed to unlock vertex buffer\n");
		result = false;
	}

	HRESULT fvf_set = IDirect3DDevice9_SetFVF(
		state->d3d_dev,
		C8_WIN_D3D_FVF
	);

	if (fvf_set != D3D_OK)
	{
		OutputDebugStringA("SetFVF failed\n");
		result = false;

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
		result = false;
	}

	HRESULT drawn = IDirect3DDevice9_DrawPrimitive(
		state->d3d_dev,
		D3DPT_TRIANGLELIST,
		0,
		state->vertex_count / 3
	);

	if (drawn != D3D_OK) {
		OutputDebugStringA("DrawPrimitive failed\n");
		result = false;
	}

	if (IDirect3DDevice9_EndScene(state->d3d_dev) != D3D_OK) {
		OutputDebugStringA("EndScene failed\n");
		result = false;
	}

	if (IDirect3DDevice9_Present(state->d3d_dev, 0, 0, 0, 0) != D3D_OK) {
		OutputDebugStringA("Present failed\n");
		result = false;
	}

	state->vertex_count = 0;

	return result;
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

void c8_win_process_key(C8_Key* key, WORD key_flags) {

	BOOL is_up = (key_flags & KF_UP) == KF_UP;
	BOOL was_up = (key_flags & KF_REPEAT) != KF_REPEAT;

	if (is_up) {
		key->ended_down = false;
		key->was_lifted = true;
	}
	else {
		key->ended_down = true;
		key->was_down = true;
		key->was_pressed = true;
	}

	key->half_transitions++;
}

bool c8_win_init_dsound(C8_Win_State* state, HWND window, i32 samples_per_sec) {
	bool result = false;

	LPDIRECTSOUND dsound;

	HRESULT ds_created = DirectSoundCreate(0, &dsound, 0);

	WAVEFORMATEX wformat;
	wformat.nChannels = 1;

	DWORD buf_bytes = samples_per_sec * sizeof(i16) * wformat.nChannels;

	wformat.wFormatTag = WAVE_FORMAT_PCM;
	wformat.nSamplesPerSec = samples_per_sec;
	wformat.wBitsPerSample = 16;
	wformat.nBlockAlign = (wformat.nChannels * wformat.wBitsPerSample) / 8;
	wformat.nAvgBytesPerSec = wformat.nSamplesPerSec * wformat.nBlockAlign;
	wformat.cbSize = 8;

	if (SUCCEEDED(ds_created))
	{
		HRESULT cl = IDirectSound_SetCooperativeLevel(dsound, window, DSSCL_PRIORITY);
		if (SUCCEEDED(cl)) {
			DSBUFFERDESC pbuf_desc;
			c8_clear_struct(pbuf_desc);
			pbuf_desc.dwSize = sizeof(pbuf_desc);
			pbuf_desc.dwFlags = DSBCAPS_PRIMARYBUFFER;
			LPDIRECTSOUNDBUFFER pbuf;

			HRESULT pbuf_created = IDirectSound_CreateSoundBuffer(dsound, &pbuf_desc, &pbuf, 0);

			if (SUCCEEDED(pbuf_created)) {
				HRESULT fset = IDirectSoundBuffer_SetFormat(pbuf, &wformat);
				if (SUCCEEDED(fset)) {
					DSBUFFERDESC sbuf_desc;
					c8_clear_struct(sbuf_desc);
					sbuf_desc.dwSize = sizeof(sbuf_desc);
					sbuf_desc.dwBufferBytes = buf_bytes;
					sbuf_desc.lpwfxFormat = &wformat;
					LPDIRECTSOUNDBUFFER sbuf;

					HRESULT sbuf_created = IDirectSound_CreateSoundBuffer(dsound, &sbuf_desc, &sbuf, 0);

					if (SUCCEEDED(sbuf_created)) {

						LPVOID buf_data;
						DWORD buf_size;
						HRESULT locked = IDirectSoundBuffer_Lock(
							sbuf,
							0,
							0,
							&buf_data,
							&buf_size,
							0,
							0,
							DSBLOCK_ENTIREBUFFER
						);

						if (SUCCEEDED(locked)) {

							i32 hz = 250;
							i32 wave_period = samples_per_sec / hz;

							i32 half_wave_period = wave_period / 2;

							i16* samples = (i16*)buf_data;

							for (i16 i = 0; i < buf_size / sizeof(i16); i++) {
								i16 value;
								if ((i / half_wave_period) % 2 == 0) {
									value = 16000;
								}
								else {
									value = -16000;
								}

								samples[i] = value;
							}

							HRESULT unlocked = IDirectSoundBuffer_Unlock(
								sbuf,
								buf_data,
								buf_size,
								0,
								0,
								);

							if (SUCCEEDED(unlocked)) {

								state->ds = dsound;
								state->ds_sec_buf = sbuf;
								state->has_sound = true;
								result = true;
							}
							else {
								OutputDebugStringA("Could not unlock buffer");

							}

						}
						else {
							OutputDebugStringA("Could not lock buffer\n");
						}

					}
					else {
						OutputDebugStringA("Could not create secondary buffer\n");
					}

				}
				else {
					OutputDebugStringA("Could not set wave format of buffer\n");
				}
			}
			else {
				OutputDebugStringA("Failed to create primary buffer\n");
			}

		}
		else {
			OutputDebugStringA("Failed to set cooperative level\n");
		}

	}
	else {
		OutputDebugStringA("Could not create dsound interface\n");
	}

	return result;
}

LRESULT CALLBACK WindowProc(HWND window, UINT msg, WPARAM wparam, LPARAM lparam) {
	char buf[256];
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

		global_state.app_state.cli_width = LOWORD(lparam);
		global_state.app_state.cli_height = HIWORD(lparam);

		if (global_state.d3d_dev != 0)
		{
			D3DPRESENT_PARAMETERS d3dpp = c8_win_init_d3d_params(window);
			IDirect3DDevice9_Reset(global_state.d3d_dev, &d3dpp);
		}

	}break;
	case WM_KEYDOWN:
	case WM_KEYUP: {
		WORD key_flags = HIWORD(lparam);
		WORD scan_code = LOBYTE(key_flags);
		WORD vkey_code = LOWORD(wparam);
		C8_Control_Keys* controls = &(global_state.app_state.control_keys);
		switch (vkey_code) {
		case VK_RETURN: {
			C8_Key* key = &(controls->enter);
			c8_win_process_key(key, key_flags);
		}break;
		case VK_ESCAPE: {
			C8_Key* key = &(controls->esc);
			c8_win_process_key(key, key_flags);
		}break;
		case 'P': {
			C8_Key* key = &(controls->p);
			c8_win_process_key(key, key_flags);
		}break;
		case VK_SPACE: {
			C8_Key* key = &(controls->space);
			c8_win_process_key(key, key_flags);
		}break;
		}

		C8_Keypad* keypad = &(global_state.app_state.keypad);
		switch (scan_code) {
		case 2: {
			C8_Key* key = &(keypad->kp_1);
			c8_win_process_key(key, key_flags);
		}break;
		case 3: {
			C8_Key* key = &(keypad->kp_2);
			c8_win_process_key(key, key_flags); }break;
		case 4: {
			C8_Key* key = &(keypad->kp_3);
			c8_win_process_key(key, key_flags);
		}break;
		case 5: {
			C8_Key* key = &(keypad->kp_c);
			c8_win_process_key(key, key_flags);
		}break;
		case 16: {
			C8_Key* key = &(keypad->kp_4);
			c8_win_process_key(key, key_flags); }break;
		case 17: {
			C8_Key* key = &(keypad->kp_5);
			c8_win_process_key(key, key_flags);
		}break;
		case 18: {
			C8_Key* key = &(keypad->kp_6);
			c8_win_process_key(key, key_flags);
		}break;
		case 19: {
			C8_Key* key = &(keypad->kp_d);
			c8_win_process_key(key, key_flags);
		}break;
		case 30: {
			C8_Key* key = &(keypad->kp_7);
			c8_win_process_key(key, key_flags);
		}break;
		case 31: {
			C8_Key* key = &(keypad->kp_8);
			c8_win_process_key(key, key_flags);
		}break;
		case 32: {
			C8_Key* key = &(keypad->kp_9);
			c8_win_process_key(key, key_flags);
		}break;
		case 33: {
			C8_Key* key = &(keypad->kp_e);
			c8_win_process_key(key, key_flags);
		}break;
		case 44: {
			C8_Key* key = &(keypad->kp_a);
			c8_win_process_key(key, key_flags);
		}break;
		case 45: {
			C8_Key* key = &(keypad->kp_0);
			c8_win_process_key(key, key_flags);
		}break;
		case 46: {
			C8_Key* key = &(keypad->kp_b);
			c8_win_process_key(key, key_flags);
		}break;
		case 47: {
			C8_Key* key = &(keypad->kp_f);
			c8_win_process_key(key, key_flags);
		}break;
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

bool c8_win_start_beep(C8_Win_State * state) {
	bool result = false;

	HRESULT play = IDirectSoundBuffer_Play(state->ds_sec_buf, 0, 0, DSBPLAY_LOOPING);
	if (SUCCEEDED(play)) {
		result = true;
		state->is_beeping = true;
	}
	else {
		OutputDebugStringA("Could not start beeping\n");
	}

	return result;
}

bool c8_win_stop_beep(C8_Win_State* state) {
	bool result = false;

	HRESULT stop = IDirectSoundBuffer_Stop(state->ds_sec_buf);
	if (SUCCEEDED(stop)) {
		result = true;
		state->is_beeping = false;
	}
	else {
		OutputDebugStringA("Could not stop beeping\n");
	}

	return result;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, PSTR cmd_line, int cmd_show) {
	c8_clear_struct(global_state);

	HWND window = c8_win_create_window(instance);

	C8_Win_Timer timer = c8_win_init_timer();

	i32 samples_per_sec = 8000;
	i32 bytes_per_sample = 2;

	//i32 wave_counter = 0;
	//i32 half_wave_period = wave_period / 2;
	//u32 sample_index = 0;

	if (window != 0) {

		if (c8_win_initd3d(&global_state, window)) {
			ShowWindow(window, cmd_show);
			global_state.app_state.running = true;

			global_state.has_sound = false;
			if (c8_win_init_dsound(&global_state, window, samples_per_sec, bytes_per_sample)) {
				global_state.has_sound = true;
			}
			else {
				OutputDebugStringA("Could not initialize Direct Sound\n");
				assert(false);
			}

			//DWORD wave_counter = 0;

		/*	HRESULT play = IDirectSoundBuffer_Play(global_state.ds_sec_buf, 0, 0, DSBPLAY_LOOPING);

			if (FAILED(play)) {
				OutputDebugStringA("Could not play");
				assert(false);
			}*/

			while (global_state.app_state.running)
			{

				if (!c8_win_process_msgs(&global_state, window))
				{
					break;
				}

				if (!c8_app_update(&global_state.app_state))
				{
					OutputDebugStringA("Could not update app\n");
					assert(false);
				}

				if (!c8_win_render(&global_state))
				{
					OutputDebugStringA("Could not render\n");
					// TODO: understand why clear sometimes fails
					//assert(false);
				}

				if (!global_state.is_beeping && global_state.app_state.should_beep) {
					assert(c8_win_start_beep(&global_state));
				}

				if (global_state.is_beeping && !global_state.app_state.should_beep) {
					assert(c8_win_stop_beep(&global_state));
				}

				float milli_elapsed = c8_win_millis_elapsed(&timer, true);

				char str[255];
				sprintf(str, "Milliseconds: %f\n", milli_elapsed);
				//OutputDebugStringA(str);
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

C8_File c8_plat_read_file(char* name, i32 name_length, C8_Arena* arena) {
	C8_File result;
	c8_clear_struct(result);

	char buf[256];

	OFSTRUCT ofstruct;
	HFILE f = OpenFile(
		name,
		&ofstruct,
		OF_READ
	);

	if (f != HFILE_ERROR) {
		LARGE_INTEGER f_size;
		BOOL has_sz = GetFileSizeEx(
			f,
			&f_size
		);

		if (has_sz) {
			if (f_size.QuadPart <= C8_WIN_DWORD_MAX) {
				DWORD bytes_read;
				void* data = c8_arena_alloc(arena, f_size.QuadPart);

				if (data != 0)
				{
					BOOL read = ReadFile(
						f,
						data,
						f_size.QuadPart,
						&bytes_read,
						0
					);

					if (read) {
						result.size = bytes_read;
						result.data = data;
					}
					else {
						sprintf(buf, "Could not read\n", name);
						OutputDebugStringA(buf);
					}

				}
				else {
					OutputDebugStringA("Failed to allocate memory for file");
				}
			}
			else {
				sprintf(buf, "%s is too large\n", name);
				OutputDebugStringA(buf);
			}

		}
		else {
			sprintf(buf, "Could not get size of %s\n", name);
			OutputDebugStringA(buf);
		}

		if (!CloseHandle(f)) {
			sprintf(buf, "Could not close %s\n", name);
			OutputDebugStringA(buf);
		}
	}
	else {
		sprintf(buf, "Could not open %s\n", name);
		OutputDebugStringA(buf);
	}

	return result;
}

void c8_plat_debug_out(char* str) {
#if C8_DEBUG_PRINT

	OutputDebugStringA(str);
#endif

}

int c8_plat_debug_printf(char* format, ...) {

#if C8_DEBUG_PRINT
	char buf[256];
	va_list argp;
	va_start(argp, format);
	int result = vsnprintf(buf, sizeof(buf), format, argp);
	va_end(argp);
	OutputDebugStringA(buf);
	return result;
#else
	return 0;
#endif

}

#include "c8_app.c"
#endif // !C8_WIN_C