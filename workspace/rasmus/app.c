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
            sprintf(buffer, "W: %u | H: %u ", block[0].width, block[0].height);
            ev3_lcd_draw_string(buffer, 0, EV3_LCD_HEIGHT / 2);
            tslp_tsk(1000);
        }

    }

}

void count_colors() {

    block_signature_t signature = SIGNATURE_1;
    SYSTIM start, end, prev_img, curr_img;
    uint8_t blocks_detected = 0;
    uint8_t pictures_taken = 0;
    char str[16][40];

    for (uint8_t i = 0; i < 16; i++)
    {
        sprintf(str[i], "                                        ");
    }
    

    pixycam2_block_response_t response;
    pixycam2_block_t block[1];
    response.blocks = &block;
    bool found = false;
    ev3_led_set_color(LED_RED);
    tslp_tsk(400);
    while(!found) {
        pixycam_2_get_blocks(EV3_PORT_1, &response, signature, 1);
        found = response.header.payload_length == 14;
    }
    ev3_led_set_color(LED_ORANGE);
    get_tim(&start);
    end = start;
    prev_img = start;
    while(end - start < 400) {
        pixycam_2_get_blocks(EV3_PORT_1, &response, signature, 1);
        pictures_taken++;
        if (response.header.payload_length == 14) {
            if(response.blocks[0].signature == 1){
                blocks_detected++;
                if(blocks_detected > 0 && blocks_detected < 15) {
                    get_tim(&curr_img);
                    sprintf(str[blocks_detected + 1], "%i X:%u Y:%u %lu   ", blocks_detected, response.blocks[0].x_center, response.blocks[0].y_center, curr_img - prev_img);
                    prev_img = curr_img;
                }
                response.blocks[0].signature = 0;
                response.header.payload_length = 0;
            }
        }
        get_tim(&end);
    }
    ev3_led_set_color(LED_OFF);

    sprintf(str[0], "Pics taken: %u      ", pictures_taken);
    sprintf(str[1], "Count: %i           ", blocks_detected);
    for (int8_t i = 0; i < 16; i++)
    {
        ev3_lcd_draw_string(str[i], 0, i*8);
    }
}

void setup_count(block_signature_t signature) {


    ev3_sensor_config(EV3_PORT_1, PIXYCAM_2);

    ev3_lcd_set_font(EV3_FONT_SMALL);
    ev3_button_set_on_clicked(DOWN_BUTTON, count_colors, 0);
    while(1) {

    }

}

void main_task(intptr_t unused) {

    setup_count(SIGNATURE_1);
}
