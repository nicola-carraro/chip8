#ifndef C8_APP
#define C8_APP
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>

typedef int32_t i32;
typedef int64_t i64;
typedef char u8;

#define c8_clear_struct(obj)(memset(&obj, 0, sizeof(obj)))

#define c8_arr_count(a)(sizeof(a) / sizeof(a[0]))

typedef struct {
	bool running;
	i32 cli_width;
	i32 cli_height;
} C8_App_State;

typedef struct {
	union {
		struct {
			float x;
			float y;
		};
	
		float coords[2];
	};
} C8_V2;

typedef struct {
	u8 r;
	u8 g;
	u8 b;
}C8_Rgb;

#endif 
