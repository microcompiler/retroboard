#include "rb_draw.h"
#include "rb_board.h"
#include "rb_printf.h"

// RGB Colors
// static rb_color_t COLOR_BLACK = {0, 0, 0};
// static rb_color_t COLOR_WHITE = {255, 255, 255};
// static rb_color_t COLOR_RED = {255, 0, 0};
// static rb_color_t COLOR_GREEN = {0, 255, 0};
// static rb_color_t COLOR_BLUE = {0, 0, 255};
// static rb_color_t COLOR_YELLOW = {255, 255, 0};
// static rb_color_t COLOR_CYAN = {0, 255, 255};
// static rb_color_t COLOR_PINK = {255, 0, 255};

// RGB matrix values
static rb_color_t frame[RB_MTX_ROWS][RB_MTX_COLUMNS];
static uint8_t power = 1;
static uint8_t brightness = 100;
static uint8_t rotation = 0;
static uint32_t row = 0;

// RGB encoder values
uint8_t sw1_state; 
uint8_t sw1_pressed; 
int32_t sw1_pos;

uint8_t sw2_state; 
uint8_t sw2_pressed; 
int32_t sw2_pos; 

uint8_t sw3_state;
uint8_t sw3_pressed; 
uint32_t sw3_pos; 

uint8_t sw4_state;
uint8_t sw4_pressed; 
int32_t sw4_pos; 

void rb_debug_init(void)
{
    sys_enable(sys_device_uart1);
    gpio_function(RB_UART1_RX, pad_uart1_txd);
    gpio_function(RB_UART1_RX, pad_uart1_rxd);

    uart_open(UART1, 1, UART_DIVIDER_19200_BAUD, uart_data_bits_8, uart_parity_none, uart_stop_bits_1);

    init_printf(NULL, rb_debug_write);

    uart_puts(UART1,
              "\x1B[2J" /* ANSI/VT100 - Clear the Screen */
              "\x1B[H"  /* ANSI/VT100 - Move Cursor to Home */
              "Retroboard - Copyright (C) Microcompiler \r\n"
              "--------------------------------------------------------------------- \r\n");
}

void rb_debug_write(void *p, char c)
{
    uart_write(UART1, (uint8_t)c);
}

void rb_timer_init(void)
{
	sys_enable(sys_device_timer_wdt);

	timer_init(timer_select_a,
			   10,
			   timer_direction_down,
			   timer_prescaler_select_on,
			   timer_mode_continuous);

	timer_prescaler(5000);
	timer_enable_interrupt(timer_select_a);
	timer_start(timer_select_a);
}

void rb_rtc_init(void)
{
    sys_enable(sys_device_timer_wdt);
    rtc_init(rtc_wrap_enabled);
    rtc_start();
    delayms(100);
}

void rb_spibus_init(void)
{
    uint8_t slave_pins[] = {
        RB_SPI0_CLK,
        RB_SPI0_SS,
        RB_SPI0_MOSI,
        RB_SPI0_MISO};

    sys_enable(sys_device_spi_slave1);

    for (uint8_t i = 0; i < sizeof(slave_pins); i++)
    {
        gpio_dir(slave_pins[i], pad_dir_input);
        gpio_slew(slave_pins[i], pad_slew_fast);
        gpio_schmitt(slave_pins[i], pad_schmitt_off);
        gpio_pull(slave_pins[i], pad_pull_none);
        gpio_idrive(slave_pins[i], pad_drive_16mA);
        gpio_function(slave_pins[i], pad_func_1);
    };

    gpio_dir(RB_SPI0_MISO, pad_dir_output);

    gpio_function(RB_SPI0_CLK, pad_spis1_clk);
    gpio_function(RB_SPI0_SS, pad_spis1_ss);
    gpio_function(RB_SPI0_MOSI, pad_spis1_mosi);
    gpio_function(RB_SPI0_MISO, pad_spis1_miso);

    spi_init(SPIS1, spi_dir_slave, spi_mode_3, 4);

    spi_option(SPIS1, spi_option_fifo, 1);
    spi_option(SPIS1, spi_option_fifo_size, MX_SPI0_FIFOSIZE);
    spi_option(SPIS1, spi_option_fifo_receive_trigger, 1);
    spi_option(SPIS1, spi_option_multi_receive, 1);
}

