#ifndef C8_WIN
#define C8_WIN

#include <Windows.h>
#include <d3d9.h>
#include<DSound.h>
#include "c8_app.h"

#define C8_WIN_D3D_FVF (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)

#define C8_WIN_MAX_VERTICES (C8_PIXEL_ROWS * C8_PIXEL_COLS * 2 * 3) + (8 * 3)

typedef struct {
	FLOAT x;
	FLOAT y;
	FLOAT z;
	FLOAT rhw;
	D3DCOLOR color;
} C8_Win_D3d_Vertex;

typedef struct {
	LPDIRECT3D9 d3d;
	LPDIRECT3DDEVICE9 d3d_dev;
	LPDIRECT3DVERTEXBUFFER9 vb;
	LPDIRECTSOUND ds;
	LPDIRECTSOUNDBUFFER ds_sec_buf;
	C8_App_State app_state;
	C8_Win_D3d_Vertex vertices[C8_WIN_MAX_VERTICES];
	i32 vertex_count;
	bool has_sound;
	bool is_beeping;
} C8_Win_State;

typedef struct {
	LARGE_INTEGER perf_freq;
	LARGE_INTEGER perf_count;
	bool has_timer;
}C8_Win_Timer;

C8_Win_State global_state;

int c8_plat_debug_printf(char* str, char* format, psz size, ...);

#endif // !C8_WIN