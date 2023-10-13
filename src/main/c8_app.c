#ifndef C8_APP_C
#define C8_APP_C

#include "c8_win.h"

u16 c8_read_instruction(u16 bytes)
{
	u16 result = ((bytes & 0x00ff) << 8) | ((bytes & 0xff00) >> 8);
	return result;
}

float c8_frame_x(C8_State *state)
{
	float result = (state->cli_width / 2) - (C8_MONITOR_WIDTH / 2);

	return result;
}

float c8_frame_y(C8_State *state)
{
	float result = (state->cli_height / 2) - (C8_MONITOR_HEIGHT / 2);

	return result;
}

void c8_push_frame(C8_State *state)
{
	float frame_x = c8_frame_x(state);

	float frame_y = c8_frame_y(state);

	c8_draw_rect(
		state,
		frame_x,
		frame_y,
		C8_MONITOR_WIDTH,
		C8_FRAME_WIDTH,
		emulator_color);

	c8_draw_rect(
		state,
		frame_x + C8_MONITOR_WIDTH,
		frame_y,
		C8_FRAME_WIDTH,
		C8_MONITOR_HEIGHT,
		emulator_color);

	c8_draw_rect(
		state,
		frame_x + C8_FRAME_WIDTH,
		frame_y + C8_MONITOR_HEIGHT,
		C8_MONITOR_WIDTH,
		C8_FRAME_WIDTH,
		emulator_color

	);

	c8_draw_rect(
		state,
		frame_x,
		frame_y + C8_FRAME_WIDTH,
		C8_FRAME_WIDTH,
		C8_MONITOR_HEIGHT,
		emulator_color

	);
}

bool c8_push_color_vertex(C8_State *state, float x, float y, u8 r, u8 g, u8 b, u8 a)
{
	bool result = false;

	assert(state->color_vertex_count < C8_ARRCOUNT(state->color_vertices));

	if (state->color_vertex_count < C8_ARRCOUNT(state->color_vertices))
	{
		state->color_vertices[state->color_vertex_count].x = x;
		state->color_vertices[state->color_vertex_count].y = y;

		C8_Rgba color = {0};
		color.r = r;
		color.g = g;
		color.b = b;
		color.a = a;
		state->color_vertices[state->color_vertex_count].color = color;
		state->color_vertex_count++;
		result = true;
	}
	else
	{
		OutputDebugStringA("Vertex buffer size exceeded");
	}

	return result;
}

void c8_load_roam(char const *const filePath, C8_State *state)
{
	C8_File file = c8_plat_read_file(filePath, &state->arena);

	if (file.data != 0)
	{
		if (file.size <= sizeof(state->ram))
		{

			memset(state->ram, 0, sizeof(state->ram));
			memset(state->pixels, 0, sizeof(state->pixels));
			state->pc = C8_PROG_ADDR;
			memcpy(state->ram + state->pc, file.data, file.size);

			state->program_loaded = true;

			c8_arena_free_all(&state->arena);

			const u8 font_sprites[C8_FONT_SIZE * C8_FONT_COUNT] = {
				0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
				0x20, 0x60, 0x20, 0x20, 0x70, // 1
				0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
				0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
				0x90, 0x90, 0xF0, 0x10, 0x10, // 4
				0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
				0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
				0xF0, 0x10, 0x20, 0x40, 0x40, // 7
				0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
				0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
				0xF0, 0x90, 0xF0, 0x90, 0x90, // A
				0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
				0xF0, 0x80, 0x80, 0x80, 0xF0, // C
				0xE0, 0x90, 0x90, 0x90, 0xE0, // D
				0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
				0xF0, 0x80, 0xF0, 0x80, 0x80  // F
			};

			memcpy(state->ram + C8_FONT_ADDR, font_sprites, sizeof(font_sprites));
		}
	}
}

bool c8_push_text_triangle(C8_State *state, C8_V2 p1, C8_V2 p2, C8_V2 p3, C8_Rgba rgb, float u1, float v1, float u2, float v2, float u3, float v3)
{
	bool push1 = c8_push_text_vertex(state, p1.xy.x, p1.xy.y, rgb.r, rgb.g, rgb.b, rgb.a, u1, v1);
	bool push2 = c8_push_text_vertex(state, p2.xy.x, p2.xy.y, rgb.r, rgb.g, rgb.b, rgb.a, u2, v2);
	bool push3 = c8_push_text_vertex(state, p3.xy.x, p3.xy.y, rgb.r, rgb.g, rgb.b, rgb.a, u3, v3);

	return push1 && push2 && push3;
}

