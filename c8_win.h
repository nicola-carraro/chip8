#ifndef C8_WIN
#define C8_WIN

#include <Windows.h>
#include <d3d9.h>
#include<DSound.h>
#include "c8_app.h"

#define C8_WIN_D3D_FVF (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)

#define C8_WIN_TEX_FVF (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)

typedef struct
{
	FLOAT x;
	FLOAT y;
	FLOAT z;
	FLOAT rhw;
	D3DCOLOR color;
	float u;
	float v;
} C8_Win_Texture_Vertex;

typedef struct {
	FLOAT x;
	FLOAT y;
	FLOAT z;
	FLOAT rhw;
	D3DCOLOR color;
} C8_Win_Color_Vertex;

typedef struct {
	LPDIRECT3D9 d3d;
	LPDIRECT3DDEVICE9 d3d_dev;
	LPDIRECT3DVERTEXBUFFER9 color_vb;
	LPDIRECT3DVERTEXBUFFER9 text_vb;
	LPDIRECT3DTEXTURE9 texture;
	LPDIRECTSOUND ds;
	LPDIRECTSOUNDBUFFER ds_sec_buf;
	C8_App_State app_state;
	C8_Win_Texture_Vertex text_vertices[C8_MAX_VERTICES];
	i32 text_vertex_count;
	bool has_sound;
	bool is_beeping;

} C8_Win_State;

typedef struct {
	LARGE_INTEGER perf_freq;
	LARGE_INTEGER perf_count;
	bool has_timer;
}C8_Win_Timer;

C8_Win_State global_state;

int c8_plat_debug_printf(char* format, ...);

C8_File c8_plat_read_file(char* name, i32 name_length, C8_Arena* arena);

#endif // !C8_WIN