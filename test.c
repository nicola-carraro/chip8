#define _CRT_SECURE_NO_WARNINGS

#define C8_TEST

#include <stdio.h>

#include "chip8.c"

#define c8_test_def(a) (printf("TEST : %s\n", a))

void c8_test_arena_init()
{
	c8_test_def("initialize arena");

	C8_Arena arena;

	i32 alignement = 4;

	psz size = 100;

	c8_arena_init(&arena, size, alignement);

	assert(arena.alignement == alignement);

	assert(arena.max_bytes == size);

	assert(arena.offset == 0);
}

void c8_test_arena_alloc_free()
{
	c8_test_def("allocate and free with arena");

	C8_Arena arena1;

	i32 alignement = 4;

	psz size = 1024;

	c8_arena_init(&arena1, size, alignement);

	void *mem1 = c8_arena_alloc(&arena1, 101);

	assert(arena1.offset == 104);

	void *mem2 = c8_arena_alloc(&arena1, 4);

	assert(arena1.offset == 108);

	assert(((char *)mem2 - (char *)mem1) == 104);

	c8_arena_free_all(&arena1);

	assert(arena1.offset == 0);
}

void c8_test_call()
{
	c8_test_def("Basic call");

	C8_State state = {0};

	state.pc = C8_PROG_ADDR;

	u16 nnn = 255;

	c8_call(&state, nnn);

	assert(state.pc == nnn && "Pc does not point to called address");

	assert(state.stack_pointer == 1 && "Stack pointer not incremented");

	assert(state.stack[0] == C8_PROG_ADDR && "Address not pushed to stack");
}

void c8_test_add_number_to_register_no_overflow()
{
	c8_test_def("Add number to register without overflow");

	C8_State state = {0};

	state.var_registers[0] = 8;

	state.var_registers[C8_FLAG_REG] = 123;

	c8_add_number_to_register(&state, 0, 9);

	assert(state.var_registers[0] == 17 && "Addition result is incorrect");
	assert(state.var_registers[C8_FLAG_REG] == 0 && "Overflow flag incorrect");
}

void c8_test_add_number_to_register_overflow()
{

	c8_test_def("Add number to register with overflow");
	C8_State state = {0};

	state.var_registers[0] = 252;

	state.var_registers[C8_FLAG_REG] = 222;

	c8_add_number_to_register(&state, 0, 5);

	assert(state.var_registers[0] == 1 && "Addition result is incorrect");
	assert(state.var_registers[C8_FLAG_REG] == 1 && "Overflow flag incorrect");
}

void c8_test_clear()
{
	c8_test_def("Clear emulator's screen");
	C8_State state = {0};
	state.pixels[10][10] = true;
	state.pixels[0][0] = true;
	c8_clear(&state);

	for (int i = 0; i < C8_ARRCOUNT(state.pixels); i++)
	{
		for (int j = 0; j < C8_ARRCOUNT(state.pixels[0]); j++)
		{
			assert(state.pixels[i][j] == false && "Pixel not cleared");
		}
	}
}

void c8_test_return()
{
	c8_test_def("Return from call");
	C8_State state = {0};
	state.stack_pointer = 10;
	state.stack[9] = 3;
	c8_return(&state);
	assert(state.stack_pointer == 9 && "Stack pointer not decremented");
	assert(state.pc == 3 && "Program counter not updated");
}

void c8_test_return_underflow()
{
	c8_test_def("Return that causes stack underflow");
	C8_State state = {0};
	state.program_loaded = true;
	state.stack_pointer = 0;
	c8_return(&state);
	assert(!state.program_loaded && "Execution not stopped");
}

void c8_test_display_sprite()
{
	c8_test_def("Display sprite");

	C8_State state = {0};
	u16 source_index = 8;
	u8 *source = state.ram + source_index;
	memset(source, 0xff, 8);

	u8 x = 0;
	u8 y = 1;
	u8 n = 8;

	state.var_registers[x] = 8;
	state.var_registers[y] = 8;

	state.pixels[8][8] = true;
	state.pixels[15][15] = true;
	state.index_register = source_index;

	c8_display_sprite(&state, x, y, n);

	assert(!state.pixels[8][8] && "Pixels not flipped from 1 to 0");

	assert(state.pixels[9][9] && "Pixels not flipped from 0 to 1");

	assert(!state.pixels[15][15] && "Pixels not flipped from 1 to 0");

	assert(!state.pixels[16][16] && "Pixels should not have been flipped");

	assert(state.var_registers[C8_FLAG_REG] && "Flag register not set");
}

int main(void)
{
	c8_test_arena_init();
	c8_test_arena_alloc_free();
	c8_test_call();
	c8_test_add_number_to_register_no_overflow();
	c8_test_add_number_to_register_overflow();
	c8_test_clear();
	c8_test_return();
	c8_test_return_underflow();
	c8_test_display_sprite();
}