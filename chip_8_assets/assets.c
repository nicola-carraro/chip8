#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdint.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

void write_bmp(stbtt_fontinfo* font, char c) {

	int32_t width;
	int32_t height;
	int32_t x_offset;
	int32_t y_offset;
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

	int x0;
	int y0;
	int x1;
	int y1;

	stbtt_GetFontBoundingBox(font, &x0, &y0, &x1, &y1);

	int advanceWidth;

	int leftSideBearing;

	stbtt_GetGlyphHMetrics(font, stbtt_FindGlyphIndex(font, c), &advanceWidth, &leftSideBearing);

	if (o != 0) {
		/*	fprintf(o, "P6\n");
			fprintf(o, "%d\n", width);
			fprintf(o, "%d\n", height);
			fprintf(o, "255\n");*/

			//fwrite(&width, sizeof(width), 1, o);
			//fwrite(&height, sizeof(height), 1, o);
			//fwrite(bitmap, sizeof(*bitmap), width * height, o);

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

void write_atlas(stbtt_fontinfo* font, char start_char, char one_past_end_char) {

	int32_t char_width;
	int32_t char_height;
	int32_t char_x_offset;
	int32_t char_y_offset;

	int32_t total_width = 0;

	int32_t total_height = 0;

	int32_t char_count = one_past_end_char - start_char;

	uint8_t** char_bitmaps = malloc(sizeof(*char_bitmaps) * (char_count));

	int32_t* widths = malloc(sizeof(*widths) * (char_count));

	int32_t* heights = malloc(sizeof(*heights) * (char_count));

	for (int32_t i = 0; i < (char_count); i++)
	{

		char c = start_char + i;

		char_bitmaps[i] = stbtt_GetCodepointBitmap(
			font,
			0,
			stbtt_ScaleForPixelHeight(font, 120),
			c,
			&char_width,
			&char_height,
			&char_x_offset,
			&char_y_offset
		);

		widths[i] = char_width;
		heights[i] = char_height;

		total_width += char_width;

		if (char_height > total_height) {
			total_height = char_height;
		}
	}

	char f_name[256] = "out\\atlas.ppm";

	FILE* o = fopen(f_name, "wb");

	int x0;
	int y0;
	int x1;
	int y1;

	//stbtt_GetFontBoundingBox(font, &x0, &y0, &x1, &y1);

	int advanceWidth;

	int leftSideBearing;

	//stbtt_GetGlyphHMetrics(font, stbtt_FindGlyphIndex(font, c), &advanceWidth, &leftSideBearing);

	int32_t current_x = 0;
	int32_t current_y = 0;

	if (o != 0) {
		fprintf(o, "P6\n");
		fprintf(o, "%d\n", total_width);
		fprintf(o, "%d\n", total_height);
		fprintf(o, "255\n");

		//fwrite(&width, sizeof(width), 1, o);
		//fwrite(&height, sizeof(height), 1, o);
		//fwrite(bitmap, sizeof(*bitmap), width * height, o);

		for (int bmp_row = 0; bmp_row < total_height; bmp_row++) {

			int32_t char_index = 0;

			int32_t char_x = 0;

			int32_t char_width = widths[0];

			int32_t char_height = heights[0];

			for (int bmp_col = 0; bmp_col < total_width; bmp_col++) {

				if (char_x >= char_width) {
					char_x = 0;
					char_index++;
					char_width = widths[char_index];
					char_height = heights[char_index];
				}

				int pixel_i = (bmp_row * char_width) + char_x;

				uint8_t color = 0;

				if (bmp_row < char_height) {
					color = char_bitmaps[char_index][pixel_i];
				}

				for (int channel = 0; channel < 3; channel++) {
					fputc(color, o);
				}

				if (color) {
					printf("X");
				}
				else {
					printf(" ");
				}

				char_x++;
			}

			printf("\n");

		}

		fflush(o);

	}

	//stbtt_FreeBitmap(bitmap, 0);
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

			//for (char c = '0'; c <= 'Z'; c++) {
			//	if (read != 0) {
			//		stbtt_fontinfo font;
			//		stbtt_InitFont(&font, data, stbtt_GetFontOffsetForIndex(data, 0));
			//		//write_bmp(&font, c);

			//		
			//	}
			//}

			stbtt_fontinfo font;
			stbtt_InitFont(&font, data, stbtt_GetFontOffsetForIndex(data, 0));

			write_atlas(&font, 'B', 'F');

		}

		free(data);

	}

	return 0;
}