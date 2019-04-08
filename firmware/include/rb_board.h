/*
 * Includes
 */
#include <string.h>
#include <ft900.h>

#define HIGH (1)
#define LOW (0)
#define SWAPCOORD(a, b) { rb_coord_t t = a; a = b; b = t; }

/* RGB matrix data */
#define RB_MTX_ROWS           (32)
#define RB_MTX_COLUMNS        (32)
#define RB_MTX_SCANLINE       (16)

// Hardware pin data
#define RB_IO_MCU_LED         (16)

#define RB_IO_MTX_R1          (17)
#define RB_IO_MTX_G1          (18)
#define RB_IO_MTX_B1          (19)
#define RB_IO_MTX_R2          (20)
#define RB_IO_MTX_G2          (21)
#define RB_IO_MTX_B2          (22)
#define RB_IO_MTX_A           (24)
#define RB_IO_MTX_B           (25)
#define RB_IO_MTX_C           (26)
#define RB_IO_MTX_D           (27)
#define RB_IO_MTX_CLK         (29)
#define RB_IO_MTX_STB         (30)
#define RB_IO_MTX_OE          (31)

#define BIT_MASK_R1           (1 << 17)
#define BIT_MASK_G1           (1 << 18)
#define BIT_MASK_B1           (1 << 19)
#define BIT_MASK_R2           (1 << 20)
#define BIT_MASK_G2           (1 << 21)
#define BIT_MASK_B2           (1 << 22)
#define BIT_MASK_A            (1 << 24)
#define BIT_MASK_B            (1 << 25)
#define BIT_MASK_C            (1 << 26)
#define BIT_MASK_D            (1 << 27)
#define BIT_MASK_CLK          (1 << 29)
#define BIT_MASK_STB          (1 << 30)
#define BIT_MASK_OE           (1 << 31)

#define RB_IO_SW1_RED         (32) 
#define RB_IO_SW1_GREEN       (33)  
#define RB_IO_SW1_SWITCH      (34) 
#define RB_IO_SW1_BLUE        (35) 
#define RB_IO_SW1_TA          (45)  
#define RB_IO_SW1_TB          (44) 

#define RB_IO_SW2_RED         (46) 
#define RB_IO_SW2_GREEN       (47)  
#define RB_IO_SW2_SWITCH      (48) 
#define RB_IO_SW2_BLUE        (49) 
#define RB_IO_SW2_TA          (51)  
#define RB_IO_SW2_TB          (50) 

#define RB_IO_SW3_RED         (61) 
#define RB_IO_SW3_GREEN       (62)  
#define RB_IO_SW3_SWITCH      (63) 
#define RB_IO_SW3_BLUE        (64) 
#define RB_IO_SW3_TA          (66)  
#define RB_IO_SW3_TB          (65) 

#define RB_IO_SW4_RED         (0) 
#define RB_IO_SW4_GREEN       (1)  
#define RB_IO_SW4_SWITCH      (2) 
#define RB_IO_SW4_BLUE        (3) 
#define RB_IO_SW4_TA          (5)  
#define RB_IO_SW4_TB          (4)  

#define RB_SPI0_CLK           (36)
#define RB_SPI0_SS            (37)
#define RB_SPI0_MOSI          (38)
#define RB_SPI0_MISO          (39)
#define MX_SPI0_FIFOSIZE      (64)

#define RB_SPI1_CLK           (40)
#define RB_SPI1_SS            (41)
#define RB_SPI1_MOSI          (42)
#define RB_SPI1_MISO          (43)

#define RB_UART1_RX           (52)
#define RB_UART1_TX           (53)
#define RB_UART1_RTS          (54)
#define RB_UART1_CTS          (55)

void rb_debug_init(void);
void rb_debug_write(void *p, char c);
void rb_timer_init(void);
void rb_spibus_init(void);
void rb_gpio_init(void);
void rb_encoder_init(void);

void rb_matrix_render(void);
void rb_encoder_update(void);

void rb_encoder_button_decode(void);
void rb_encoder_sw1_decode(void);
void rb_encoder_sw2_decode(void);
void rb_encoder_sw3_decode(void);
void rb_encoder_sw4_decode(void);

void rb_matrix_clear(void);
void rb_matrix_set_power(uint8_t i);
void rb_matrix_set_brightness(uint8_t i);
void rb_matrix_set_rotation(uint8_t i);
void rb_matrix_set_pixel(rb_coord_t x, rb_coord_t y, rb_color_t color);

uint8_t rb_encoder_get_switch(uint8_t i);
void rb_encoder_set_color(uint8_t enc, rb_color_t color);

rb_color_t rb_get_color(uint8_t red, uint8_t green, uint8_t blue);