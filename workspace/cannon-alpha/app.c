#include "app.h"  

void main_task(intptr_t unused){

    ev3_lcd_set_font(EV3_FONT_MEDIUM);

    ev3_motor_config(EV3_PORT_A, LARGE_MOTOR);

    int32_t t = ev3_motor_get_counts(EV3_PORT_A);
    
    ev3_motor_rotate(EV3_PORT_A, -1, 100, 1);

    int32_t k = ev3_motor_get_counts(EV3_PORT_A);

    char buff[20];

    sprintf(&buff, "Pos: %d", t);

    ev3_lcd_draw_string(buff, 0, 0);

    sprintf(&buff, "Pos: %d", k);

    ev3_lcd_draw_string(buff, 0, 18);

}

