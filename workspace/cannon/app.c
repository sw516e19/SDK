#include "ev3api.h"
#include "app.h"

#include "libcpp-test.h"

#include <t_stddef.h>
#include <t_syslog.h>
#include <string.h>
#include "platform_interface_layer.h"
#include "api_common.h"

#include <math.h>

/**
 * Fall length = tan(20) * 1 meter * 2 (2 pices) 
 * Resolution = Fall length / 208 pixel = 0.0035 meter/pixel
 * Gravityacceleration: 9.815 m/s^2 / 0.0035 m/pixel = 2804 pixel/s^2
 * Recalc to milliseconds = 2804 p/s^2 * 1/1000 s/ms * 1/1000 s/ms = 0.00280428571428571428571428571429 pixel/ms^2
 * Recalc to microseconds = 2804 p/s^2 * 1/1000000 s/µs * 1/1000000 s/µs = 2.8042857142857142857142857142857E-9
 */
const double GRAVITY_PIXELS = 2.804285e-09; // Measured in pixels/microsecond. Fomular: (tan(20) * 1 * 2) / 208 = 0.0035 m/pixel. 9.815 m/s^2 / 0.0035 m/pixel = 2804 pixel/second
#define PIXYCAM_RESPONSE_THRESHOLD 5 // The maximum amount of responses that we want to retrieve
#define PIXYCAM_BLOCK_THRESHOLD 1 // The maximum amount of pixycam blocks we want to retrieve in the response
/**
 * Resolution = 0.0035 m/pixel
 * Distance from camera to shooter = 0.66 m
 * Drop in flight = 0.09 m
 * Falltotal = (0.66 m + 0.09 m) / 0.0035 m/pixel = 214 pixel
 * Offset = 208 / 2 = 104 pixel
 * Total = 214 pixel + 104 pixel = 318 pixel 
 */
#define POINT_OF_IMPACT 318 // the calculated point of impact at a distance of a meter
#define GEARING 5
#define PROJECTILE_TRAVEL_TIME 135 * 1000 //Microseconds

// Enable debugging
#define DEBUG

// Enable WCRTA (Worst-case response time analysis)
#define WCRTA_NO

// Global variable for trigger time in microseconds
uint32_t trigger_time = 90 * 1000;

void printshoot_time()
{
    char str[30];
    sprintf(str, "Trigger time: %lu", trigger_time);
    ev3_lcd_draw_string(str, 0, EV3_LCD_HEIGHT / 2);
#ifdef DEBUG
    syslog(LOG_NOTICE, str);
#endif
}

// Function to modify the trigger time
void modify_trigger_time(intptr_t datapointer)
{
    int16_t addtime = (int16_t)datapointer;
    trigger_time = trigger_time + addtime;
    printshoot_time();
}

void rotate_moter(intptr_t datapointer)
{
    int16_t rotate = (int8_t)datapointer;
#ifdef DEBUG
    syslog(LOG_NOTICE, " %d change", rotate);
#endif
    ev3_motor_rotate(EV3_PORT_A, rotate * GEARING, 100, true);
}

// The first task that is executed, which will perform initial setup that cannot be done in other tasks
void init_task(intptr_t unused) {

#ifdef DEBUG
    syslog(LOG_NOTICE, "Started init task");
#endif

    // Create EV3 button clicked events
    int16_t incr = 5000;
    int16_t decr = -5000;
    ev3_button_set_on_clicked(BRICK_BUTTON_DOWN, modify_trigger_time, decr);
    ev3_button_set_on_clicked(BRICK_BUTTON_UP, modify_trigger_time, incr);
    int8_t motor_incr = 5;
    int8_t motor_decr = -5;
    ev3_button_set_on_clicked(BRICK_BUTTON_LEFT, rotate_moter, motor_decr);
    ev3_button_set_on_clicked(BRICK_BUTTON_RIGHT, rotate_moter, motor_incr);
    ev3_lcd_set_font(EV3_FONT_MEDIUM);
    printshoot_time();

#ifdef DEBUG
    syslog(LOG_NOTICE, "Init task finished");
#endif

}

// The struct of the detected pixycam block response. Contains the detection time and current block index to be parsed by calculate_task.
typedef struct {
    uint16_t y;
    SYSUTM timestamp;
} detected_pixycam_block_t;

// Global variable for the block that was detected
detected_pixycam_block_t detected_blocks[CAMDATAQUEUESIZE + 1];
SYSUTM calculated_deadlines[CALCDATAQUEUESIZE + 1];

