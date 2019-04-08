#include "rb_font.h"

typedef int16_t rb_coord_t;

typedef struct color
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} rb_color_t;

typedef struct point 
{
	rb_coord_t x;
	rb_coord_t y;
} rb_point_t;

static const rb_color_t COLOR_BLACK = {0, 0, 0};
static const rb_color_t COLOR_WHITE = {255, 255, 255};
static const rb_color_t COLOR_RED = {255, 0, 0};
static const rb_color_t COLOR_GREEN = {0, 255, 0};
static const rb_color_t COLOR_BLUE = {0, 0, 255};
static const rb_color_t COLOR_YELLOW = {255, 255, 0};
static const rb_color_t COLOR_CYAN = {0, 255, 255};
static const rb_color_t COLOR_PINK = {255, 0, 255};

void rb_draw_pixel(rb_coord_t x, rb_coord_t y, rb_color_t color);
void rb_draw_point(rb_point_t point, rb_color_t color);
void rb_draw_line(rb_point_t p1, rb_point_t p2, rb_color_t color);
void rb_draw_vline(rb_coord_t x1, rb_coord_t y1, rb_coord_t y2, rb_color_t color);
void rb_draw_hline(rb_coord_t x1, rb_coord_t x2, rb_coord_t y1, rb_color_t color);
void rb_draw_uint16(rb_point_t point, const rb_font_t * font_p, uint16_t i, rb_color_t color);
void rb_draw_symbol(rb_point_t point, const rb_font_t * font_p, uint16_t symbol, rb_color_t color);
void rb_draw_string(rb_point_t point, const rb_font_t * font_p, uint8_t * text_p, rb_color_t color);