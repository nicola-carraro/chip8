#ifndef C8_APP
#define C8_APP
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>

typedef int32_t i32;
typedef int64_t i64;
typedef size_t psz;
typedef char u8;

#define c8_clear_struct(obj)(memset(&obj, 0, sizeof(obj)))

#define c8_arr_count(a)(sizeof(a) / sizeof(a[0]))

typedef struct {
	bool running;
	i32 cli_width;
	i32 cli_height;
} C8_App_State;

typedef struct {
	union {
		struct {
			float x;
			float y;
		};
	
		float coords[2];
	};
} C8_V2;

typedef struct {
	u8 r;
	u8 g;
	u8 b;
}C8_Rgb;

typedef struct {
	void* data;
	psz max_bytes;
	i32 alignement;
	psz offset;
}C8_Arena;

bool c8_plat_push_rect(float x, float y, float width, float height, C8_Rgb rgb);

void* c8_plat_allocate(psz size);

bool c8_app_update();

void* c8_arena_alloc(C8_Arena* arena, psz size);

bool c8_arena_init(C8_Arena* arena, psz size, i32 alignement);

void c8_arena_free_all(C8_Arena* arena);

bool c8_app_update();

#endif // !C8_APP
