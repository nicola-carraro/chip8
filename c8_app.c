#ifndef C8_APP_C
#define C8_APP_C

#include "c8_app.h"

u16 c8_read_instruction(u16 bytes)
{
	u16 result = ((bytes & 0x00ff) << 8) | ((bytes & 0xff00) >> 8);
	return result;
}

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

void c8_debug_keyboard(C8_Key *key, char* n) {
	bool printed = false;
	char buf[256];
	if (key->started_down) {
		c8_plat_debug_printf(buf, c8_arr_count(buf), "%s started down\n",n);
		printed = true;
	}

	if (key->was_down) {
		c8_plat_debug_printf(buf, c8_arr_count(buf), "%s was down\n", n);
		printed = true;

	}

	if (key->ended_down) {
		c8_plat_debug_printf(buf, c8_arr_count(buf), "%s ended down\n", n);
		printed = true;

	}
	if (key->was_pressed) {
		c8_plat_debug_printf(buf, c8_arr_count(buf), "%s was pressed\n", n);
		printed = true;

	}
	if (key->was_lifted) {
		c8_plat_debug_printf(buf, c8_arr_count(buf), "%s was lifted\n", n);
		printed = true;

	}

	if (key->half_transitions != 0) {
		c8_plat_debug_printf(buf, c8_arr_count(buf), "%s half transitions : %d\n", n, key->half_transitions);
	
		printed = true;
	}

	if (printed) {
		c8_plat_debug_out("\n");
	}
}

bool c8_app_update(C8_App_State* state) {

	char buf[256];
	if (!state->initialised)
	{
		c8_clear_struct(state->arena);
		if (c8_arena_init(&state->arena, 1024, 4)) {
			state->initialised = true;
		}
	}

	if (!state->program_loaded) {
		char f_name[] = "data\\ibm_logo.ch8";
		C8_File file = c8_plat_read_file(f_name, c8_arr_count(f_name) - 1, &state->arena);

		if (file.data != 0) {
			if (file.size <= sizeof(state->ram)) {
				state->pc = C8_PROG_ADDR;
				memcpy(state->ram + state->pc, file.data, file.size);

				c8_arena_free_all(&state->arena);
				state->program_loaded = true;
			}
		}

	}

	for (i32 i = 0; i < C8_INSTRUCTIONS_PER_FRAME; i++)
	{
		assert(state->pc < sizeof(state->ram));
		char buf[255];
		u16 instruction = c8_read_instruction( * ((u16*)(state->ram + state->pc)));

		u8 n0 = instruction >> 12;

		u8 x = (instruction & 0x0f00) >> 8;
		u8 y = (instruction & 0x00f0) >> 4;
		u8 n = (instruction & 0x000f);
		u8 nn = (instruction & 0x00ff);
		u16 nnn = (instruction & 0x0fff);

		sprintf(buf, "Instruction : %04X \n", instruction);
		//c8_plat_debug_out(buf);

		if (instruction == 0x00e0) {
			c8_plat_debug_out("Clear\n");
			c8_clear_struct(state->pixels);
		}
		else if (n0 == 0x1) {
		//	c8_plat_debug_out("Jump\n");
			state->pc = nnn;
			continue;
		}
		else if (n0 == 0x6) {
			c8_plat_debug_out("Set register\n");
			state->var_registers[x] = nn;
		}
		else if (n0 == 0x7) {
			c8_plat_debug_out("Add to register\n");
			state->var_registers[x] += nn;
		}
		else if (n0 == 0xa) {
			c8_plat_debug_out("Set index register\n");
			state->index_register = nnn;
		}
		else if (n0 == 0xd) {
			c8_plat_debug_out("Display\n");
			u16 flag_register = 0;

			u8 row_count = n;
			u16 sprite_x = state->var_registers[x];
			u16 sprite_y = state->var_registers[y];
			u8 *sprite_start = state->ram + state->index_register;

			for(i32 r = 0; r < n; r++){
				u8 sprite_row = *(sprite_start + r);
				sprintf(buf, "Row : %x \n", instruction);
				c8_plat_debug_out(buf);
				for (i32 c = 0; c < 8; c++) {
					u8 on = (sprite_row >> (7 -c)) & 0x01;
					if (on == 0x01)
					{
						if (state->pixels[sprite_y + r][sprite_x + c]) {
							state->pixels[sprite_y + r][sprite_x + c] = false;
						}
						else {
							state->pixels[sprite_y + r][sprite_x + c] = true;
						}
						flag_register = 1;
					}
					else {
						assert(on == 0x00);
					}
				}
			}

			state->var_registers[0xf] = flag_register;
		}
		else {
			c8_plat_debug_out("Unimplemented instruction\n");
			assert(false);
		}

		state->pc += 2;
	}

	bool push_frame = c8_push_frame(state);

	bool push_pixels = c8_push_pixels(state);

	for (int kp = 0; kp < c8_arr_count(state->keypad.keys); kp++)
	{
		snprintf(buf, c8_arr_count(buf), "%x", kp);
		c8_debug_keyboard((&state->keypad.keys[kp]), buf);
		state->keypad.keys[kp].started_down = state->keypad.keys[kp].ended_down;
		state->keypad.keys[kp].was_down = state->keypad.keys[kp].ended_down;
		state->keypad.keys[kp].was_lifted = false;
		state->keypad.keys[kp].was_pressed = false;
		state->keypad.keys[kp].half_transitions = 0;
	}

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