void c8_push_glyph(C8_State *state, C8_Atlas_Glyph glyph, float x, float y, float width, float height, C8_Rgba rgb)
{

	c8_push_text_vertex(state, x, y, rgb.r, rgb.g, rgb.b, rgb.a, glyph.u_left, glyph.v_top);
	c8_push_text_vertex(state, x + width, y, rgb.r, rgb.g, rgb.b, rgb.a, glyph.u_right, glyph.v_top);
	c8_push_text_vertex(state, x + width, y + height, rgb.r, rgb.g, rgb.b, rgb.a, glyph.u_right, glyph.v_bottom);

	c8_push_text_vertex(state, x, y, rgb.r, rgb.g, rgb.b, rgb.a, glyph.u_left, glyph.v_top);
	c8_push_text_vertex(state, x + width, y + height, rgb.r, rgb.g, rgb.b, rgb.a, glyph.u_right, glyph.v_bottom);
	c8_push_text_vertex(state, x, y + height, rgb.r, rgb.g, rgb.b, rgb.a, glyph.u_left, glyph.v_bottom);
}

#if 0

bool c8_push_text(C8_State *state, char *text, size_t text_length, float x, float y, C8_Text_Size text_size, C8_Rgba rgb)
{
	float glyph_x = x;

	for (size_t i = 0; i < text_length; i++)
	{
		char c = text[i];

		i32 glyph_index = c - C8_FIRST_CHAR;
		C8_Atlas_Glyph glyph = state->atlas_header.glyphs[glyph_index];

		float glyph_v_height = glyph.v_bottom - glyph.v_top;

		float glyph_height_pixels = glyph_v_height * text_size.vertical_scaling;

		float glyph_y = y + text_size.height_pixels - glyph_height_pixels;

		float width = (glyph.u_right - glyph.u_left) * text_size.horizontal_scaling;

		float height = (glyph.v_bottom - glyph.v_top) * text_size.vertical_scaling;

		if (!c8_push_glyph(state, glyph, glyph_x, glyph_y, width, height, rgb))
		{
			return false;
		}

		glyph_x += glyph.advancement * text_size.horizontal_scaling;
	}

	return true;
}

#endif

void c8_draw_text(C8_State *state, char *text, float x, float y, float height, C8_Rgba rgba)
{

	float x_offset = 0;

	char c = 0;

	float scale = height / state->atlas_header.height;

	while (*text)
	{
		c = *text;
		C8_Atlas_Glyph glyph = state->atlas_header.glyphs[c - C8_FIRST_CHAR];
		c8_push_glyph(state, glyph, x + x_offset, y + glyph.y_offset * scale, glyph.width * scale, glyph.height * scale, rgba);
		x_offset += glyph.advancement * scale;
		text++;
	}
}

void c8_push_text_vertex(C8_State *state, float x, float y, u8 r, u8 g, u8 b, u8 a, float u, float v)
{
	assert(state->text_vertex_count < C8_ARRCOUNT(state->text_vertices));

	if (state->text_vertex_count < C8_ARRCOUNT(state->text_vertices))
	{
		state->text_vertices[state->text_vertex_count].x = x;
		state->text_vertices[state->text_vertex_count].y = y;
		state->text_vertices[state->text_vertex_count].u = u;
		state->text_vertices[state->text_vertex_count].v = v;
		C8_Rgba color = {0};
		color.r = r;
		color.g = g;
		color.b = b;
		color.a = a;
		state->text_vertices[state->text_vertex_count].color = color;
		state->text_vertex_count++;
	}
	else
	{
		C8_LOG_ERROR("Vertex buffer size exceeded");
	}
}