void rb_gpio_init(void)
{
    // MCU status Led
    gpio_dir(RB_IO_MCU_LED, pad_dir_output);
    gpio_write(RB_IO_MCU_LED, LOW);

    // RGB panel pins
    uint8_t mtx_pins[] = {
        RB_IO_MTX_R1,
        RB_IO_MTX_G1,
        RB_IO_MTX_B1,
        RB_IO_MTX_R2,
        RB_IO_MTX_G2,
        RB_IO_MTX_B2,
        RB_IO_MTX_A,
        RB_IO_MTX_B,
        RB_IO_MTX_C,
        RB_IO_MTX_D,
        RB_IO_MTX_CLK,
        RB_IO_MTX_STB,
        RB_IO_MTX_OE};

    for (uint8_t i = 0; i < sizeof(mtx_pins); i++)
    {
        gpio_dir(mtx_pins[i], pad_dir_output);
        gpio_pull(mtx_pins[i], pad_pull_pulldown);
        gpio_slew(mtx_pins[i], pad_slew_fast);
        gpio_schmitt(mtx_pins[i], pad_schmitt_off);
        gpio_idrive(mtx_pins[i], pad_drive_16mA);
        gpio_function(mtx_pins[i], pad_func_0);
        gpio_write(mtx_pins[i], 0);
    };

    // RGB switch pins
    uint8_t sw_rgb_pins[] = {
        RB_IO_SW1_RED,
        RB_IO_SW1_GREEN,
        RB_IO_SW1_BLUE,
        RB_IO_SW2_RED,
        RB_IO_SW2_GREEN,
        RB_IO_SW2_BLUE,
        RB_IO_SW3_RED,
        RB_IO_SW3_GREEN,
        RB_IO_SW3_BLUE,
        RB_IO_SW4_RED,
        RB_IO_SW4_GREEN,
        RB_IO_SW4_BLUE};

    for (uint8_t i = 0; i < sizeof(sw_rgb_pins); i++)
    {
        gpio_dir(sw_rgb_pins[i], pad_dir_output);
        gpio_pull(sw_rgb_pins[i], pad_pull_pulldown);
        gpio_slew(sw_rgb_pins[i], pad_slew_fast);
        gpio_schmitt(sw_rgb_pins[i], pad_schmitt_on);
        gpio_idrive(sw_rgb_pins[i], pad_drive_16mA);
        gpio_function(sw_rgb_pins[i], pad_func_0);
        gpio_write(sw_rgb_pins[i], HIGH);
    };

    gpio_write(RB_IO_MTX_OE, 1);

    // RGB switch pins
    uint8_t sw_btn_pins[] = {
        RB_IO_SW1_SWITCH,
        RB_IO_SW2_SWITCH,
        RB_IO_SW3_SWITCH,
        RB_IO_SW4_SWITCH};

    for (uint8_t i = 0; i < sizeof(sw_btn_pins); i++)
    {
        gpio_dir(sw_btn_pins[i], pad_dir_input);
        gpio_pull(sw_btn_pins[i], pad_pull_pulldown);
        gpio_slew(sw_btn_pins[i], pad_slew_fast);
        gpio_schmitt(sw_btn_pins[i], pad_schmitt_off);
        gpio_idrive(sw_btn_pins[i], pad_drive_16mA);
        gpio_function(sw_btn_pins[i], pad_func_0);
        gpio_write(sw_btn_pins[i], HIGH);
    };

    // RGB encoder pins
    uint8_t sw_enc_pins[] = {
        RB_IO_SW1_TA,
        RB_IO_SW1_TB,
        RB_IO_SW2_TA,
        RB_IO_SW2_TB,
        RB_IO_SW3_TA,
        RB_IO_SW3_TB,
        RB_IO_SW4_TA,
        RB_IO_SW4_TB};

    for (uint8_t i = 0; i < sizeof(sw_enc_pins); i++)
    {
        gpio_dir(sw_enc_pins[i], pad_dir_input);
        gpio_pull(sw_enc_pins[i], pad_pull_pullup);
        gpio_slew(sw_enc_pins[i], pad_slew_fast);
        gpio_schmitt(sw_enc_pins[i], pad_schmitt_off);
        gpio_idrive(sw_enc_pins[i], pad_drive_16mA);
        gpio_function(sw_enc_pins[i], pad_func_0);
        gpio_write(sw_enc_pins[i], HIGH);
    };
}

void rb_encoder_init(void)
{
    uint8_t s = 0;

    if (gpio_read(RB_IO_SW1_TA))
        s |= 1;
    if (gpio_read(RB_IO_SW1_TB))
        s |= 2;
    sw1_state = s;

    if (gpio_read(RB_IO_SW2_TA))
        s |= 1;
    if (gpio_read(RB_IO_SW2_TB))
        s |= 2;
    sw2_state = s;

    if (gpio_read(RB_IO_SW3_TA))
        s |= 1;
    if (gpio_read(RB_IO_SW3_TB))
        s |= 2;
    sw3_state = s;

    if (gpio_read(RB_IO_SW4_TA))
        s |= 1;
    if (gpio_read(RB_IO_SW4_TB))
        s |= 2;
    sw4_state = s;
}