// Detect an object with the PixyCam and write the block to a buffer to be further processed
void detect_task(intptr_t unused) {

#ifdef DEBUG
    syslog(LOG_NOTICE, "Detect task init");
#endif
    // Initialize the pixycam
    ev3_sensor_config(EV3_PORT_1, PIXYCAM_2);

    // Declare and assign signature and num_blocks variables
    block_signature_t signatures = SIGNATURE_1;

    pixycam2_block_t pixycam_block[PIXYCAM_BLOCK_THRESHOLD];
    pixycam2_block_response_t pixycam_response;
    pixycam_response.blocks = pixycam_block;
    uint8_t index = 0;

    pixycam_response.header.payload_length = 0;
    intptr_t output_ptr;
#ifdef WCRTA
    SYSUTM startTime, endTime;
#endif

#ifdef DEBUG
    syslog(LOG_NOTICE, "Detect task finished init");
#endif

    // Begin detect_task loop
    while (true) {
        // Call get blocks
        pixycam_2_sendblocks(EV3_PORT_1, signatures, 1); // Time: 0
        
        // Sleep to let other tasks do some processing
        while (pixycam_2_fetch_blocks(EV3_PORT_1, &pixycam_response, 1) == 0)
            tslp_tsk(2);
#ifdef WCRTA
        get_utm(&startTime);
#endif        

        // If the payload length is 0, no block(s) were detected and the loop should be continued

        if(pixycam_response.header.payload_length == 0) 
            continue; // Time: 17

        if(pixycam_response.blocks[0].signature != signatures)
            continue;

#ifdef DEBUG
        syslog(LOG_NOTICE, "[detect] =>DTQ");
#endif
        // Get the detection time
        get_utm(&detected_blocks[index].timestamp); // Time: 17
        detected_blocks[index].y = pixycam_response.blocks[0].y_center;

        output_ptr = (intptr_t) &detected_blocks[index];

        snd_dtq(CAMDATAQUEUE, output_ptr);
        index++;
        if(index > CAMDATAQUEUESIZE) {
            index = 0;
        }

        pixycam_response.header.payload_length = 0;  
        pixycam_response.blocks[0].signature = -1;   
#ifdef WCRTA
        get_utm(&endTime);
        syslog(LOG_NOTICE, "[WCRTA] dect: %lu", endTime - startTime);
#endif
    }
}

int32_t calculate_fallduration(uint16_t *y0_location, uint16_t *y1_location, SYSUTM *y0_time, SYSUTM *y1_time) {

    // Dereference the values. POTENTIALLY TURN TO PASS-BY-VALUE, LOOK THROUGH LATER
    uint16_t y_0 = *y0_location;
    uint16_t y_1 = *y1_location;
    //int32_t falltime = *y1_time - *y0_time;  //Original value.
    int32_t falltime = 16231; //Hardcoded value. Measured. Is approximatly 1/61.58 seconds in µseconds.

    #ifdef DEBUG
    syslog(LOG_NOTICE, "Y0: %d Y1: %d", y_0, y_1);
    #endif

    //double milis_dec = falltime / 1000; //is this actually necessary?
    
    // 1. Find average fall speed
    double v_avg = (double)(y_1 - y_0) / falltime;

    // 2. Find v_0 by subtracting the gained velocity in half the sample time.
    double v_0 = v_avg - GRAVITY_PIXELS * falltime * 0.5;

    // 3. Calculate the milliseconds needed to fall to the point of impact (POI) use rewrite of:
    // x = x_0 + v_0 * t + 0.5 * g * t² => t = (sqrt(2 a (y - x) + v^2) - v)/a and a!=0
    return round((sqrt(2 * GRAVITY_PIXELS * (POINT_OF_IMPACT - y_0) + pow(v_0, 2)) - v_0) / GRAVITY_PIXELS);
}

