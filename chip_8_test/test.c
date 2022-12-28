#include<stdio.h>

#include "../c8_win.c"

#define c8_test_def(a)(printf("TEST : %s\n", a))

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

	void* mem1 = c8_arena_alloc(&arena1, 101);

	assert(arena1.offset == 104);

	void* mem2 = c8_arena_alloc(&arena1, 4);

	assert(arena1.offset == 108);

	assert(((char*)mem2 - (char*)mem1) == 104);

	c8_arena_free_all(&arena1);

	assert(arena1.offset == 0);
}

int main(char** args, int argv)
{
	c8_test_arena_init();

	c8_test_arena_alloc_free();
}