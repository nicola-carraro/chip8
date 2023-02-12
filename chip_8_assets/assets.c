#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdint.h>
#include<stdbool.h>
#include"../c8_app.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

void write_bmp(stbtt_fontinfo* font, char c) {

	float scale_factor = stbtt_ScaleForPixelHeight(font, 120);

	int32_t width;
	int32_t height;
	int32_t x_offset;
	int32_t y_offset;
	uint8_t* bitmap = stbtt_GetCodepointBitmap(
		font,
		0,
		scale_factor,
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

void write_atlas(stbtt_fontinfo* font, char start_char, char one_past_end_char, bool write_ppm) {

	int32_t total_width = 0;

	int32_t total_height = 0;

	int32_t char_count = one_past_end_char - start_char;

	uint8_t** char_bitmaps = malloc(sizeof(*char_bitmaps) * (char_count));

	int32_t* widths = malloc(sizeof(*widths) * (char_count));

	int32_t* heights = malloc(sizeof(*heights) * (char_count));

	int32_t* ascenders = malloc(sizeof(*ascenders) * (char_count));

	int32_t max_ascender = 0;

	int32_t max_descender = 0;

	float scale_factor = stbtt_ScaleForPixelHeight(font, 120);

	for (int32_t i = 0; i < (char_count); i++)
	{
		int32_t glyph_width;
		int32_t glyph_height;
		int32_t glyph_x_offset;
		int32_t glyph_ascender;

		char c = start_char + i;

		char_bitmaps[i] = stbtt_GetCodepointBitmap(
			font,
			0,
			scale_factor,
			c,
			&glyph_width,
			&glyph_height,
			&glyph_x_offset,
			&glyph_ascender
		);

		widths[i] = glyph_width;
		heights[i] = glyph_height;
		glyph_ascender = -glyph_ascender;
		ascenders[i] = glyph_ascender;
		int32_t glyph_descender = glyph_height - glyph_ascender;

		total_width += glyph_width;

		if (glyph_height > total_height) {
			total_height = glyph_height;
		}

		if (glyph_ascender > max_ascender) {
			max_ascender = glyph_ascender;
		}

		if (glyph_descender > max_descender) {
			max_descender = glyph_descender;
		}
	}

	C8_Atlas_Header header = { 0 };

	header.total_width_in_pixels = total_width;
	header.total_height_in_pixels = total_height;
	float u_pixel = (1.0f / (float)total_width);
	float v_pixel = (1.0f / (float)total_height);
	int32_t line_height_in_pixels = max_ascender + max_descender;
	header.v_line_height = v_pixel * line_height_in_pixels;

	{
		int32_t current_x = 0;
		for (int32_t glyph_index = 0; glyph_index < c8_arr_count(header.glyphs); glyph_index++) {
			char c = start_char + glyph_index;

			C8_Atlas_Glyph glyph = { 0 };
			int32_t width = widths[glyph_index];
			int32_t height = heights[glyph_index];

			glyph.u_left = u_pixel * ((float)current_x);

			glyph.v_top = 0.0f;
			current_x += width;
			glyph.u_right = u_pixel * ((float)current_x);
			glyph.v_bottom = v_pixel * ((float)height);

			int32_t y_offset_from_line_top =  max_ascender - ascenders[glyph_index];
			glyph.v_offset = v_pixel * (float)y_offset_from_line_top;

			int advance;

			int left_side_bearing;

			stbtt_GetGlyphHMetrics(font, stbtt_FindGlyphIndex(font, c), &advance, &left_side_bearing);

			int32_t advance_in_pixels = advance * scale_factor;

			glyph.u_advancement = u_pixel * advance_in_pixels;

			header.glyphs[glyph_index] = glyph;
		}

	}

	char f_name[256];
		
	if (write_ppm) {
		strcpy(f_name, "out/atlas.ppm");
	}
	else {
		strcpy(f_name, "../data/atlas.c8");
	}

	FILE* o = fopen(f_name, "wb");

	int x0;
	int y0;
	int x1;
	int y1;

	//stbtt_GetFontBoundingBox(font, &x0, &y0, &x1, &y1);

	{
		int32_t current_x = 0;
		int32_t current_y = 0;

		if (o != 0) {

			if (write_ppm)
			{
				fprintf(o, "P6\n");
				fprintf(o, "%d\n", total_width);
				fprintf(o, "%d\n", total_height);
				fprintf(o, "255\n");
			}
			else {
				fwrite(&header, sizeof(header), 1, o);
			}

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

					uint8_t alpha = 0;

					if (bmp_row < char_height) {
						alpha = char_bitmaps[char_index][pixel_i];
					}

					if (write_ppm) {
						for (int channel = 0; channel < 3; channel++) {
							fputc(alpha, o);
						}
					}
					else {

						uint8_t color = alpha == 0 ? 0 : 255;

						fputc(alpha, o);
					}

					if (alpha) {
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

			stbtt_fontinfo font;
			stbtt_InitFont(&font, data, stbtt_GetFontOffsetForIndex(data, 0));

			write_atlas(&font, C8_FIRST_CHAR, C8_ONE_PAST_LAST_CHAR, true);

		}

		free(data);

	}

	return 0;
}