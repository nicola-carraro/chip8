#ifndef C8_APP_C
#define C8_APP_C

#include "c8_app.h"

bool c8_app_update()
{
	C8_Rgb rgb = { 255, 0, 0 };

	C8_Arena arena;

	c8_arena_init(&arena, 1024, 4);

	void* mem = c8_arena_alloc(&arena, 100);

	c8_arena_free_all(&arena);

	return c8_plat_push_rect(0.0, 10.0, 200.0, 300.0, rgb);
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
