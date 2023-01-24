#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdint.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

void write_bmp(stbtt_fontinfo *font, char c) {

	uint32_t width;
	uint32_t height;
	uint32_t x_offset;
	uint32_t y_offset;
	uint8_t* bitmap = stbtt_GetCodepointBitmap(
		font,
		0,
		stbtt_ScaleForPixelHeight(font, 120),
		c,
		&width,
		&height,
		&x_offset,
		&y_offset
	);
	char f_name[256];
	sprintf(f_name, "out\\%c", c);
	FILE* o = fopen(f_name, "wb");

	if (o != 0) {
	/*	fprintf(o, "P6\n");
		fprintf(o, "%d\n", width);
		fprintf(o, "%d\n", height);
		fprintf(o, "255\n");*/

		fwrite(&width, sizeof(width), 1, o);
		fwrite(&height, sizeof(height), 1, o);
		fwrite(bitmap, sizeof(*bitmap), width * height, o);

		/*for (int bmp_row = 0; bmp_row < height; bmp_row++) {

			for (int bmp_col = 0; bmp_col < width; bmp_col++) {

				int pixel_i = (bmp_row * width) + bmp_col;

				if (bitmap[pixel_i]) {
					printf("x");
				}
				else {
					printf("o");
				}
			for (int color = 0; color < 3; color++) {
					fputc(0, o);
				}

				for (int color = 0; color < 3; color++) {
					fputc(bitmap[pixel_i], o);
				}

				//fputc(bitmap[pixel_i], o);

			}

			printf("\n");

		}*/

		fflush(o);

	}

	stbtt_FreeBitmap(bitmap, 0);
}

int main(char** args, int argv) {

	FILE* i = fopen("MonospaceTypewriter.ttf", "rb");

	if (i != 0) {
		fseek(i, 0L, SEEK_END);

		size_t sz = ftell(i);

		fseek(i, 0L, SEEK_SET);

		void* data = malloc(sz);

		if (data != 0) {
			memset(data, 0, sz);

			size_t read = fread(data, sz, 1, i);

			for (char c = '0'; c <= 'Z'; c++) {
				if (read != 0) {
					stbtt_fontinfo font;
					stbtt_InitFont(&font, data, stbtt_GetFontOffsetForIndex(data, 0));
					write_bmp(&font, c);
				}
			}

		}

		free(data);

	}

	return 0;
}