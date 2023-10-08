#ifndef C8_APP_C
#define C8_APP_C

#include "c8_app.h"

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

bool c8_push_frame(C8_State *state)
{
	float frame_x = c8_frame_x(state);

	float frame_y = c8_frame_y(state);

	bool push1 = c8_draw_rect(
		state,
		frame_x,
		frame_y,
		C8_MONITOR_WIDTH,
		C8_FRAME_WIDTH,
		emulator_color);

	bool push2 = c8_draw_rect(
		state,
		frame_x + C8_MONITOR_WIDTH,
		frame_y,
		C8_FRAME_WIDTH,
		C8_MONITOR_HEIGHT,
		emulator_color);

	bool push3 = c8_draw_rect(
		state,
		frame_x + C8_FRAME_WIDTH,
		frame_y + C8_MONITOR_HEIGHT,
		C8_MONITOR_WIDTH,
		C8_FRAME_WIDTH,
		emulator_color

	);

	bool push4 = c8_draw_rect(
		state,
		frame_x,
		frame_y + C8_FRAME_WIDTH,
		C8_FRAME_WIDTH,
		C8_MONITOR_HEIGHT,
		emulator_color

	);

	return push1 && push2 && push3 && push4;
}

bool c8_push_color_vertex(C8_State *state, float x, float y, u8 r, u8 g, u8 b, u8 a)
{
	bool result = false;

	assert(state->color_vertex_count < c8_arr_count(state->color_vertices));

	if (state->color_vertex_count < c8_arr_count(state->color_vertices))
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

void c8_load_roam(wchar_t *filePath, C8_State *state)
{
	C8_File file = c8_plat_read_file(filePath, wcslen(filePath), &state->arena);

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

bool c8_push_glyph(C8_State *state, C8_Atlas_Glyph glyph, float x, float y, float width, float height, C8_Rgba rgb)
{

	bool push1 = c8_push_text_vertex(state, x, y, rgb.r, rgb.g, rgb.b, rgb.a, glyph.u_left, glyph.v_top);
	bool push2 = c8_push_text_vertex(state, x + width, y, rgb.r, rgb.g, rgb.b, rgb.a, glyph.u_right, glyph.v_top);
	bool push3 = c8_push_text_vertex(state, x + width, y + height, rgb.r, rgb.g, rgb.b, rgb.a, glyph.u_right, glyph.v_bottom);

	bool push4 = c8_push_text_vertex(state, x, y, rgb.r, rgb.g, rgb.b, rgb.a, glyph.u_left, glyph.v_top);
	bool push5 = c8_push_text_vertex(state, x + width, y + height, rgb.r, rgb.g, rgb.b, rgb.a, glyph.u_right, glyph.v_bottom);
	bool push6 = c8_push_text_vertex(state, x, y + height, rgb.r, rgb.g, rgb.b, rgb.a, glyph.u_left, glyph.v_bottom);

	return push1 && push2 && push3 && push4 && push5 && push6;
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

bool c8_push_text_vertex(C8_State *state, float x, float y, u8 r, u8 g, u8 b, u8 a, float u, float v)
{
	bool result = false;

	assert(state->text_vertex_count < c8_arr_count(state->text_vertices));

	if (state->text_vertex_count < c8_arr_count(state->text_vertices))
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
		result = true;
	}
	else
	{
		OutputDebugStringA("Vertex buffer size exceeded");
	}

	return result;
}

bool c8_draw_rect(C8_State *state, float x, float y, float width, float height, C8_Rgba rgb)
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

	bool push1 = c8_push_color_triangle(state, p1, p2, p3, rgb);
	bool push2 = c8_push_color_triangle(state, p1, p3, p4, rgb);

	return push1 && push2;
}

bool c8_push_pixels(C8_State *state)
{

	for (i32 r = 0; r < c8_arr_count(state->pixels); r++)
	{
		for (i32 c = 0; c < c8_arr_count(state->pixels[r]); c++)
		{
			if (state->pixels[r][c])
			{
				float frame_x = c8_frame_x(state);
				float frame_y = c8_frame_y(state);

				bool push = c8_draw_rect(
					state,
					frame_x + (c * C8_PIXEL_SIDE) + C8_FRAME_WIDTH,
					frame_y + (r * C8_PIXEL_SIDE) + C8_FRAME_WIDTH,
					C8_PIXEL_SIDE,
					C8_PIXEL_SIDE,
					emulator_color);

				if (!push)
				{
					return false;
				}
			}
		}
	}

	return true;
}

void c8_debug_keyboard(C8_Key *key, char *n)
{
	bool printed = false;
	if (key->started_down)
	{
		c8_plat_debug_printf("%s started down\n", n);
		printed = true;
	}

	if (key->was_down)
	{
		c8_plat_debug_printf("%s was down\n", n);
		printed = true;
	}

	if (key->ended_down)
	{
		c8_plat_debug_printf("%s ended down\n", n);
		printed = true;
	}
	if (key->was_pressed)
	{
		c8_plat_debug_printf("%s was pressed\n", n);
		printed = true;
	}
	if (key->was_lifted)
	{
		c8_plat_debug_printf("%s was lifted\n", n);
		printed = true;
	}

	if (key->half_transitions != 0)
	{
		c8_plat_debug_printf("%s half transitions : %d\n", n, key->half_transitions);

		printed = true;
	}

	if (printed)
	{
		c8_plat_debug_out("\n");
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

void c8_debug_display(C8_State *state)
{
	for (i32 r = 0; r < c8_arr_count(state->pixels); r++)
	{
		for (i32 c = 0; c < c8_arr_count(state->pixels[r]); c++)
		{

			if (state->pixels[r][c])
			{
				c8_plat_debug_out("x");
			}
			else
			{
				c8_plat_debug_out(".");
			}
		}
		c8_plat_debug_out("\n");
	}
}

void c8_debug_row(u8 row)
{

	for (i32 i = 0; i < 8; i++)
	{
		u8 pixel = (row >> (7 - i)) & 0x01;

		if (pixel == 1)
		{
			c8_plat_debug_out("x");
		}
		else
		{
			c8_plat_debug_out(".");
		}
	}

	c8_plat_debug_out("\n");
}

void c8_add_number_to_register(C8_State *state, u8 x, u8 nn)
{

	u8 vx = state->var_registers[x];
	u16 result = (u16)vx + (u16)nn;

	c8_plat_debug_printf("Add %2x to v%x (%2x) = %2x\n", nn, x, vx, result);
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
	c8_plat_debug_printf("Call %03x\n", nnn);
	state->stack[state->stack_pointer] = state->pc;
	state->stack_pointer++;
	if (state->stack_pointer >= c8_arr_count(state->stack))
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

bool c8_update_emulator(C8_State *state)
{

	char buf[256];
	float button_x = (state->cli_width / 2.0f) - (C8_LOAD_BUTTON_WIDTH / 2.0f);

	float button_y = C8_LOAD_BUTTON_Y;
	float button_width = C8_LOAD_BUTTON_WIDTH;
	float button_height = C8_LOAD_BUTTON_HEIGHT;

	bool is_mouse_over_button = state->mouse_position.xy.x >= button_x && state->mouse_position.xy.x <= button_x + button_width && state->mouse_position.xy.y >= button_y && state->mouse_position.xy.y <= button_y + button_width;

	if (state->load_button_down & state->mouse_buttons.left_button.was_lifted)
	{

		if (is_mouse_over_button)
		{
			wchar_t *path = c8_plat_open_file_dialog();

			if (path != NULL)
			{
				c8_load_roam(path, state);
			}
		}
		state->load_button_down = false;
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

			c8_plat_debug_printf("Pc %02x\n", state->pc);

#if 1
			sprintf(buf, "Instruction : %04X \n", instruction);
			c8_plat_debug_out(buf);
#endif

			if (instruction == 0x00e0)
			{
				c8_plat_debug_out("Clear\n");
				c8_clear_struct(state->pixels);
			}
			else if (op == 0x1)
			{
				c8_plat_debug_printf("Jump to %03X\n", nnn);

				state->pc = nnn;
			}
			else if (op == 0x2)
			{

				c8_call(state, nnn);
			}
			else if (instruction == 0x00ee)
			{
				c8_plat_debug_out("Return\n");

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
				c8_plat_debug_printf("Set register %X to %02X\n", x, nn);
				state->var_registers[x] = nn;
			}
			else if (op == 0x7)
			{

				c8_add_number_to_register(state, x, nn);
			}
			else if (op == 0xa)
			{
				c8_plat_debug_printf("Set index register to %03x\n", nnn);
				state->index_register = nnn;
			}
			else if (op == 0xc)
			{
				c8_plat_debug_out("Random\n");

				int random = rand();
				u8 result = (u8)((u16)random & nn);
				state->var_registers[x] = result;
			}
			else if (op == 0xd)
			{
				u8 flag_register = 0;
				u16 sprite_x = state->var_registers[x] % C8_PIXEL_COLS;
				u16 sprite_y = state->var_registers[y] % C8_PIXEL_ROWS;
				u8 *sprite_start = state->ram + state->index_register;

				c8_plat_debug_printf(
					buf,
					c8_arr_count(buf),
					"Display sprite at %04x at x = v%x (%x), y = v%x (%x) \n",
					state->index_register,
					x,
					sprite_x,
					y,
					sprite_y);

				for (i32 r = 0; r < n && sprite_y + r < C8_PIXEL_ROWS; r++)
				{
					u8 sprite_row = *(sprite_start + r);
					// sprintf(buf, "Row : %x \n", instruction);
					// c8_plat_debug_out(buf);
					// c8_debug_row(sprite_row);
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

#if 0
			c8_debug_display(state);
			c8_plat_debug_out("\n");
#endif
			}
			else if (op == 0x3)
			{
#if 0
			c8_plat_debug_printf( "Skip if v%x (%x) equals %02x\n", x, vx, nn);
#endif
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
#if 0

			c8_plat_debug_printf( "Skip if v%x (%x) not equals %02x\n", x, vx, nn);

#endif
			}
			else if ((instruction & 0xf00f) == 0x5000)
			{
				c8_plat_debug_out("Skip if x equals y\n");

				if (state->var_registers[x] == state->var_registers[y])
				{
					state->pc += 2;
				}
			}
			else if ((instruction & 0xf00f) == 0x9000)
			{
				c8_plat_debug_out("Skip if x not equals y\n");

				if (state->var_registers[x] != state->var_registers[y])
				{
					state->pc += 2;
				}
			}
			else if ((instruction & 0xf00f) == 0x8000)
			{
				c8_plat_debug_out("Set x register to value of y register\n");
				state->var_registers[x] = state->var_registers[y];
			}
			else if ((instruction & 0xf00f) == 0x8001)
			{
				c8_plat_debug_out("Binary or\n");
				state->var_registers[x] = state->var_registers[x] | state->var_registers[y];
			}
			else if ((instruction & 0xf00f) == 0x8002)
			{
				u8 result = vx & vy;
				c8_plat_debug_printf("v%x = v%x (%x) & v%x (%x) = %x\n", x, x, vx, y, vy, result);
				state->var_registers[x] = result;
			}
			else if ((instruction & 0xf00f) == 0x8003)
			{
				c8_plat_debug_out("Binary xor\n");
				state->var_registers[x] = state->var_registers[x] ^ state->var_registers[y];
			}
			else if ((instruction & 0xf00f) == 0x8004)
			{
				c8_plat_debug_out("x + y\n");
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
				c8_plat_debug_out("Shift right\n");

				u8 bit = state->var_registers[x] & 0x1;
				state->var_registers[x] = state->var_registers[x] >> 1;
				state->var_registers[C8_FLAG_REG] = bit;
			}
			else if ((instruction & 0xf00f) == 0x8005)
			{
				c8_plat_debug_out("x - y\n");

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
				c8_plat_debug_out("y - x\n");

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
				c8_plat_debug_out("Shift left\n");

				u8 bit = state->var_registers[x] >> 7;
				state->var_registers[x] = state->var_registers[x] << 1;
				state->var_registers[C8_FLAG_REG] = bit;
			}
			else if ((instruction & 0xf0ff) == 0xE09E)
			{
				c8_plat_debug_printf("Skip if %x key is pressed\n", x);
				u8 key = state->var_registers[x];

				if (state->keypad.keys[key].ended_down)
				{
					state->pc += 2;
				}
			}
			else if ((instruction & 0xf0ff) == 0xE0A1)
			{
				c8_plat_debug_printf("Skip if %x key is not pressed\n", state->var_registers[x]);
				u8 key = state->var_registers[x];
				if (!state->keypad.keys[key].ended_down)
				{
					state->pc += 2;
				}
			}
			else if ((instruction & 0xf0ff) == 0xF00A)
			{
				c8_plat_debug_printf("Wait for %x key\n", x);

				if (!state->keypad.keys[x].ended_down)
				{
					state->pc -= 2;
				}
			}
			else if ((instruction & 0xf0ff) == 0xF007)
			{
				c8_plat_debug_out("Set register to delay timer\n");
				state->var_registers[x] = state->delay_timer;
			}
			else if ((instruction & 0xf0ff) == 0xF015)
			{
				c8_plat_debug_out("Set delay timer\n");
				state->delay_timer = state->var_registers[x];
			}
			else if ((instruction & 0xf0ff) == 0xF018)
			{
				c8_plat_debug_out("Set sound timer\n");
				state->sound_timer = state->var_registers[x];
			}
			else if ((instruction & 0xf0ff) == 0xF01E)
			{
				u16 result = state->index_register + vx;

				if (result > 0x0fff)
				{
					state->var_registers[C8_FLAG_REG] = 1;
					result &= 0x0fff;
				}

				c8_plat_debug_printf("Add v%x (%x) to index (%x) giving %x\n",
									 x,
									 vx,
									 state->index_register,
									 result);

				state->index_register = result;
			}
			else if ((instruction & 0xf0ff) == 0xF029)
			{
				c8_plat_debug_printf("Point index  to font v%x (%x) \n", x, vx);
				u8 c = (state->var_registers[x]) & 0x0f;
				state->index_register = C8_FONT_ADDR + (C8_FONT_SIZE * c);
			}
			else if ((instruction & 0xf0ff) == 0xF033)
			{
				c8_plat_debug_out("Decimal conversion\n");
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

				u16 start = state->index_register;

				c8_plat_debug_printf(
					"Store registers 0 to %x at memory  location %2x\n", x, start);

				for (int reg_i = 0; reg_i <= x; reg_i++)
				{
					state->ram[start + reg_i] = state->var_registers[reg_i];
					c8_plat_debug_printf(
						"v%x = %x\n", reg_i, state->var_registers[reg_i]);
				}
			}
			else if ((instruction & 0xf0ff) == 0xF065)
			{
				u16 start = state->index_register;

				c8_plat_debug_printf(
					"Load registers 0 to %x from memory location %2x\n", x, start);

				for (i32 reg_i = 0; reg_i <= x; reg_i++)
				{
					state->var_registers[reg_i] = state->ram[start + reg_i];
					c8_plat_debug_printf(
						"v%x = %x\n", reg_i, state->var_registers[reg_i]);
				}
			}
			else
			{
				c8_plat_debug_out("Unimplemented instruction\n");
				assert(false);
			}
		}
	}

	bool push_frame = c8_push_frame(state);

	bool push_pixels = c8_push_pixels(state);

	for (int kp = 0; kp < c8_arr_count(state->keypad.keys); kp++)
	{
#if 1
		snprintf(buf, c8_arr_count(buf), "%x", kp);
		c8_debug_keyboard((&state->keypad.keys[kp]), buf);
#endif
		C8_Key *k = &(state->keypad.keys[kp]);
		c8_reset_key(k);
	}

#if 0
	c8_debug_keyboard((&state->control_keys.esc), "Esc");
	c8_debug_keyboard((&state->control_keys.p), "P");
	c8_debug_keyboard((&state->control_keys.space), "Space");
	c8_debug_keyboard((&state->control_keys.enter), "Enter");
#endif
	for (int ck = 0; ck < c8_arr_count(state->control_keys.keys); ck++)
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

	return push_frame && push_pixels;
}

bool c8_app_update(C8_State *state)
{
	state->color_vertex_count = 0;
	state->text_vertex_count = 0;

	return c8_update_emulator(state);
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
		OutputDebugStringA("Arena initialization failed");
		assert(false);
		arena = 0;
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
void c8_file_list_init(C8_String_List *file_list, C8_Arena *arena)
{
	file_list->arena = arena;
	file_list->file_names = c8_arena_alloc(arena, C8_FILE_LIST_INITIAL_CAPACITY * sizeof(*(file_list->file_names)));
	if (file_list->file_names != NULL)
	{
		file_list->capacity = C8_FILE_LIST_INITIAL_CAPACITY;
		file_list->count = 0;
	}
}

bool c8_push_file_name(C8_String_List *file_list, wchar_t *file_name, size_t name_length)
{

	if (file_list->count >= file_list->capacity)
	{
		size_t new_capacity = file_list->capacity * 2;
		C8_String *new_buffer = c8_arena_alloc(file_list->arena, new_capacity * sizeof(*new_buffer));
		if (!new_buffer)
		{
			return false;
		}
		memcpy(new_buffer, file_list->file_names, file_list->capacity * sizeof(*new_buffer));
		file_list->capacity = new_capacity;
		file_list->file_names = new_buffer;
	}

	wchar_t *name_buffer = c8_arena_alloc(file_list->arena, name_length * sizeof(wchar_t));

	if (!name_buffer)
	{
		return false;
	}

	memcpy(name_buffer, file_name, name_length);
	file_list->file_names[file_list->count].length = name_length;
	file_list->file_names[file_list->count].text = name_buffer;
	file_list->count++;

	return true;
}

#endif // !C8_APP_C
