#include "ev3api.h"
#include "app.h"

#include "libcpp-test.h"

#include <t_stddef.h>
#include <t_syslog.h>
#include <string.h>
#include "platform_interface_layer.h"
#include "api_common.h"

#include <math.h>

#define GRAVITY_PIXELS 0.002804285714285714 // Measured in pixels/ms
#define PIXYCAM_BLOCK_THRESHOLD 5 // The maximum amount of pixycam blocks we can detect
#define POINT_OF_IMPACT 281 // the calculated point of impact at a distance of a meter
#define GEARING 5
#define MOTOR_ROTATION_MILLIS 500
#define PROJECTILE_TRAVEL_TIME 135

// A 2d vector with an x coordinate, y coordinate, and v for velocity
typedef struct {
    double x;
    int y;
    double v;
} vec_t;

typedef struct {
    pixycam2_block_response_t pixycam_block_response[PIXYCAM_BLOCK_THRESHOLD];
    SYSTIM detection_time;
    uint8_t current_block_index;
} detected_pixycam_block_t;

int8_t direction = 1;





void write_string(const char *arr, bool_t top) {
    ev3_lcd_set_font(EV3_FONT_MEDIUM);

    int y = top ? 0 : EV3_LCD_HEIGHT / 2;
    ev3_lcd_draw_string(arr, 0, y);
}

// Global variable for the block that was detected
detected_pixycam_block_t detected_block;

// Global variable for the detect_task block index. This value is set by detect_task and shoot_task
uint8_t detect_task_block_index = 0;

// Detect an object with the PixyCam and write the block to a buffer to be further processed
void detect_task(intptr_t unused) {

    // Initialize detected_block's values
    detected_block.detection_time = 0;

    // Declare and assign signature and num_blocks variables
    block_signature_t signatures = SIGNATURE_1;
    uint8_t NUM_BLOCKS = 1;

    // Begin detect_task loop
    while (true) {
        // If the threshold has been reached, do not allow getting blocks from the pixycam
        // Since arrays are 0-based, an array of size 5 would trigger an exception if the current index is 5
        if (detect_task_block_index == PIXYCAM_BLOCK_THRESHOLD) { // Time: 0
            tslp_tsk(5);
            continue;
        }

        // Call get blocks
        pixycam_2_get_blocks(EV3_PORT_1, &detected_block.pixycam_block_response[detect_task_block_index], &signatures, NUM_BLOCKS); // Time: 0
        
        // Sleep to let other tasks do some processing
        tslp_tsk(17); // Time: 17
        
        // If the payload length is 0, no block(s) were detected and the loop should be continued
        if(detected_block.pixycam_block_response[detect_task_block_index].header.payload_length == 0)
            continue; // Time: 17

        // Get the detection time
        get_tim(&detected_block.detection_time); // Time: 17

        // In order for the calculate_task to grab the index of the current block, set it in the data structure
        // This is necessary as detect_task_block_index will be incremented, and therefore reference the next block
        detected_block.current_block_index = detect_task_block_index;

        // Increment the detect_task_block_index for detect_task to know where the next block should be read to
        ++detect_task_block_index; // Time: 17
    }
}


