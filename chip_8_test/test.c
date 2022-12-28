#include<stdio.h>

#include "../c8_win.c"

void c8_test_arena_alloc()
{
	C8_Arena arena;

	c8_arena_init(&arena, 100, 4);

	printf("c8 test arena alloc \n");

}

int main(char** args, int argv)
{
	c8_test_arena_alloc();
}