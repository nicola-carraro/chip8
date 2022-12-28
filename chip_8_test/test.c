#include<stdio.h>

#include "../c8_win.c"

void c8_test_arena_init()
{
	printf("TEST : initialise arena\n");

	C8_Arena arena;

	i32 alignement = 4;

	psz size = 100;

	c8_arena_init(&arena, size, alignement);

	assert(arena.alignement == alignement);

	assert(arena.max_bytes == size);

	assert(arena.offset == 0);
}

int main(char** args, int argv)
{
	c8_test_arena_init();
}