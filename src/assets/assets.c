#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "../main/chip8.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

void write_atlas(stbtt_fontinfo *info, int pixel_height, char start_char, char one_past_end_char)
{

	int bounding_x0 = 0;

	int bounding_x1 = 0;

	int bounding_y0 = 0;

	int bounding_y1 = 0;

	stbtt_GetFontBoundingBox(info, &bounding_x0, &bounding_y0, &bounding_x1, &bounding_y1);

	int ascent = 0;
	int descent = 0;
	int line_gap = 0;

	stbtt_GetFontVMetrics(info, &ascent, &descent, &line_gap);

	float scale = stbtt_ScaleForPixelHeight(info, (float)pixel_height);

	float baseline = scale * bounding_y1;

	int bounding_width = (int)((bounding_x1 - bounding_x0) * scale);

	int bounding_height = (int)((bounding_y1 - bounding_y0) * scale);

	int char_count = one_past_end_char - start_char;

	int atlas_width = bounding_width * char_count;

	int atlas_height = bounding_height;

	int atlas_size = atlas_width * atlas_height;

	uint8_t *atlas_buffer = malloc(atlas_size);

	memset(atlas_buffer, 0, atlas_size);

	assert(atlas_buffer);

	C8_Font header = {
		.width = bounding_width * char_count,
		.height = bounding_height,
		.above_bl = -bounding_y0 * scale};

	int x_offset = 0;
	for (char c = start_char; c < one_past_end_char; c++)
	{
		int width = 0;
		int height = 0;
		uint8_t *bitmap = stbtt_GetCodepointBitmap(info, 0, scale, c, &width, &height, 0, 0);

		int glyph_x0 = 0;

		int glyph_x1 = 0;

		int glyph_y0 = 0;

		int glyph_y1 = 0;
		stbtt_GetCodepointBitmapBox(info, c, scale, scale, &glyph_x0, &glyph_y0, &glyph_x1, &glyph_y1);

		int advanceWidth = 0;
		int leftSideBearing = 0;
		stbtt_GetCodepointHMetrics(info, c, &advanceWidth, &leftSideBearing);

		int y_offset = (int)baseline + glyph_y0;

		for (int row = 0; row < height; row++)
		{
			for (int col = 0; col < width; col++)
			{

				atlas_buffer[(row + y_offset) * atlas_width + (col + x_offset)] = bitmap[row * width + col];
			}
		}

		float u_left = (float)x_offset / (float)atlas_width;

		float u_right = (float)(x_offset + width) / (float)atlas_width;

		float v_top = (float)y_offset / (float)atlas_height;

		float v_bottom = (float)(y_offset + height) / (float)atlas_height;

		x_offset += bounding_width;

		float g_ascent = 0.0f;
		float g_descent = 0.0f;

		if (glyph_y0 < 0.0f)
		{
			g_ascent = -(float)glyph_y0;
		}

		if (glyph_y1 > 0.0f)
		{
			g_descent = (float)glyph_y1;
		}

		C8_Glyph glyph = {
			.u_left = u_left,
			.u_right = u_right,
			.v_top = v_top,
			.v_bottom = v_bottom,
			.y_offset = (float)glyph_y0,
			.width = (float)width,
			.height = (float)height,
			.ascent = g_ascent,
			.descent = g_descent};

		header.glyphs[c - start_char] = glyph;
		// int y_offset = baseline
	}

	FILE *o = fopen("data/font.ppm", "wb");

	assert(o);

	fprintf(o, "P6\n");
	fprintf(o, "%d\n", atlas_width);
	fprintf(o, "%d\n", atlas_height);
	fprintf(o, "255\n");

	for (int i = 0; i < atlas_size; i++)

	{
		uint8_t alpha = atlas_buffer[i];
		fputc(alpha, o);
		for (int channel = 0; channel < 2; channel++)
		{
			fputc(0, o);
		}
	}

	fclose(o);

	o = fopen("data/fonts", "wb");

	fwrite(&header, sizeof(header), 1, o);

	assert(o);

	for (int i = 0; i < atlas_size; i++)

	{
		uint8_t alpha = atlas_buffer[i];

		fputc(alpha, o);
	}

	fclose(o);
}

int main(void)
{

	char file_name[] = "..\\src\\assets\\data\\VT323-Regular.ttf";
	FILE *i = fopen(file_name, "rb");

	if (i != 0)
	{
		fseek(i, 0L, SEEK_END);

		size_t sz = ftell(i);

		fseek(i, 0L, SEEK_SET);

		void *data = malloc(sz);

		if (data != 0)
		{
			size_t read = fread(data, sz, 1, i);

			if (read == 1)
			{
				stbtt_fontinfo font;
				stbtt_InitFont(&font, data, stbtt_GetFontOffsetForIndex(data, 0));

				write_atlas(&font, 120, C8_FIRST_CHAR, C8_ONE_PAST_LAST_CHAR);
			}
			else
			{
				printf("Could not read %s", file_name);
			}
		}

		free(data);
	}
	else
	{
		printf("Could not open %s", file_name);
	}

	return 0;
}