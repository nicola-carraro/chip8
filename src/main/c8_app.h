#ifndef C8_APP
#define C8_APP
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define C8_DEBUG_PRINT 1

#define C8_MAX_VERTICES (C8_PIXEL_ROWS * C8_PIXEL_COLS * 2 * 3) + (8 * 3)

typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint32_t u32;
typedef uint64_t u64;
typedef uint16_t u16;
typedef size_t psz;
typedef uint8_t u8;

#define C8_UNREFERENCED(a) (a)

#define C8_WIN_DWORD_MAX 4294967295

#define C8_RAM_BYTE_SIZE 4096

#define C8_FRAME_WIDTH 4.0f

#define C8_PIXEL_SIDE 8.0f

#define C8_PIXEL_ROWS 32

#define C8_PIXEL_COLS 64

#define C8_SCREEN_WIDTH (C8_PIXEL_SIDE * C8_PIXEL_COLS)

#define C8_SCREEN_HEIGHT (C8_PIXEL_SIDE * C8_PIXEL_ROWS)

#define C8_MONITOR_WIDTH (C8_SCREEN_WIDTH + (2 * C8_FRAME_WIDTH))

#define C8_MONITOR_HEIGHT (C8_SCREEN_HEIGHT + (2 * C8_FRAME_WIDTH))

#define c8_clear_struct(obj) (memset(&obj, 0, sizeof(obj)))

#define c8_arr_count(a) (sizeof(a) / sizeof(a[0]))

#define C8_INSTRUCTIONS_PER_SEC 880

#define C8_FRAMES_PER_SEC 60

#define C8_INSTRUCTIONS_PER_FRAME (C8_INSTRUCTIONS_PER_SEC / C8_FRAMES_PER_SEC)

#define C8_FONT_SIZE 5

#define C8_FONT_COUNT 16

#define C8_FONT_ADDR 0x050

#define C8_PROG_ADDR 0x200

#define C8_FLAG_REG 0xf

#define C8_FIRST_CHAR '!'

#define C8_ONE_PAST_LAST_CHAR 0x7f

#define C8_CHAR_COUNT (C8_ONE_PAST_LAST_CHAR - C8_FIRST_CHAR)

#define C8_LOAD_BUTTON_WIDTH (100.0f)

#define C8_LOAD_BUTTON_HEIGHT (50.0f)

#define C8_LOAD_BUTTON_Y (10.0f)

typedef struct
{
	u8 r;
	u8 g;
	u8 b;
	u8 a;
} C8_Rgba;

typedef struct
{
	float x;
	float y;
	C8_Rgba color;
	float u;
	float v;
} C8_Texture_Vertex;

typedef struct
{
	float x;
	float y;
	C8_Rgba color;
} C8_Color_Vertex;

typedef struct
{
	float u_left;
	float u_right;
	float v_top;
	float v_bottom;
	float y_offset;
	float advancement;
} C8_Atlas_Glyph;

typedef struct
{
	C8_Atlas_Glyph glyphs[C8_CHAR_COUNT];
	uint32_t width;
	uint32_t height;
	float line_height;
} C8_Atlas_Header;

typedef struct
{
	float horizontal_scaling;
	float vertical_scaling;
	float width_pixels;
	float height_pixels;
} C8_Text_Size;

typedef struct
{
	void *data;
	psz max_bytes;
	i32 alignement;
	psz offset;
} C8_Arena;

typedef struct
{
	bool started_down;
	bool ended_down;
	bool was_down;
	bool was_pressed;
	bool was_lifted;
	i32 half_transitions;
} C8_Key;

typedef union
{
	struct
	{
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
	} keypad;
	C8_Key keys[16];
} C8_Keypad;

typedef union
{
	struct
	{
		C8_Key enter;
		C8_Key esc;
		C8_Key p;
		C8_Key space;
	} control_keys;
	C8_Key keys[4];
} C8_Control_Keys;

typedef struct
{
	C8_Key left_button;
	C8_Key right_button;
} C8_Mouse_Buttons;

typedef union
{
	struct
	{
		float x;
		float y;
	} xy;

	float coords[2];
} C8_V2;

typedef struct
{
	psz size;
	void *data;
} C8_File;

typedef struct
{
	wchar_t *text;
	size_t length;
} C8_String;

typedef struct
{
	C8_Arena *arena;
	C8_String *file_names;
	size_t count;
	size_t capacity;
} C8_String_List;

typedef struct
{
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
	C8_Arena transient_arena;
	C8_Arena file_names_arena;
	u16 pc;
	bool initialised;
	bool program_loaded;
	C8_Keypad keypad;
	C8_Control_Keys control_keys;
	bool should_beep;
	C8_Atlas_Header atlas_header;
	C8_V2 mouse_position;
	C8_Mouse_Buttons mouse_buttons;
	bool load_button_down;
	C8_Color_Vertex color_vertices[C8_MAX_VERTICES];
	uint32_t color_vertex_count;
	C8_Texture_Vertex text_vertices[C8_MAX_VERTICES];
	uint32_t text_vertex_count;
	bool is_file_dialog_open;
	C8_String_List file_list;
	wchar_t *file_name;
} C8_App_State;

#define C8_FILE_LIST_INITIAL_CAPACITY 10

void *c8_plat_allocate(psz size);

bool c8_app_update();

void *c8_arena_alloc(C8_Arena *arena, psz size);

bool c8_arena_init(C8_Arena *arena, psz size, i32 alignement);

void c8_arena_free_all(C8_Arena *arena);

bool c8_app_update();

void c8_plat_debug_out(char *str);

const C8_Rgba emulator_color = {0, 0, 0, 255};

bool c8_push_color_vertex(C8_App_State *state, float x, float y, u8 r, u8 g, u8 b, u8 a);

bool c8_push_color_rect(C8_App_State *state, float x, float y, float width, float height, C8_Rgba rgb);

bool c8_plat_push_text(char *text, size_t text_length, float x, float y, C8_Text_Size text_size, C8_Rgba rgb);

bool c8_push_glyph(C8_App_State *state, C8_Atlas_Glyph glyph, float x, float y, float width, float height, C8_Rgba rgb);

bool c8_push_text_vertex(C8_App_State *state, float x, float y, u8 r, u8 g, u8 b, u8 a, float u, float v);

void c8_file_list_init(C8_String_List *file_list, C8_Arena *arena);

bool c8_plat_list_folder_content(C8_App_State *state, wchar_t *folder_name, size_t folder_name_length);

bool c8_push_file_name(C8_String_List *file_list, wchar_t *file_name, size_t name_length);

C8_File c8_plat_read_file(wchar_t *name, size_t name_length, C8_Arena *arena);

#endif // !C8_APP
