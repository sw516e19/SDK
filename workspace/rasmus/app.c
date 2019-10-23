#include "ev3api.h"
#include "app.h"

#include <t_stddef.h>
#include <t_syslog.h>
#include <string.h>
#include <stdbool.h>


void lcd_clear(){
    ev3_lcd_fill_rect(0, 0, EV3_LCD_WIDTH - 1, EV3_LCD_HEIGHT - 1, EV3_LCD_WHITE);

}

void fire(){

    ev3_led_set_color(LED_ORANGE);
    ev3_motor_rotate(EV3_PORT_A, 360, 100, 1);
    ev3_led_set_color(LED_OFF);
}

void cannon_physics_test(){
    ev3_motor_config(EV3_PORT_A, LARGE_MOTOR);
    ev3_button_set_on_clicked(DOWN_BUTTON, fire, 0);
    
    while(1){

    }
}

void measure_brick(block_signature_t signature){

    sensor_port_t pixy_port = EV3_PORT_1;

    pixycam2_block_response_t response;
    pixycam2_block_t block[1];
    response.blocks = &block;

    ev3_sensor_config(pixy_port, PIXYCAM_2);

    ev3_lcd_set_font(EV3_FONT_MEDIUM);

    char buffer[30];

    ev3_lcd_draw_string("Nothing read yet...", 0, EV3_LCD_HEIGHT / 2);

    while(1){ 

        pixycam_2_get_blocks(pixy_port, &response, signature, 1);

        if(response.header.payload_length > 0 ){
            sprintf(&buffer, "W: %u | H: %u ", block[0].width, block[0].height);
            ev3_lcd_draw_string(buffer, 0, EV3_LCD_HEIGHT / 2);
            tslp_tsk(1000);
        }

    }

}

void main_task(intptr_t unused) {

    measure_brick(SIGNATURE_1);

}