void rb_matrix_render(void)
{
    if (timer_is_interrupted(timer_select_a) == 1)
	{
        // reset row value
        if (row >= RB_MTX_ROWS / 2)
            row = 0;
    
        int16_t subrow = row + RB_MTX_SCANLINE;

        // Set the row address
        gpio_write(RB_IO_MTX_A, (row & 1) == 0 ? 0 : 1);
        gpio_write(RB_IO_MTX_B, (row & 2) == 0 ? 0 : 1);
        gpio_write(RB_IO_MTX_C, (row & 4) == 0 ? 0 : 1);
        gpio_write(RB_IO_MTX_D, (row & 8) == 0 ? 0 : 1);

        for (int16_t col = 0; col < RB_MTX_COLUMNS; col++) {

            //Set color data
            gpio_write(RB_IO_MTX_R1, (frame[row][col].red) ? HIGH : LOW);
            gpio_write(RB_IO_MTX_G1, (frame[row][col].green) ? HIGH : LOW);
            gpio_write(RB_IO_MTX_B1, (frame[row][col].blue) ? HIGH : LOW);
            gpio_write(RB_IO_MTX_R2, (frame[subrow][col].red) ? HIGH : LOW);
            gpio_write(RB_IO_MTX_G2, (frame[subrow][col].green) ? HIGH : LOW);
            gpio_write(RB_IO_MTX_B2, (frame[subrow][col].blue) ? HIGH : LOW);

            // Set clock signal
            gpio_write(RB_IO_MTX_CLK, HIGH);
            gpio_write(RB_IO_MTX_CLK, LOW);    
        }

        // Set output enabled
        if (power)
            gpio_write(RB_IO_MTX_OE, LOW);

        // Set latch signal
        gpio_write(RB_IO_MTX_STB, LOW);
        gpio_write(RB_IO_MTX_STB, HIGH);

        // Set brightness delay 
        delayus(brightness);

        // Set output disabled
        gpio_write(RB_IO_MTX_OE, HIGH);
        
        // Increment row value
        row++; 
    }
}

void rb_encoder_update(void)
{
    rb_encoder_button_decode();
    rb_encoder_sw1_decode();
    rb_encoder_sw2_decode();
    rb_encoder_sw3_decode();
    rb_encoder_sw4_decode();
}

void rb_encoder_button_decode(void)
{
    sw1_pressed = 0;
    sw2_pressed = 0;
    sw3_pressed = 0;
    sw4_pressed = 0;

    if (gpio_read(RB_IO_SW1_SWITCH) == HIGH)
        sw1_pressed = 1;

    if (gpio_read(RB_IO_SW2_SWITCH) == HIGH)
        sw2_pressed = 1;

    if (gpio_read(RB_IO_SW3_SWITCH) == HIGH)
        sw3_pressed = 1;
  
    if (gpio_read(RB_IO_SW4_SWITCH) == HIGH)
        sw4_pressed = 1;
}

void rb_encoder_sw1_decode(void)
{
    uint8_t s = sw1_state & 3;

    if (gpio_read(RB_IO_SW1_TA)) s |= 4;
    if (gpio_read(RB_IO_SW1_TB)) s |= 8;
    switch (s) {
        case 0: case 5: case 10: case 15:
            break;
        case 1: case 7: case 8: case 14:
            sw1_pos++; break;
        case 2: case 4: case 11: case 13:
            sw1_pos--; break;
        case 3: case 12:
            sw1_pos += 2; break;
        default:
            sw1_pos -= 2; break;
    }
    sw1_state = (s >> 2);   
}

void rb_encoder_sw2_decode(void)
{
    uint8_t s = sw2_state & 3;

    if (gpio_read(RB_IO_SW2_TA)) s |= 4;
    if (gpio_read(RB_IO_SW2_TB)) s |= 8;
    switch (s) {
        case 0: case 5: case 10: case 15:
            break;
        case 1: case 7: case 8: case 14:
            sw2_pos++; break;
        case 2: case 4: case 11: case 13:
            sw2_pos--; break;
        case 3: case 12:
            sw2_pos += 2; break;
        default:
            sw2_pos -= 2; break;
    }
    sw2_state = (s >> 2);   
}

