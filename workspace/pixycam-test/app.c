#include "ev3api.h"
#include "app.h"

#include <t_stddef.h>
#include <t_syslog.h>
#include <string.h>
#include <stdbool.h>


void lcd_clear(){
    ev3_lcd_fill_rect(0, 0, EV3_LCD_WIDTH - 1, EV3_LCD_HEIGHT - 1, EV3_LCD_WHITE);

}

void main_task(intptr_t unused) {

    pixycam_2_block block;

    pixycam_2_block_response response;

    response.blocks = &block;

    bool_t touched = 0;

    ev3_sensor_config(EV3_PORT_1, PIXYCAM_2);
    ev3_sensor_config(EV3_PORT_2, TOUCH_SENSOR);

    int object_counter = 0;
    int old_object_count = 0;

    unsigned short y_coords[200];
    unsigned short x_coords[200];
    char buffer[15];

    int x_lcd = 0;
    int y_lcd = 0;

    ev3_lcd_set_font(EV3_FONT_MEDIUM);
    ev3_button_set_on_clicked(UP_BUTTON, lcd_clear, 0);

    ev3_lcd_draw_string("init done...", 0, 0);
    tslp_tsk(1000);
    lcd_clear();

    while(1){
        touched = ev3_touch_sensor_is_pressed(EV3_PORT_2);

        if(touched){
            ev3_lcd_draw_string("Tracking...", 0, 0);
            while(touched && object_counter < 200){
                touched = ev3_touch_sensor_is_pressed(EV3_PORT_2);

                pixycam_2_get_blocks(EV3_PORT_1, &response, 8, 1);
                if(response.header.payload_length > 0){
                    x_coords[object_counter] = block.x_center;
                    y_coords[object_counter] = block.y_center;
                    object_counter++;
                }
            }
            old_object_count = object_counter;

            //Write coords to file
            ev3_lcd_fill_rect(0, 0, EV3_LCD_WIDTH - 1, EV3_LCD_HEIGHT - 1, EV3_LCD_WHITE);
            ev3_lcd_set_font(EV3_FONT_SMALL);
            for(int i = 0; i < old_object_count; i++){

                sprintf(&buffer, "%u.%u;", x_coords[i], y_coords[i]);
                
                ev3_lcd_draw_string(buffer, x_lcd * 50, y_lcd * 8);
                x_lcd++;
                if(((x_lcd + x_lcd) * 50) >= EV3_LCD_WIDTH){
                    y_lcd++;
                    x_lcd = 0;
                }
                if(((y_lcd + y_lcd) * 8) >= EV3_LCD_HEIGHT){
                    y_lcd = 0;
                    x_lcd = 0;
                    break;
                }
            }
            y_lcd = 0;
            x_lcd = 0;
        }else{
            object_counter = 0;
        }

