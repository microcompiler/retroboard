#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rb_draw.h"

#define SWAPCOORD(a, b) { rb_coord_t t = a; a = b; b = t; }
#define MX_MATH_ABS(x) ((x)>0?(x):(-(x)))

void rb_matrix_set_pixel(rb_coord_t x, rb_coord_t y, rb_color_t color);
rb_font_t * rb_font_get(uint8_t font);

void rb_draw_pixel(rb_coord_t x, rb_coord_t y, rb_color_t color)
{
	rb_matrix_set_pixel(x, y, color);
}

void rb_draw_point(rb_point_t point, rb_color_t color)
{
	rb_matrix_set_pixel(point.x, point.y, color);
}

void rb_draw_line(rb_point_t p1, rb_point_t p2, rb_color_t color)
{
	rb_coord_t dx, dy;
	uint32_t steep;
	int32_t error;
	int32_t ystep;

	if ((p1.x + p1.y + p2.x + p2.y ) == 0)
		return;

	if (p1.x == p2.x) {
		rb_draw_vline(p1.x, p1.y, p2.y, color);
	}
	else if (p1.y == p2.y) {
		rb_draw_hline(p1.x, p2.x, p1.y, color);
	}

	steep = MX_MATH_ABS(p2.y - p1.y) > MX_MATH_ABS(p2.x - p1.x);

	if (steep) {
		SWAPCOORD(p1.x, p1.y);
		SWAPCOORD(p2.x, p2.y);
	}

	if (p1.x > p2.x) {
		SWAPCOORD(p1.x, p2.x);
		SWAPCOORD(p1.y, p2.y);
	}

	dx = p2.x - p1.x;
	dy = MX_MATH_ABS(p2.y - p1.y);

	error = dx / 2;

	if (p1.y < p2.y) {
		ystep = 1;
	}
	else {
		ystep = -1;
	}

	for (; p1.x <= p2.x; p1.x++) {
		if (steep) {
			rb_draw_pixel(p1.y, p1.x, color);
		}
		else {
			rb_draw_pixel(p1.x, p1.y, color);
		}
		error -= dy;
		if (error < 0) {
			p1.y += ystep;
			error += dx;
		}
	}
}

void rb_draw_vline(rb_coord_t x1, rb_coord_t y1, rb_coord_t y2, rb_color_t color)
{
	rb_coord_t y;

	if ((x1 + y1 + y2) == 0)
        return;

	if (y1 < y2) {
		for (y = y1; y <= y2; y++) {
			rb_draw_pixel(x1, y, color);
		}
	}
	else {
		for (y = y2; y <= y1; y++) {
			rb_draw_pixel(x1, y, color);
		}
	}
}

void rb_draw_hline(rb_coord_t x1, rb_coord_t x2, rb_coord_t y1, rb_color_t color)
{
	rb_coord_t x;

	if ((x1 + x2 + y1) == 0)
		return;

	if (x1 < x2) {
		for (x = x1; x <= x2; x++) {
			rb_draw_pixel(x, y1, color);
		}
	}
	else {
		for (x = x2; x <= x1; x++) {
			rb_draw_pixel(x, y1, color);
		}
	}
}

void rb_draw_uint16(rb_point_t point, const rb_font_t * font_p, uint16_t val, rb_color_t color)
{
	static uint8_t buf[9];
	uint32_t i = 7;
	
	do {
		buf[i--] = (val % 10) + '0';
		val /= 10;
	} while (val > 0);
	
	rb_draw_string(point, font_p, buf + i + 1, color);	
}

void rb_draw_symbol(rb_point_t point, const rb_font_t * font_p, uint16_t symbol, rb_color_t color)
{
	uint16_t col, col_sub, row;

	uint8_t w = rb_font_get_width(font_p, symbol);
	const uint8_t * bitmap_p = rb_font_get_bitmap(font_p, symbol);

	for (row = 0; row < font_p->height_row; row++) {
		for (col = 0, col_sub = 7; col < w; col++, col_sub--) {
			if (*bitmap_p & (1 << col_sub)) {
				rb_draw_pixel(point.x + col, point.y + row, color);
			}

			if (col_sub == 0) {
				bitmap_p++;
				col_sub = 8;
			}
		}
		if (col_sub != 7) bitmap_p++;
	}
}

void rb_draw_string(rb_point_t point, const rb_font_t * font_p, uint8_t * text_p, rb_color_t color)
{
	uint16_t col, col_sub, row;
	int i = 0;

	while (text_p[i] != 0)
	{
		uint8_t width = rb_font_get_width(font_p, text_p[i]);
		const uint8_t *bitmap_p = rb_font_get_bitmap(font_p, text_p[i]);

		for (row = 0; row < font_p->height_row; row++)
		{
			for (col = 0, col_sub = 7; col < width; col++, col_sub--)
			{
				if (*bitmap_p & (1 << col_sub))
				{
					rb_draw_pixel(point.x + row, point.y + col, color);
				}

				if (col_sub == 0)
				{
					bitmap_p++;
					col_sub = 8;
				}
			}
			if (col_sub != 7)
				bitmap_p++;
		}
		point.y += width;
		i++;
	}
}
