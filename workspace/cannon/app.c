#include "ev3api.h"
#include "app.h"

#include "libcpp-test.h"

#include <t_stddef.h>
#include <t_syslog.h>
#include <string.h>
#include "platform_interface_layer.h"
#include "api_common.h"

int8_t direction = 1;



void write_string(const char *arr, bool_t top) {
        ev3_lcd_set_font(EV3_FONT_MEDIUM);

    int y = top ? 0 : EV3_LCD_HEIGHT / 2;
    ev3_lcd_draw_string(arr, 0, y);
}

int write = 0;

// Detect an object with the PixyCam and write the block to a buffer to be further processed
void detect_task(intptr_t unused) {

    while (true)
    {
        ++write;

        write_string("DETECT_TASK", true);
        tslp_tsk(1000);

        //1. Get block data

        //2. Set interrupt/flag for calc task
    }

    // Get the block of the falling object. Write it to a data structure that calculate_task can read
    // TODO: REWRITE DIS

}

// Perform calculations on the data that the pixycam detected, and estimate when to shoot the target
void calculate_task(intptr_t unused) {

    char buff[30];

    

    while (true)
    {

        sprintf(&buff, "CALCULATE_TASK %d", write);

        write_string(buff, false);

        tslp_tsk(100);

        // 1. Wait for data to be available

        // 2. Grab data from global variable

        // 3. Begin processing on the data from the pixycam

        // 4. Once data has been processed, set when to fire the turret (raise shoot tasks priority?)
    }


    // Read data from detect_task, perform calculations and eventually pass shooting request to shoot_task

}

// Fire the cannon, and (hopefully) hit the target
void shoot_task(intptr_t unused) {

    // Use the motor to fire the projectile
    
    // 0. Wait for available time to shoot through the global variable
    
    // 1. shoot when time is reached 

    // 2. (then lower shoot tasks priority?)

}
