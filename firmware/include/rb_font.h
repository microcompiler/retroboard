#include <stdint.h>
#include <stddef.h>

#include "rb_dejavu_10.h"
#include "rb_dejavu_20.h"
#include "rb_dejavu_30.h"

typedef struct {
	uint32_t first_ascii;
	uint32_t last_ascii;
	uint8_t height_row;
	const uint8_t * bitmap;
	const uint32_t * map;
	const uint8_t * width;
} rb_font_t;

static const rb_font_t rb_font_dejavu_10 =
{
	32,/*First letter's unicode */
	126,/*Last letter's unicode */
	10,/*Letters height (rows) */
	dejavu_10_bitmap,/*Glyph's bitmap*/
	dejavu_10_map,/*Glyph start indexes in the bitmap*/
	dejavu_10_width,/*Glyph widths (columns)*/
};

static const rb_font_t rb_font_dejavu_20 =
{
	32,/*First letter's unicode */
	126,/*Last letter's unicode */
	20,/*Letters height (rows) */
	dejavu_20_bitmap,/*Glyph's bitmap*/
	dejavu_20_map,/*Glyph start indexes in the bitmap*/
	dejavu_20_width/*Glyph widths (columns)*/
};

static const rb_font_t rb_font_dejavu_30 =
{
	32,/*First letter's unicode */
	126,/*Last letter's unicode */
	30,/*Letters height (rows) */
	dejavu_30_bitmap,/*Glyph's bitmap*/
	dejavu_30_map,/*Glyph start indexes in the bitmap*/
	dejavu_30_width,/*Glyph widths (columns)*/
};

const uint8_t * rb_font_get_bitmap(const rb_font_t * font_p, uint32_t letter);


static inline uint8_t rb_font_get_height(const rb_font_t * font_p)
{
	return font_p->height_row;
}

uint8_t rb_font_get_width(const rb_font_t * font_p, uint32_t letter);