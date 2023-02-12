
#ifndef C8_WIN_C
#define C8_WIN_C
#include "c8_win.h"

BOOL c8_win_draw_text(C8_Win_State* state) {

	BOOL result = false;

	if (SUCCEEDED(IDirect3DDevice9_SetFVF(state->d3d_dev, C8_WIN_TEX_FVF))) {
		if (SUCCEEDED(IDirect3DDevice9_SetStreamSource(state->d3d_dev, 0, state->text_vb, 0, sizeof(C8_Win_Texture_Vertex)))) {

			C8_Win_Texture_Vertex* vertices;

			HRESULT locked = IDirect3DVertexBuffer9_Lock(state->text_vb, 0, 0, (void**)&vertices, NULL);
			D3DCOLOR color = D3DCOLOR_ARGB(255, 255, 255, 255);
			if (SUCCEEDED(locked)) {

				for (i32 i = 0; i < state->text_vertex_count; i++) {

					C8_Win_Texture_Vertex vertex = state->text_vertices[i];
					vertices[i] = vertex;
				}

				HRESULT unlocked = IDirect3DVertexBuffer9_Unlock(state->text_vb);
				if (SUCCEEDED(unlocked)) {

					if (SUCCEEDED(IDirect3DDevice9_SetTexture(state->d3d_dev, 0, state->texture))) {
						if (SUCCEEDED(IDirect3DDevice9_DrawPrimitive(state->d3d_dev, D3DPT_TRIANGLEFAN, 0, 2))) {
							result = TRUE;
						}
						else {
							OutputDebugStringA("Could not draw primitive\n");

						}
					}
					else {
						OutputDebugStringA("Could not set texture\n");

					}
				}
				else {
					OutputDebugStringA("Could not unlock vertex buffer\n");

				}
			}
			else {
				OutputDebugStringA("Could not lock vertex buffer\n");
			}
		}
		else {
			OutputDebugStringA("Could not set stream source texture\n");
		}

	}
	else {
		OutputDebugStringA("Could not set FVF for texture\n");
	}

	return result;
}

BOOL c8_win_load_font(C8_Win_State* state, char* file_name, i32 name_length)
{

	BOOL result = false;

	C8_File file = c8_plat_read_file(file_name, name_length, &state->app_state.arena);

	if (file.data) {
		C8_Atlas_Header header = *((C8_Atlas_Header*)file.data);

		state->app_state.atlas = header;

		u8* input_bitmap = (u8*)file.data + sizeof(header);
		HRESULT textureCreated =
			IDirect3DDevice9_CreateTexture(
				state->d3d_dev,
				header.total_width_in_pixels,
				header.total_height_in_pixels,
				0,
				0,
				D3DFMT_A32B32G32R32F,
				D3DPOOL_MANAGED,
				&state->texture,
				NULL
			);

		if (SUCCEEDED(textureCreated)) {
			D3DLOCKED_RECT out_rect = { 0 };

			HRESULT locked = IDirect3DTexture9_LockRect(state->texture, 0, &out_rect, 0, 0);
			if (SUCCEEDED(locked)) {

				uint8_t* row_start = (uint8_t*)(out_rect.pBits);
				for (uint32_t row = 0; row < header.total_height_in_pixels; row++) {

					uint32_t f_index = 0;
					for (uint32_t column = 0; column < header.total_width_in_pixels; column++) {
						uint8_t src_pixel = input_bitmap[(row * header.total_width_in_pixels) + column];

						uint8_t* dest_pixel = ((uint8_t*)out_rect.pBits) + (row * out_rect.Pitch) + (column * 4);

						float alpha = ((float)src_pixel) / 255.0f;

					/*	if (alpha) {
							OutputDebugStringA("x");
						}
						else {
							OutputDebugStringA(" ");
						}*/

						((float*)row_start)[column * 4] = alpha;

						((float*)row_start)[column * 4 + 1] = alpha;
						((float*)row_start)[column * 4 + 2] = alpha;
						((float*)row_start)[column * 4 + 3] = alpha;

					}

					OutputDebugStringA("\n");

					row_start += out_rect.Pitch;
				}

			}
			else {
				OutputDebugStringA("Could not lock triangle\n");
			}

			int count = 0;

			uint8_t* input_bitmap = ((UINT)file.data + 2);

			uint8_t* row_start = (uint8_t*)(out_rect.pBits);

			HRESULT unlocked = IDirect3DTexture9_UnlockRect(state->texture, 0);
			if (SUCCEEDED(unlocked)) {

				HRESULT vb_created = IDirect3DDevice9_CreateVertexBuffer(
					state->d3d_dev,
					sizeof(C8_Win_Texture_Vertex) * 4,
					0,
					C8_WIN_TEX_FVF,
					D3DPOOL_MANAGED,
					&state->text_vb,
					0
				);

				if (SUCCEEDED(vb_created)) {
					result = TRUE;
				}
				else {
					OutputDebugStringA("Could not create vertex buffer\n");
				}
			}
			else {
				OutputDebugStringA("Could not unlock rect\n");
			}
		}
		else {
			OutputDebugStringA("Could not create texture\n");

		}

	}

	return result;

}

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

