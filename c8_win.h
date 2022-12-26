#ifndef C8_WIN
#define C8_WIN

#include <Windows.h>
#include <d3d9.h>
#include "c8_app.h"

#define C8_WIN_D3D_FVF (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)

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
	C8_App_State app_state;
	C8_Win_D3d_Vertex vertices[3];
} C8_Win_State;

C8_Win_State global_state;

#endif 