void c8_draw_rect(C8_State *state, float x, float y, float width, float height, C8_Rgba rgb)
{
	C8_V2 p1 = {0};
	p1.xy.x = x;
	p1.xy.y = y;

	C8_V2 p2 = {0};
	p2.xy.x = x + width;
	p2.xy.y = y;

	C8_V2 p3 = {0};
	p3.xy.x = x + width;
	p3.xy.y = y + height;

	C8_V2 p4 = {0};
	p4.xy.x = x;
	p4.xy.y = y + height;

	c8_push_color_triangle(state, p1, p2, p3, rgb);
	c8_push_color_triangle(state, p1, p3, p4, rgb);
}

void c8_push_pixels(C8_State *state)
{

	for (i32 r = 0; r < C8_ARRCOUNT(state->pixels); r++)
	{
		for (i32 c = 0; c < C8_ARRCOUNT(state->pixels[r]); c++)
		{
			if (state->pixels[r][c])
			{
				float frame_x = c8_frame_x(state);
				float frame_y = c8_frame_y(state);

				c8_draw_rect(
					state,
					frame_x + (c * C8_PIXEL_SIDE) + C8_FRAME_WIDTH,
					frame_y + (r * C8_PIXEL_SIDE) + C8_FRAME_WIDTH,
					C8_PIXEL_SIDE,
					C8_PIXEL_SIDE,
					emulator_color);
			}
		}
	}
}

void c8_reset_key(C8_Key *k)
{
	k->started_down = k->ended_down;
	k->was_down = k->ended_down;
	k->was_lifted = false;
	k->was_pressed = false;
	k->half_transitions = 0;
}

void c8_add_number_to_register(C8_State *state, u8 x, u8 nn)
{

	u8 vx = state->var_registers[x];
	u16 result = (u16)vx + (u16)nn;

	if (result > 0xff)
	{
		state->var_registers[C8_FLAG_REG] = 1;
	}
	else
	{
		state->var_registers[C8_FLAG_REG] = 0;
	}

	state->var_registers[x] = (result & 0xff);
}

bool c8_call(C8_State *state, u16 nnn)
{
	state->stack[state->stack_pointer] = state->pc;
	state->stack_pointer++;
	if (state->stack_pointer >= C8_ARRCOUNT(state->stack))
	{
		assert(false);
		// TODO: handle stackoverflow
	}
	state->pc = nnn;

	return true;
}

float c8_max_v_height(char *text, size_t text_length, C8_Atlas_Header *atlas_header)
{

	float result = 0.0f;
	for (size_t i = 0; i < text_length; i++)
	{
		char c = text[i];

		i32 glyph_index = c - C8_FIRST_CHAR;
		C8_Atlas_Glyph glyph = atlas_header->glyphs[glyph_index];

		float v_height = glyph.v_bottom - glyph.v_top;

		if (v_height > result)
		{
			result = v_height;
		}
	}

	return result;
}

C8_Text_Size c8_text_scale_for_max_size(char *text, size_t text_length, float max_width_pixels, float max_height_pixels, C8_Atlas_Header *atlas_header)
{

	float max_v_height = c8_max_v_height(text, text_length, atlas_header);

	float vertical_scaling_factor = max_height_pixels / max_v_height;

	float aspect_ratio = ((float)atlas_header->width) / ((float)atlas_header->height);

	float horizontal_scaling_factor = vertical_scaling_factor * aspect_ratio;

	float height_pixels = max_height_pixels;

	float width_pixels = 0;

	for (size_t i = 0; i < text_length; i++)
	{
		char c = text[i];

		i32 glyph_index = c - C8_FIRST_CHAR;
		C8_Atlas_Glyph glyph = atlas_header->glyphs[glyph_index];

		if (i < text_length - 1)
		{
			width_pixels += glyph.advancement * horizontal_scaling_factor;
		}
		else
		{
			width_pixels += (glyph.u_right - glyph.u_left) * horizontal_scaling_factor;
		}
	}

	if (width_pixels > max_width_pixels)
	{
		float rescaling = (max_width_pixels / width_pixels);
		horizontal_scaling_factor *= rescaling;
		vertical_scaling_factor *= rescaling;
		width_pixels = max_width_pixels;
		height_pixels *= rescaling;
	}

	C8_Text_Size scaling;

	scaling.horizontal_scaling = horizontal_scaling_factor;
	scaling.vertical_scaling = vertical_scaling_factor;
	scaling.width_pixels = width_pixels;
	scaling.height_pixels = height_pixels;

	return scaling;
}