// Perform calculations on the data that the pixycam detected, and estimate when to shoot the target
void calculate_task(intptr_t unused) {

#ifdef DEBUG
    syslog(LOG_NOTICE, "Calculate task init");
#endif

    uint32_t current_time_to_shoot = 0;
    uint32_t sumfall = 0;
    uint32_t avgfalltime = 0;
    uint32_t offsetFromFirstDetect = 0;
    uint16_t count;
    SYSUTM firstDetect;
    uint8_t queue_index = 0;

#ifdef DEBUG
    syslog(LOG_NOTICE, "Calculate task init finished");
#endif
    intptr_t current_ptr, output_ptr;
    detected_pixycam_block_t *currentdata, *olddata = NULL;
#ifdef WCRTA
    SYSUTM startTime, endTime;
#endif

    while (true) {
        //TODO: Add timeout.
#ifdef DEBUG
        syslog(LOG_NOTICE, "[calc] DTQ=>");
#endif
        rcv_dtq(CAMDATAQUEUE, &current_ptr);
#ifdef WCRTA
        get_utm(&startTime);
#endif
        
        currentdata = (detected_pixycam_block_t*)current_ptr;

        if(olddata == NULL) {
#ifdef DEBUG
            syslog(LOG_NOTICE, "RESET CALC: first shot");
#endif
            firstDetect = currentdata->timestamp;
            olddata = currentdata;
            sumfall = 0;
            count = 0;
            continue;
        }

        if(currentdata->timestamp - olddata->timestamp > 1 * 1000 * 1000) {
#ifdef DEBUG
            syslog(LOG_NOTICE, "RESET CALC: > 1 second old");
#endif
            firstDetect = currentdata->timestamp;
            olddata = currentdata;
            sumfall = 0;
            count = 0;
            continue;
        }
        if(olddata->y == currentdata->y) {
            continue;
        }
        current_time_to_shoot = calculate_fallduration(&olddata->y, &currentdata->y, &olddata->timestamp, &currentdata->timestamp);
        
        offsetFromFirstDetect = currentdata->timestamp - firstDetect + current_time_to_shoot;
        count++;

        sumfall = sumfall + offsetFromFirstDetect;
        avgfalltime = sumfall / count;
#ifdef DEBUG
        syslog(LOG_NOTICE, "timestamp: %lu", currentdata->timestamp);
        syslog(LOG_NOTICE, "firstDetect %lu", firstDetect);
        syslog(LOG_NOTICE, "y: %d", currentdata->y);
        syslog(LOG_NOTICE, "ctts: %d", current_time_to_shoot);
        syslog(LOG_NOTICE, "sumfall: %d", sumfall);
        syslog(LOG_NOTICE, "avg: %d", avgfalltime);
#endif

        calculated_deadlines[queue_index] = firstDetect + avgfalltime - trigger_time - PROJECTILE_TRAVEL_TIME;
        output_ptr = (intptr_t) &calculated_deadlines[queue_index];

#ifdef DEBUG
        syslog(LOG_NOTICE, "[calc] =>DTQ2");
#endif
        //Write data to queue
#ifdef DEBUG
        syslog(LOG_NOTICE, "Calc shoot: %lu", calculated_deadlines[queue_index]);
#endif
        snd_dtq(CALCDATAQUEUE, output_ptr);
        queue_index++;
        if(queue_index > CALCDATAQUEUESIZE)
            queue_index = 0;

        olddata = currentdata;
#ifdef WCRTA
        get_utm(&endTime);
        syslog(LOG_NOTICE, "[WCRTA] calc: %lu", endTime - startTime);
#endif

        // 1. Wait for data to be available

        // 2. Grab data from global variable

        // 3. Begin processing on the data from the pixycam

        // 4. Once data has been processed, set when to fire the turret (raise shoot tasks priority?)
    }


    // Read data from detect_task, perform calculations and eventually pass shooting request to shoot_task

}

// Fire the cannon, and (hopefully) hit the target
void shoot_task(intptr_t unused) {

#ifdef DEBUG
    syslog(LOG_NOTICE, "Shoot task init");
#endif

    SYSUTM now;
    SYSUTM *new_data;
    
    intptr_t pointer;
    SYSUTM time_to_shoot = 0;
    ER ercd;
    // Initialize the motor
    ev3_motor_config(EV3_PORT_A, LARGE_MOTOR);
#ifdef WCRTA
    SYSUTM startTime, endTime;
#endif

    while(true) {
        
        ercd = trcv_dtq(CALCDATAQUEUE, &pointer, 2);
#ifdef WCRTA
        get_utm(&startTime);
#endif
        new_data = (SYSUTM *)pointer;
        get_utm(&now); 
        if(ercd != E_TMOUT){
#ifdef DEBUG
            SYSUTM cu;
            get_utm(&cu);
            syslog(LOG_NOTICE, "Setting new tts: %lu, Old time: %lu", *new_data, time_to_shoot);
            syslog(LOG_NOTICE, "Current TS: %lu", cu);
#endif
            if(time_to_shoot == 0) {
                if(*new_data < now) {
                    //Ignore data thats too old.
                    continue;
                }
            }
            time_to_shoot = *new_data;
        }

        if(time_to_shoot == 0) {
            continue;
        }
        if (now < time_to_shoot) {
            continue;
        }
        ev3_motor_rotate(EV3_PORT_A, 360 * GEARING, 100, true);
#ifdef DEBUG
        syslog(LOG_NOTICE, "Shot!");
        syslog(LOG_NOTICE, "now: %lu", now);
        syslog(LOG_NOTICE, "tts: %lu", time_to_shoot);
#endif
        time_to_shoot = 0;
#ifdef WCRTA
        get_utm(&endTime);
        syslog(LOG_NOTICE, "[WCRTA] shoo: %lu", endTime - startTime);
#endif
        
    }

}
