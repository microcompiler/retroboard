#include "rb_main.h"

int main(void)
{    
    rb_debug_init();
    rb_rtc_init();
    rb_timer_init();
    //rb_spibus_init();
    rb_gpio_init();
    rb_encoder_init();

    while (1)
    {
        rb_encoder_update();   
        rb_gameloop();
        rb_matrix_render();    
    }
    return 0;
}