void rb_encoder_sw3_decode(void)
{
    uint8_t s = sw3_state & 3;

    if (gpio_read(RB_IO_SW3_TA)) s |= 4;
    if (gpio_read(RB_IO_SW3_TB)) s |= 8;
    switch (s) {
        case 0: case 5: case 10: case 15:
            break;
        case 1: case 7: case 8: case 14:
            sw3_pos++; break;
        case 2: case 4: case 11: case 13:
            sw3_pos--; break;
        case 3: case 12:
            sw3_pos += 2; break;
        default:
            sw3_pos -= 2; break;
    }
    sw3_state = (s >> 2);   
}

void rb_encoder_sw4_decode(void)
{
    uint8_t s = sw4_state & 3;

    if (gpio_read(RB_IO_SW4_TA)) s |= 4;
    if (gpio_read(RB_IO_SW4_TB)) s |= 8;
    switch (s) {
        case 0: case 5: case 10: case 15:
            break;
        case 1: case 7: case 8: case 14:
            sw4_pos++; break;
        case 2: case 4: case 11: case 13:
            sw4_pos--; break;
        case 3: case 12:
            sw4_pos += 2; break;
        default:
            sw4_pos -= 2; break;
    }
    sw4_state = (s >> 2);   
}

int32_t rb_rtc_get_seconds(void)
{
    uint32_t time = 0;

    rtc_read(&time);
    return time;
}

void rb_matrix_clear(void)
{
	memset(&frame, 0, sizeof(frame));
}

void rb_matrix_set_power(uint8_t i)
{
	power = i;
}

void rb_matrix_set_brightness(uint8_t i)
{
	// Set brightness in percent; range=1..100
	brightness = (i <= 100 ? (i != 0 ? i : 1) : 100);
}

void rb_matrix_set_rotation(uint8_t i)
{
	rotation = i;
}

void rb_matrix_set_pixel(rb_coord_t x, rb_coord_t y, rb_color_t color) {

	if ((x < 0) || (x >= RB_MTX_ROWS) || (y < 0) || (y >= RB_MTX_COLUMNS)) return;

	switch (rotation) {
	case 1:
		SWAPCOORD(x, y);
		x = RB_MTX_ROWS - 1 - x;
		break;
	case 2:
		x = RB_MTX_ROWS  - 1 - x;
		y = RB_MTX_COLUMNS  - 1 - y;
		break;
	case 3:
		SWAPCOORD(x, y);
		y = RB_MTX_COLUMNS - 1 - y;;
		break;
	}

	frame[x][y] = color;
}

uint8_t rb_encoder_get_switch(uint8_t i)
{
    switch (i)
    {
    case 0:
        return sw1_pressed;
    case 1:
        return sw2_pressed;
    case 2:
        return sw3_pressed;
    case 3:
        return sw4_pressed;
    }
    return 0;
}

void rb_encoder_set_color(uint8_t enc, rb_color_t color)
{
    switch (enc)
    {
    case 0:
        gpio_write(RB_IO_SW1_RED, 0);
        gpio_write(RB_IO_SW1_GREEN, color.green == 0 ? HIGH : LOW);
        gpio_write(RB_IO_SW1_BLUE, color.blue == 0 ? HIGH : LOW);
        break;
    case 1:
        gpio_write(RB_IO_SW2_RED, color.red == 0 ? HIGH : LOW);
        gpio_write(RB_IO_SW2_GREEN, color.green == 0 ? HIGH : LOW);
        gpio_write(RB_IO_SW2_BLUE, color.blue == 0 ? HIGH : LOW);
        break;
    case 2:
        gpio_write(RB_IO_SW3_RED, color.red == 0 ? HIGH : LOW);
        gpio_write(RB_IO_SW3_GREEN, color.green == 0 ? HIGH : LOW);
        gpio_write(RB_IO_SW3_BLUE, color.blue == 0 ? HIGH : LOW);
        break;
    case 3:
        gpio_write(RB_IO_SW4_RED, color.red == 0 ? HIGH : LOW);
        gpio_write(RB_IO_SW4_GREEN, color.green == 0 ? HIGH : LOW);
        gpio_write(RB_IO_SW4_BLUE, color.blue == 0 ? HIGH : LOW);
        break;
    }
}

int32_t rb_encoder_get_position(uint8_t enc)
{
    switch (enc)
    {
    case 0:
        return sw1_pos; 
        break;
    case 1:
        return sw2_pos; 
        break;
    case 2:
        return sw3_pos; 
        break;
    case 3:
        return sw4_pos; 
        break;
    default:
        return 0;
        break;
    }
}