bool c8_draw_color(C8_Win_State *state) {

	BOOL result = FALSE;

	VOID* vp;

	HRESULT locked = IDirect3DVertexBuffer9_Lock(
		state->color_vb,
		0,
		0,
		&vp,
		0);
	if (SUCCEEDED(locked)) {
		memcpy(vp, state->color_vertices, state->color_vertex_count * sizeof(C8_Win_Color_Vertex));
		if (SUCCEEDED(IDirect3DVertexBuffer9_Unlock(state->color_vb))) {
			HRESULT fvf_set = IDirect3DDevice9_SetFVF(
				state->d3d_dev,
				C8_WIN_D3D_FVF
			);

			if (SUCCEEDED(fvf_set))
			{

				HRESULT stream_src_set = IDirect3DDevice9_SetStreamSource(
					state->d3d_dev,
					0,
					state->color_vb,
					0,
					sizeof(C8_Win_Color_Vertex)
				);

				if (SUCCEEDED(stream_src_set))
				{
					HRESULT drawn = IDirect3DDevice9_DrawPrimitive(
						state->d3d_dev,
						D3DPT_TRIANGLELIST,
						0,
						state->color_vertex_count / 3
					);

					if (SUCCEEDED(drawn)) {

						result = TRUE;

					}
					else {
						OutputDebugStringA("DrawPrimitive failed\n");
					}

				}
				else {
					OutputDebugStringA("SetStreamSource failed\n");
				}
			}
			else
			{
				OutputDebugStringA("SetFVF failed\n");

			}

		}
		else
		{
			OutputDebugStringA("Failed to unlock vertex buffer\n");
		}
	}
	else
	{
		OutputDebugStringA("Failed to lock vertex buffer\n");
	}

	return result;
}

bool c8_win_render(C8_Win_State* state) {

	bool result = false;

	HRESULT cleared = IDirect3DDevice9_Clear(state->d3d_dev,
		0,
		0,
		D3DCLEAR_TARGET,
		D3DCOLOR_XRGB(0, 0, 0),
		0.0f,
		0
	);

	if (SUCCEEDED(cleared))
	{
		if (SUCCEEDED(IDirect3DDevice9_BeginScene(state->d3d_dev))) {
			c8_draw_color(state);

			//c8_win_draw_text(state);

		}
		else
		{
			OutputDebugStringA("BeginScene failed\n");
		}
	}
	else {
		OutputDebugStringA("Could not clear screen\n");

		if (cleared == D3DERR_INVALIDCALL)
		{
			OutputDebugStringA("Invalid call\n");
		}
		else {
			OutputDebugStringA("Something else");

		}

	}

	if (SUCCEEDED(IDirect3DDevice9_EndScene(state->d3d_dev)))
	{
		if (SUCCEEDED(IDirect3DDevice9_Present(state->d3d_dev, 0, 0, 0, 0)))
		{
			result = true;
		}
		else
		{
			OutputDebugStringA("Present failed\n");
		}
	}
	else {
		OutputDebugStringA("EndScene failed\n");
	}

	state->color_vertex_count = 0;

	return result;
}

