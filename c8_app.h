#ifndef C8_APP
#define C8_APP
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint32_t u32;
typedef uint64_t u64;
typedef uint16_t u16;
typedef size_t psz;
typedef uint8_t u8;

#define C8_PROG_ADDR 0x200

#define C8_WIN_DWORD_MAX 4294967295

#define C8_RAM_BYTE_SIZE 4096

#define C8_FRAME_WIDTH 4

#define C8_PIXEL_SIDE 8

#define C8_PIXEL_ROWS 32

#define C8_PIXEL_COLS 64

#define C8_SCREEN_WIDTH (C8_PIXEL_SIDE * C8_PIXEL_COLS)

#define C8_SCREEN_HEIGHT (C8_PIXEL_SIDE * C8_PIXEL_ROWS)

#define C8_MONITOR_WIDTH (C8_SCREEN_WIDTH + (2 * C8_FRAME_WIDTH))

#define C8_MONITOR_HEIGHT (C8_SCREEN_HEIGHT + (2 * C8_FRAME_WIDTH))

#define c8_clear_struct(obj)(memset(&obj, 0, sizeof(obj)))

#define c8_arr_count(a)(sizeof(a) / sizeof(a[0]))

#define C8_INSTRUCTIONS_PER_SEC 700

#define C8_FRAMES_PER_SEC 60

#define C8_INSTRUCTIONS_PER_FRAME (C8_INSTRUCTIONS_PER_SEC /C8_FRAMES_PER_SEC )

#define C8_FONT_SIZE 5

#define C8_FONT_COUNT 16

#define C8_FONT_ADDR 0x050

#define C8_FLAG_REG 0xf

typedef struct {
	void* data;
	psz max_bytes;
	i32 alignement;
	psz offset;
} C8_Arena;

typedef struct {
	bool started_down;
	bool ended_down;
	bool was_down;
	bool was_pressed;
	bool was_lifted;
	i32 half_transitions;
}C8_Key;

typedef struct {
	union {
		struct {
			C8_Key kp_0;
			C8_Key kp_1;
			C8_Key kp_2;
			C8_Key kp_3;
			C8_Key kp_4;
			C8_Key kp_5;
			C8_Key kp_6;
			C8_Key kp_7;
			C8_Key kp_8;
			C8_Key kp_9;
			C8_Key kp_a;
			C8_Key kp_b;
			C8_Key kp_c;
			C8_Key kp_d;
			C8_Key kp_e;
			C8_Key kp_f;
		};
		C8_Key keys[16];
	};
}C8_Keypad;

typedef struct {
	union {
		struct {
			C8_Key enter;
			C8_Key esc;
			C8_Key p;
			C8_Key space;
		};
		C8_Key keys[4];
	};
}C8_Control_Keys;

typedef struct {
	bool running;
	i32 cli_width;
	i32 cli_height;
	bool pixels[C8_PIXEL_ROWS][C8_PIXEL_COLS];
	u8 ram[C8_RAM_BYTE_SIZE];
	u8 delay_timer;
	u8 sound_timer;
	u16 index_register;
	u8 var_registers[16];
	u16 stack[16];
	u8 stack_pointer;
	u64 frame_count;
	C8_Arena arena;
	u16 pc;
	bool initialised;
	bool program_loaded;
	C8_Keypad keypad;
	C8_Control_Keys control_keys;
	bool should_beep;
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
} C8_Rgb;

typedef struct {
	psz size;
	void* data;
} C8_File;

bool c8_plat_push_rect(float x, float y, float width, float height, C8_Rgb rgb);

void* c8_plat_allocate(psz size);

bool c8_app_update();

void* c8_arena_alloc(C8_Arena* arena, psz size);

bool c8_arena_init(C8_Arena* arena, psz size, i32 alignement);

void c8_arena_free_all(C8_Arena* arena);

bool c8_app_update();

void c8_plat_debug_out(char* str);

#endif // !C8_APP
