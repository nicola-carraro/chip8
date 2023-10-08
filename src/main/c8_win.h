#ifndef C8_WIN
#define C8_WIN

#define UNICODE

#include <Windows.h>
#include <d3d9.h>
#include <DSound.h>
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

typedef struct
{
	FLOAT x;
	FLOAT y;
	FLOAT z;
	FLOAT rhw;
	D3DCOLOR color;
} C8_Win_Color_Vertex;

typedef struct
{
	LARGE_INTEGER perf_freq;
	LARGE_INTEGER perf_count;
	bool has_timer;
} C8_Win_Timer;

static C8_State global_state;

int c8_plat_debug_printf(char *format, ...);

#endif // !C8_WIN