void c8_draw_button(
	C8_State *state,
	float button_x,
	float button_y,
	float button_width,
	float button_height)
{

	C8_Rgba button_color = {0, 0, 0, 255};

	c8_draw_rect(state, button_x, button_y, button_width, button_height, button_color);

	C8_Rgba text_color = {255, 0, 0, 255};

	c8_draw_text(state, "Load", button_x + 20.0f, button_y + 10.0f, 30.0f, text_color);
}

void c8_update_emulator(C8_State *state)
{
	float button_x = (state->cli_width / 2.0f) - (C8_LOAD_BUTTON_WIDTH / 2.0f);

	float button_y = C8_LOAD_BUTTON_Y;
	float button_width = C8_LOAD_BUTTON_WIDTH;
	float button_height = C8_LOAD_BUTTON_HEIGHT;

	bool is_mouse_over_button = state->mouse_position.xy.x >= button_x && state->mouse_position.xy.x <= button_x + button_width && state->mouse_position.xy.y >= button_y && state->mouse_position.xy.y <= button_y + button_width;

	if (state->load_button_down & state->mouse_buttons.left_button.was_lifted)
	{

		if (is_mouse_over_button)
		{
			c8_load_from_file_dialog(state);
		}
	}

	if (state->mouse_buttons.left_button.was_pressed && is_mouse_over_button)
	{
		state->load_button_down = true;
	}

	c8_draw_button(state, button_x, button_y, button_width, button_height);

	if (state->program_loaded)
	{
		for (i32 i = 0; i < C8_INSTRUCTIONS_PER_FRAME; i++)
		{
			assert(state->pc < sizeof(state->ram));
			u16 instruction = c8_read_instruction(*((u16 *)(state->ram + state->pc)));

			u8 op = instruction >> 12;

			u8 x = (instruction & 0x0f00) >> 8;
			u8 y = (instruction & 0x00f0) >> 4;
			u8 n = (instruction & 0x000f);
			u8 nn = (instruction & 0x00ff);
			u16 nnn = (instruction & 0x0fff);

			u8 vx = state->var_registers[x];
			u8 vy = state->var_registers[y];

			state->pc += 2;

			if (instruction == 0x00e0)
			{
				// Clear
				memset(state->pixels, 0, sizeof(state->pixels));
			}
			else if (op == 0x1)
			{
				// Jump

				state->pc = nnn;
			}
			else if (op == 0x2)
			{
				// Call
				c8_call(state, nnn);
			}
			else if (instruction == 0x00ee)
			{
				// Return

				state->stack_pointer--;
				if (state->stack_pointer < 0)
				{
					assert(false);
					// TODO: handle stackundeflow (?)
				}
				state->pc = state->stack[state->stack_pointer];
			}
			else if (op == 0x6)
			{
				// Set register
				state->var_registers[x] = nn;
			}
			else if (op == 0x7)
			{
				// Add number to register

				c8_add_number_to_register(state, x, nn);
			}
			else if (op == 0xa)
			{
				// Set index to register
				state->index_register = nnn;
			}
			else if (op == 0xc)
			{
				// Random

				int random = rand();
				u8 result = (u8)((u16)random & nn);
				state->var_registers[x] = result;
			}
			else if (op == 0xd)
			{
				// Display sprite

				u8 flag_register = 0;
				u16 sprite_x = state->var_registers[x] % C8_PIXEL_COLS;
				u16 sprite_y = state->var_registers[y] % C8_PIXEL_ROWS;
				u8 *sprite_start = state->ram + state->index_register;

				for (i32 r = 0; r < n && sprite_y + r < C8_PIXEL_ROWS; r++)
				{
					u8 sprite_row = *(sprite_start + r);

					for (i32 c = 0; c < 8 && sprite_x + c < C8_PIXEL_COLS; c++)
					{
						u8 on = (sprite_row >> (7 - c)) & 0x01;
						if (on == 0x01)
						{
							if (state->pixels[sprite_y + r][sprite_x + c])
							{
								state->pixels[sprite_y + r][sprite_x + c] = false;
							}
							else
							{
								state->pixels[sprite_y + r][sprite_x + c] = true;
							}
							flag_register = 1;
						}
						else
						{
							assert(on == 0x00);
						}
					}
				}

				state->var_registers[C8_FLAG_REG] = flag_register;
			}
			// Skip
			else if (op == 0x3)
			{

				if (vx == nn)
				{
					state->pc += 2;
				}
			}
			else if (op == 0x4)
			{
				if (vx != nn)
				{
					state->pc += 2;
				}
			}
			else if ((instruction & 0xf00f) == 0x5000)
			{
				if (state->var_registers[x] == state->var_registers[y])
				{
					state->pc += 2;
				}
			}
			else if ((instruction & 0xf00f) == 0x9000)
			{
				if (state->var_registers[x] != state->var_registers[y])
				{
					state->pc += 2;
				}
			}
			else if ((instruction & 0xf00f) == 0x8000)
			{
				// Set register to register
				state->var_registers[x] = state->var_registers[y];
			}
			else if ((instruction & 0xf00f) == 0x8001)
			{
				// Binary or
				state->var_registers[x] = state->var_registers[x] | state->var_registers[y];
			}
			else if ((instruction & 0xf00f) == 0x8002)
			{
				u8 result = vx & vy;
				// Binary and
				state->var_registers[x] = result;
			}
			else if ((instruction & 0xf00f) == 0x8003)
			{
				// Binary xor
				state->var_registers[x] = state->var_registers[x] ^ state->var_registers[y];
			}
			else if ((instruction & 0xf00f) == 0x8004)
			{
				// Add
				u16 result = state->var_registers[x] + state->var_registers[y];
				if (result > 0xff)
				{
					state->var_registers[C8_FLAG_REG] = 1;
				}
				else
				{
					state->var_registers[C8_FLAG_REG] = 0;
				}
				state->var_registers[x] = (u8)result;
			}
			else if ((instruction & 0xf00f) == 0x8006)
			{
				// Shift right

				u8 bit = state->var_registers[x] & 0x1;
				state->var_registers[x] = state->var_registers[x] >> 1;
				state->var_registers[C8_FLAG_REG] = bit;
			}
			else if ((instruction & 0xf00f) == 0x8005)
			{
				// x - y

				if (state->var_registers[x] > state->var_registers[y])
				{
					state->var_registers[C8_FLAG_REG] = 1;
				}

				if (state->var_registers[x] < state->var_registers[y])
				{
					state->var_registers[C8_FLAG_REG] = 0;
				}
				state->var_registers[x] =
					state->var_registers[x] -
					state->var_registers[y];
			}
			else if ((instruction & 0xf00f) == 0x8007)
			{
				// y - x

				if (state->var_registers[y] > state->var_registers[x])
				{
					state->var_registers[C8_FLAG_REG] = 1;
				}

				if (state->var_registers[y] < state->var_registers[x])
				{
					state->var_registers[C8_FLAG_REG] = 0;
				}
				state->var_registers[x] =
					state->var_registers[y] -
					state->var_registers[x];
			}
			else if ((instruction & 0xf00f) == 0x800e)
			{
				// Shift left

				u8 bit = state->var_registers[x] >> 7;
				state->var_registers[x] = state->var_registers[x] << 1;
				state->var_registers[C8_FLAG_REG] = bit;
			}
			else if ((instruction & 0xf0ff) == 0xE09E)
			{
				// Skip if key pressed
				u8 key = state->var_registers[x];

				if (state->keypad.keys[key].ended_down)
				{
					state->pc += 2;
				}
			}
			else if ((instruction & 0xf0ff) == 0xE0A1)
			{
				// Skip if key is not pressed
				u8 key = state->var_registers[x];
				if (!state->keypad.keys[key].ended_down)
				{
					state->pc += 2;
				}
			}
			else if ((instruction & 0xf0ff) == 0xF00A)
			{
				// Wait for key

				if (!state->keypad.keys[x].ended_down)
				{
					state->pc -= 2;
				}
			}
			else if ((instruction & 0xf0ff) == 0xF007)
			{
				// Get delay timer
				state->var_registers[x] = state->delay_timer;
			}
			else if ((instruction & 0xf0ff) == 0xF015)
			{
				// Set delay timer
				state->delay_timer = state->var_registers[x];
			}
			else if ((instruction & 0xf0ff) == 0xF018)
			{
				// Set sound timer
				state->sound_timer = state->var_registers[x];
			}
			else if ((instruction & 0xf0ff) == 0xF01E)
			{

				// Add to index
				u16 result = state->index_register + vx;

				if (result > 0x0fff)
				{
					state->var_registers[C8_FLAG_REG] = 1;
					result &= 0x0fff;
				}

				state->index_register = result;
			}
			else if ((instruction & 0xf0ff) == 0xF029)
			{
				// Point index to font
				u8 c = (state->var_registers[x]) & 0x0f;
				state->index_register = C8_FONT_ADDR + (C8_FONT_SIZE * c);
			}
			else if ((instruction & 0xf0ff) == 0xF033)
			{
				// Decimal conversion
				u16 start = state->index_register;
				u8 dividend = state->var_registers[x];
				u8 divisor = 100;
				for (size_t digit_i = 0; digit_i < 3; digit_i++)
				{
					state->ram[start + digit_i] = dividend / divisor;
					dividend %= divisor;
					divisor /= 10;
				}
			}
			else if ((instruction & 0xf0ff) == 0xF055)
			{

				// Store register
				u16 start = state->index_register;

				for (int reg_i = 0; reg_i <= x; reg_i++)
				{
					state->ram[start + reg_i] = state->var_registers[reg_i];
				}
			}
			else if ((instruction & 0xf0ff) == 0xF065)
			{
				// Load register

				u16 start = state->index_register;

				for (i32 reg_i = 0; reg_i <= x; reg_i++)
				{
					state->var_registers[reg_i] = state->ram[start + reg_i];
				}
			}
			else
			{
				C8_LOG_ERROR("Unimplemented instruction\n");
				assert(false);
			}
		}
	}

	c8_push_frame(state);

	c8_push_pixels(state);

	for (int kp = 0; kp < C8_ARRCOUNT(state->keypad.keys); kp++)
	{

		C8_Key *k = &(state->keypad.keys[kp]);
		c8_reset_key(k);
	}

#if 0
	c8_debug_keyboard((&state->control_keys.esc), "Esc");
	c8_debug_keyboard((&state->control_keys.p), "P");
	c8_debug_keyboard((&state->control_keys.space), "Space");
	c8_debug_keyboard((&state->control_keys.enter), "Enter");
#endif
	for (int ck = 0; ck < C8_ARRCOUNT(state->control_keys.keys); ck++)
	{
		C8_Key *k = &(state->control_keys.keys[ck]);
		c8_reset_key(k);
	}

	c8_reset_key(&(state->mouse_buttons.left_button));
	c8_reset_key(&(state->mouse_buttons.right_button));

	state->frame_count++;

	if (state->sound_timer > 0)
	{
		state->sound_timer--;
		state->should_beep = true;
	}
	else
	{
		state->should_beep = false;
	}

	if (state->delay_timer > 0)
	{
		state->delay_timer--;
	}
}

