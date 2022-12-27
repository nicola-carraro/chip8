#ifndef C8_APP
#define C8_APP
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>

typedef int32_t i32;
typedef int64_t i64;

#define c8_clear_struct(obj)(memset(&obj, 0, sizeof(obj)))

typedef struct {
	bool running;
	i32 cli_width;
	i32 cli_height;
} C8_App_State;

#endif 