SYSTIM get_time_until_impact(uint16_t *y_0_ptr, uint16_t *y_1_ptr, SYSTIM *y0_millis_ptr, SYSTIM *y1_millis_ptr) {

    // Dereference the values. POTENTIALLY TURN TO PASS-BY-VALUE, LOOK THROUGH LATER
    uint16_t y_0 = *y_0_ptr;
    uint16_t y_1 = *y_1_ptr;
    int millis = *y1_millis_ptr - *y0_millis_ptr;

    //double milis_dec = millis / 1000; //is this actually necessary?
    
    // 1. Find average fall speed
    double v_avg = (y_1 - y_0) / millis;

    // 2. Find v_0 by subtracting half the acceleration from the free fall equation, e.g. 0.5 * GRAVITY_PIXELS * pow(t / 2, 2)
    double v_0 = v_avg - GRAVITY_PIXELS * millis;

    // 3. Calculate the milliseconds needed to fall to the point of impact (POI) use rewrite of:
    // x = x_0 + v_0 * t + 0.5 * g * t² => t = (sqrt(2 a (y - x) + v^2) - v)/a and a!=0
    int delta_t = round(sqrt(2 * GRAVITY_PIXELS * (POINT_OF_IMPACT - y_0) + pow(v_0, 2) - v_0) / GRAVITY_PIXELS);
    SYSTIM fall_time = *y0_millis_ptr + delta_t - PROJECTILE_TRAVEL_TIME;

    return fall_time;
}

ulong_t new_blocks_available(SYSTIM *old_stamp){

    ulong_t result = detected_block.detection_time - *old_stamp;

    old_stamp = detected_block.detection_time;

    return result;

}

SYSTIM time_to_shoot = 0;

// Perform calculations on the data that the pixycam detected, and estimate when to shoot the target
void calculate_task(intptr_t unused) {

    SYSTIM old = 0;

    SYSTIM current_time_to_shoot = 0;
    uint16_t y_0 = -1, y_1 = -1;

    while (true) {
        if (!(new_blocks_available(&old))) {
            tslp_tsk(5);
            continue;
        }
       
        if(y_0 < 0){
           y_0 = detected_block.pixycam_block_response[detected_block.current_block_index].blocks->y_center;
           continue;
        }

        // 1. check if y_1 is set, if not, set it
        // 2. if y_1 is set, do y_0 = y_1 and then set y_1
        // 3. calculate
        bool_t y_1_is_set = y_1 >= 0;
        
        if(y_1_is_set){
            y_0 = y_1;
        }
        y_1 = detected_block.pixycam_block_response[detected_block.current_block_index].blocks->y_center;


        //weight by picture, so the first picture is the least weighted
       
        current_time_to_shoot = get_time_until_impact(&y_0, &y_1, &old, &detected_block.detection_time);

        if(detected_block.current_block_index > 0){
            time_to_shoot = (time_to_shoot + current_time_to_shoot) / 2;
        }else{
            time_to_shoot = current_time_to_shoot;
        }
        

        // 1. Wait for data to be available

        // 2. Grab data from global variable

        // 3. Begin processing on the data from the pixycam

        // 4. Once data has been processed, set when to fire the turret (raise shoot tasks priority?)
    }


    // Read data from detect_task, perform calculations and eventually pass shooting request to shoot_task

}

// Fire the cannon, and (hopefully) hit the target
void shoot_task(intptr_t unused) {

    bool_t motor_running = false;
    SYSTIM now;
    bool_t await_trigger_time = false;
    bool_t await_trigger_preparation = false;


    while(true){
        /*while( !motor_running && time_to_shoot == 0 )
        tslp_tsk(1);*/
        get_tim(&now);

        await_trigger_time = now < time_to_shoot;
        await_trigger_preparation = motor_running || time_to_shoot == 0;


        if(await_trigger_preparation || await_trigger_time){
            continue;
        }

        ev3_motor_rotate(EV3_PORT_A, 360 * GEARING, 100, false);
        motor_running = true;


        while(ev3_motor_get_counts(EV3_PORT_A) != ((360 * GEARING) - 5)){
            tslp_tsk(20);

            ev3_motor_reset_counts(EV3_PORT_A);
            motor_running = false;
        }
        
    }

     
    //shoot, reload, set time_to_shoot to 0

    // Use the motor to fire the projectile
    
    // 0. Wait for available time to shoot through the global variable
    
    // 1. shoot when time is reached 

    // 2. (then lower shoot tasks priority?)



    // At the end of shoot task, reset detect task's block index to 0
    detect_task_block_index = 0;
}
