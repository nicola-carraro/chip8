#ifndef C8_APP_C
#define C8_APP_C

#include "c8_app.h"

i32 c8_frame_x(C8_App_State* state)
{
	i32 result = (state->cli_width / 2) - (C8_MONITOR_WIDTH / 2);

	return result;
}

i32 c8_frame_y(C8_App_State* state)
{
	i32 result = (state->cli_height / 2) - (C8_MONITOR_HEIGHT / 2);

	return result;
}

bool c8_push_frame(C8_App_State* state) {
	C8_Rgb frame_color = { 100, 100, 255 };

	C8_Arena arena;

	i32 frame_x = c8_frame_x(state);

	i32 frame_y = c8_frame_y(state);

	bool push1 = c8_plat_push_rect(
		frame_x,
		frame_y,
		C8_MONITOR_WIDTH,
		C8_FRAME_WIDTH,
		frame_color
	);

	bool push2 = c8_plat_push_rect(
		frame_x + C8_MONITOR_WIDTH,
		frame_y,
		C8_FRAME_WIDTH,
		C8_MONITOR_HEIGHT,
		frame_color);

	bool push3 = c8_plat_push_rect(
		frame_x + C8_FRAME_WIDTH,
		frame_y + C8_MONITOR_HEIGHT,
		C8_MONITOR_WIDTH,
		C8_FRAME_WIDTH,
		frame_color
	);

	bool push4 = c8_plat_push_rect(
		frame_x,
		frame_y + C8_FRAME_WIDTH,
		C8_FRAME_WIDTH,
		C8_MONITOR_HEIGHT,
		frame_color
	);

	return push1 && push2 && push3 && push4;
}

bool c8_push_pixels(C8_App_State* state) {

	C8_Rgb pixel_color = { 0, 0, 0 };

	for (i32 r = 0; r < c8_arr_count(state->pixels); r++)
	{
		for (i32 c = 0; c < c8_arr_count(state->pixels[r]); c++)
		{
			if (state->pixels[r][c])
			{
				i32 frame_x = c8_frame_x(state);
				i32 frame_y = c8_frame_y(state);

				bool push = c8_plat_push_rect(
					frame_x + (c * C8_PIXEL_SIDE) + C8_FRAME_WIDTH,
					frame_y + (r * C8_PIXEL_SIDE) + C8_FRAME_WIDTH,
					C8_PIXEL_SIDE,
					C8_PIXEL_SIDE,
					pixel_color
				);

				if(!push){
					return false;
				}
			}
		}
	}

	return true;
}

bool c8_app_update(C8_App_State *state){

	state->pixels[state->frame_count % C8_PIXEL_ROWS][state->frame_count % C8_PIXEL_COLS] = true;

	bool push_frame = c8_push_frame(state);

	bool push_pixels = c8_push_pixels(state);

	state->frame_count++;

	return push_frame && push_pixels;
}

bool c8_arena_init(C8_Arena* arena, psz size, i32 alignement) {
	bool result = false;

	assert(size % alignement == 0);

	void* mem = c8_plat_allocate(size);

	if (mem != 0) {
		arena->max_bytes = size;
		arena->alignement = alignement;
		arena->offset = 0;
		arena->data = mem;
	}
	else {
		OutputDebugStringA("Arena initialization failed");
		assert(false);
		arena = 0;
	}

	return result;
}

void* c8_arena_alloc(C8_Arena* arena, psz size) {
	assert(arena != 0);

	void* result = 0;

	psz eff_size = size;
	if (eff_size % arena->alignement != 0)
	{
		eff_size += arena->alignement - (eff_size % arena->alignement);
	}

	bool big_enough = arena->offset + eff_size <= arena->max_bytes;

	assert(big_enough);

	if (big_enough)
	{
		result = ((u8*)arena->data) + arena->offset;
		arena->offset += eff_size;
	}

	return result;
}

void c8_arena_free_all(C8_Arena* arena) {
	assert(arena != 0);

	arena->offset = 0;
}

#endif // !C8_APP_C
