#include "ev3api.h"
#include "app.h"

#include <t_stddef.h>
#include <t_syslog.h>
#include <string.h>


void main_task(intptr_t unused) {

    ev3_lcd_set_font(EV3_FONT_MEDIUM);


    ev3_sensor_config(EV3_PORT_1, PIXYCAM_2);

    pixycam_2_block block;
    char buffer[20];
    int counter = 0;

    while(1){
        

        pixycam_2_get_blocks(EV3_PORT_1, &block, 3, 255);

        sprintf(&buffer, "Size: %hu", block.sync);

        ev3_lcd_draw_string(buffer, 0, EV3_LCD_HEIGHT / 2);

        tslp_tsk(500);
        counter++;
    }


    ev3_lcd_draw_string("Done         ", 0, EV3_LCD_HEIGHT/2);

}
