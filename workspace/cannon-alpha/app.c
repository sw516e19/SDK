#include "app.h"  

void fire(){
    ev3_motor_rotate(EV3_PORT_A, - 360, 100, 0);
}

void main_task(intptr_t unused){

    ev3_lcd_set_font(EV3_FONT_MEDIUM);

    ev3_motor_config(EV3_PORT_A, LARGE_MOTOR);

    bool_t stop = 0;

    ev3_button_set_on_clicked(BACK_BUTTON, fire, 0);

    while(1);
    

}