bool c8_win_init_texture(C8_Win_State* state, char* file_name, i32 name_length) {

	bool result = false;

	C8_File file = c8_plat_read_file(file_name, name_length, &state->app_state.arena);

	if (file.data != 0) {

		UINT width = ((UINT*)file.data)[0];
		UINT height = ((UINT*)file.data)[1];

		HRESULT textureCreated =
			IDirect3DDevice9_CreateTexture(
				state->d3d_dev,
				width,
				height,
				0,
				0,
				D3DFMT_A32B32G32R32F,
				D3DPOOL_MANAGED,
				&state->texture,
				NULL
			);

		if (SUCCEEDED(textureCreated)) {

			D3DLOCKED_RECT rect;
			rect.Pitch = width;
			rect.pBits = 0;

			u8* input_bitmap = (u8*)(((UINT*)file.data) + 2);
			HRESULT locked = IDirect3DTexture9_LockRect(state->texture, 0, &rect, 0, 0);

			if (SUCCEEDED(locked)) {

				u32 float_count = 0;
				for (u32 row = 0; row < height; row++) {
					for (u32 column = 0; column < width; column++) {
						u8 pixel = input_bitmap[(row * width) + column];
						if (pixel != 0) {
							OutputDebugStringA("x");

						}
						else {
							OutputDebugStringA(".");

						}

						for (u32 channel = 0; channel < 4; channel++) {

							float color = (float)pixel / 255.0f;
							((float*)rect.pBits)[float_count] = color;
							float_count++;
						}
					}
					OutputDebugStringA("\n");
				}

				HRESULT unlocked = IDirect3DTexture9_UnlockRect(state->texture, 0);

				if (SUCCEEDED(unlocked)) {
					result = true;

				}
				else {
					OutputDebugStringA("Failed to unlock texture\n");
				}

			}
			else {
				OutputDebugStringA("Failed to lock texture\n");
			}
		}
		else {
			OutputDebugStringA("Failed to create texture\n");
		}
	}

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
			D3DCREATE_HARDWARE_VERTEXPROCESSING,
			&d3dpp,
			&state->d3d_dev
		);

		if (SUCCEEDED(device_created)) {
			c8_clear_struct(state->color_vertices);

			HRESULT vb_created = IDirect3DDevice9_CreateVertexBuffer(
				state->d3d_dev,
				sizeof(state->color_vertices),
				0,
				C8_WIN_D3D_FVF,
				D3DPOOL_MANAGED,
				&state->color_vb,
				0
			);

			if (SUCCEEDED(vb_created)) {
				result = true;
				char file_name[] = "data/atlas.c8";
				c8_win_load_font(state, file_name, c8_arr_count(file_name) - 1);
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
								0);

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

bool c8_win_push_color_vertex(C8_Win_State* state, float x, float y, u8 r, u8 g, u8 b) {
	bool result = false;

	assert(state->color_vertex_count < c8_arr_count(state->color_vertices));

	if (state->color_vertex_count < c8_arr_count(state->color_vertices))
	{
		state->color_vertices[state->color_vertex_count].x = x;
		state->color_vertices[state->color_vertex_count].y = y;
		state->color_vertices[state->color_vertex_count].z = 0;
		state->color_vertices[state->color_vertex_count].rhw = 0;

		state->color_vertices[state->color_vertex_count].color = D3DCOLOR_XRGB(r, g, b);
		state->color_vertex_count++;
		result = true;
	}
	else {
		OutputDebugStringA("Vertex buffer size exceeded");
	}

	return result;
}

bool c8_win_push_text_vertex(C8_Win_State* state, float x, float y,  u8 r, u8 g, u8 b, float u, float v) {
	bool result = false;

	assert(state->color_vertex_count < c8_arr_count(state->text_vertices));

	if (state->text_vertex_count < c8_arr_count(state->text_vertices))
	{
		state->text_vertices[state->text_vertex_count].x = x;
		state->text_vertices[state->text_vertex_count].y = y;
		state->text_vertices[state->text_vertex_count].z = 0;
		state->text_vertices[state->text_vertex_count].rhw = 0;
		state->text_vertices[state->text_vertex_count].u = u;
		state->text_vertices[state->text_vertex_count].v = v;

		state->text_vertices[state->text_vertex_count].color = D3DCOLOR_XRGB(r, g, b);
		state->text_vertex_count++;
		result = true;
	}
	else {
		OutputDebugStringA("Vertex buffer size exceeded");
	}

	return result;
}

bool c8_win_push_color_triangle(C8_Win_State* state, C8_V2 p1, C8_V2 p2, C8_V2 p3, C8_Rgb rgb) {
	bool push1 = c8_win_push_color_vertex(state, p1.x, p1.y, rgb.r, rgb.g, rgb.b);
	bool push2 = c8_win_push_color_vertex(state, p2.x, p2.y, rgb.r, rgb.g, rgb.b);
	bool push3 = c8_win_push_color_vertex(state, p3.x, p3.y, rgb.r, rgb.g, rgb.b);

	return push1 && push2 && push3;
}

bool c8_win_push_text_triangle(C8_Win_State* state, C8_V2 p1, C8_V2 p2, C8_V2 p3, C8_Rgb rgb, float u1, float v1, float u2, float v2, float u3, float v3) {
	bool push1 =c8_win_push_text_vertex(state, p1.x, p1.y, rgb.r, rgb.g, rgb.b, u1, v1);
	bool push2 =c8_win_push_text_vertex(state, p2.x, p2.y, rgb.r, rgb.g, rgb.b, u2, v2);
	bool push3 =c8_win_push_text_vertex(state, p3.x, p3.y, rgb.r, rgb.g, rgb.b, u3, v3);

	return push1 && push2 && push3;
}

bool c8_win_push_glyph(C8_Win_State* state, char c, float x, float y, float width, float height, C8_Rgb rgb) {
	C8_V2 p1 = { x, y };
	C8_V2 p2 = { x + width, y };
	C8_V2 p3 = { x + width, y + height };
	C8_V2 p4 = { x, y + height };

	i32 glyph_index = c - C8_FIRST_CHAR;
	C8_Atlas_Glyph glyph = state->app_state.atlas.glyphs[glyph_index];

	bool push1 = c8_win_push_text_triangle(
		&global_state,
		p1, p2, p3, 
		rgb, 
		glyph.u_left, glyph.v_top, 
		glyph.u_right, glyph.v_top,
		glyph.u_right, glyph.v_bottom
	);
	bool push2 = c8_win_push_text_triangle(
		&global_state,
		p1, p3, p4, 
		rgb,
		glyph.u_left, glyph.v_top,
		glyph.u_right, glyph.v_bottom,
		glyph.u_left, glyph.v_bottom);

	return push1 && push2;
}

bool c8_plat_push_text(char c, float x, float y, float width, float height, C8_Rgb rgb) {
	c8_win_push_glyph(&global_state, c, x, y, width, height, rgb);
}

bool c8_plat_push_color_rect(float x, float y, float width, float height, C8_Rgb rgb) {
	C8_V2 p1 = { x, y };
	C8_V2 p2 = { x + width, y };
	C8_V2 p3 = { x + width, y + height };
	C8_V2 p4 = { x, y + height };

	bool push1 = c8_win_push_color_triangle(&global_state, p1, p2, p3, rgb);
	bool push2 = c8_win_push_color_triangle(&global_state, p1, p3, p4, rgb);

	return push1 && push2;
}

bool c8_win_start_beep(C8_Win_State* state) {
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

	if (c8_arena_init(&global_state.app_state.arena, 5 * 1024 * 1024, 4)) {

		if (window != 0) {

			if (c8_win_initd3d(&global_state, window)) {
				ShowWindow(window, cmd_show);
				global_state.app_state.running = true;

				global_state.has_sound = false;
				if (c8_win_init_dsound(&global_state, window, samples_per_sec)) {
					global_state.has_sound = true;
				}
				else {
					OutputDebugStringA("Could not initialize Direct Sound\n");
					assert(false);
				}

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
						assert(false);
					}

					if (!global_state.is_beeping && global_state.app_state.should_beep) {
						bool beeped = c8_win_start_beep(&global_state);
						assert(beeped);
					}

					if (global_state.is_beeping && !global_state.app_state.should_beep) {
						bool stopped = c8_win_stop_beep(&global_state);
						assert(stopped);
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
	else {
		OutputDebugStringA("Failed to initialize arena\n");
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
	HANDLE f = CreateFileA(
		name,
		GENERIC_READ,
		FILE_SHARE_READ,
		0,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		0
	);

	if (f != INVALID_HANDLE_VALUE) {
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
						sprintf(buf, "Could not read %s\n", name);
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