void c8_app_update(C8_State *state)
{
	state->color_vertex_count = 0;
	state->text_vertex_count = 0;

	c8_update_emulator(state);
}

bool c8_arena_init(C8_Arena *arena, psz size, i32 alignement)
{
	bool result = false;

	assert(size % alignement == 0);

	void *mem = c8_plat_allocate(size);

	if (mem != 0)
	{
		arena->max_bytes = size;
		arena->alignement = alignement;
		arena->offset = 0;
		arena->data = mem;
		result = true;
	}
	else
	{
		C8_LOG_ERROR("Arena initialization failed");
	}

	return result;
}

void *c8_arena_alloc(C8_Arena *arena, psz size)
{
	assert(arena != 0);

	void *result = 0;

	psz eff_size = size;
	if (eff_size % arena->alignement != 0)
	{
		eff_size += arena->alignement - (eff_size % arena->alignement);
	}

	bool big_enough = arena->offset + eff_size <= arena->max_bytes;

	assert(big_enough);

	if (big_enough)
	{
		result = ((u8 *)arena->data) + arena->offset;
		arena->offset += eff_size;
	}

	return result;
}

void c8_arena_free_all(C8_Arena *arena)
{
	assert(arena != 0);

	arena->offset = 0;
}

#endif // !C8_APP_C
