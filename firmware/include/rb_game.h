#include <stdlib.h>
#include "rb_draw.h"

#define TRUE (1)
#define FALSE (0)

void rb_matrix_clear(void);
void rb_matrix_set_power(uint8_t i);
void rb_matrix_set_brightness(uint8_t i);
void rb_matrix_set_rotation(uint8_t i);

int32_t rb_rtc_get_seconds(void);
uint8_t rb_encoder_get_switch(uint8_t i);
void rb_encoder_set_color(uint8_t enc, rb_color_t color);
int32_t rb_encoder_get_position(uint8_t enc);