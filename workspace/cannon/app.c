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
#define PIXYCAM_RESPONSE_THRESHOLD 5 // The maximum amount of responses that we want to retrieve
#define PIXYCAM_BLOCK_THRESHOLD 1 // The maximum amount of pixycam blocks we want to retrieve in the response
#define POINT_OF_IMPACT 281 // the calculated point of impact at a distance of a meter
#define GEARING 5
#define PROJECTILE_TRAVEL_TIME 135

// Enable debugging
#define DEBUG

// Enable WCRTA (Worst-case response time analysis)
#define WCRTA

// Global variable for trigger time
uint8_t trigger_time = 90;

// Function to modify the trigger time
void modify_trigger_time(uint8_t *change) {
    trigger_time = trigger_time + *change;
}

// The first task that is executed, which will perform initial setup that cannot be done in other tasks
void init_task(intptr_t unused) {

#ifdef DEBUG
    syslog(LOG_NOTICE, "Started init task");
#endif

    // Create EV3 button clicked events
    uint8_t incr = 5;
    uint8_t decr = -5;
    ev3_button_set_on_clicked(BRICK_BUTTON_DOWN, modify_trigger_time, decr);
    ev3_button_set_on_clicked(BRICK_BUTTON_UP, modify_trigger_time, incr);


#ifdef DEBUG
    syslog(LOG_NOTICE, "Init task finished");
#endif

}

// The struct of the detected pixycam block response. Contains the detection time and current block index to be parsed by calculate_task.
typedef struct {
    uint16_t y;
    SYSTIM timestamp;
} detected_pixycam_block_t;

// Global variable for the block that was detected
detected_pixycam_block_t detected_blocks[CAMDATAQUEUESIZE + 1];
SYSTIM calculated_deadlines[CALCDATAQUEUESIZE + 1];

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
        
        // If the payload length is 0, no block(s) were detected and the loop should be continued

        if(pixycam_response.header.payload_length == 0) 
            continue; // Time: 17

        if(pixycam_response.blocks[0].signature != signatures)
            continue;

#ifdef DEBUG
        syslog(LOG_NOTICE, "Detected block!");
#endif

        // Get the detection time
        get_tim(&detected_blocks[index].timestamp); // Time: 17
        detected_blocks[index].y = pixycam_response.blocks[0].y_center;
#ifdef DEBUG
        syslog(LOG_NOTICE, "Timestamp set: %lu", detected_blocks[index].timestamp);
#endif

        snd_dtq(CAMDATAQUEUE, &detected_blocks[index]);
        index++;
        if(index > CAMDATAQUEUESIZE) {
            index = 0;
        }

        pixycam_response.header.payload_length = 0;  
        pixycam_response.blocks[0].signature = -1;   
    }
}

int32_t calculate_fallduration(uint16_t *y_0_ptr, uint16_t *y_1_ptr, SYSTIM *y0_millis_ptr, SYSTIM *y1_millis_ptr) {

    // Dereference the values. POTENTIALLY TURN TO PASS-BY-VALUE, LOOK THROUGH LATER
    uint16_t y_0 = *y_0_ptr;
    uint16_t y_1 = *y_1_ptr;
    int millis = *y1_millis_ptr - *y0_millis_ptr;

    //double milis_dec = millis / 1000; //is this actually necessary?
    
    // 1. Find average fall speed
    double v_avg = (y_1 - y_0) / millis;

    // 2. Find v_0 by subtracting the gained velocity in half the sample time.
    double v_0 = v_avg - GRAVITY_PIXELS * millis * 0.5;

    // 3. Calculate the milliseconds needed to fall to the point of impact (POI) use rewrite of:
    // x = x_0 + v_0 * t + 0.5 * g * t² => t = (sqrt(2 a (y - x) + v^2) - v)/a and a!=0
    return round(sqrt(2 * GRAVITY_PIXELS * (POINT_OF_IMPACT - y_0) + pow(v_0, 2) - v_0) / GRAVITY_PIXELS);
}

// Perform calculations on the data that the pixycam detected, and estimate when to shoot the target
void calculate_task(intptr_t unused) {

#ifdef DEBUG
    syslog(LOG_NOTICE, "Calculate task init");
#endif

    uint32_t current_time_to_shoot = 0;
    uint32_t sumfall = 0;
    uint16_t avgfalltime = 0;
    uint16_t offsetFromFirstDetect = 0;
    uint16_t count;
    SYSTIM firstDetect;
    uint8_t queue_index = 0;
    ER ercd;

#ifdef DEBUG
    syslog(LOG_NOTICE, "Calculate task init finished");
#endif
    intptr_t current_ptr;
    detected_pixycam_block_t *currentdata, *olddata = NULL;

    while (true) {
        //TODO: Add timeout.
        rcv_dtq(CAMDATAQUEUE, &current_ptr);
        
        currentdata = (detected_pixycam_block_t*)current_ptr;
#ifdef DEBUG
        syslog(LOG_NOTICE, "Begin calculate on new block!");
        syslog(LOG_NOTICE, "timestamp: %u", currentdata->timestamp);
#endif

        if(olddata == NULL) {
#ifdef DEBUG
            syslog(LOG_NOTICE, "Old timestamp is 0, setting data...");
#endif
            firstDetect = currentdata->timestamp;
            olddata = currentdata;
            count = 0;
            continue;
        }


        if(currentdata->timestamp - olddata->timestamp > 1000) {
#ifdef DEBUG
            syslog(LOG_NOTICE, "Data is more than 1 second old. Purging.");
#endif
            firstDetect = currentdata->timestamp;
            olddata = currentdata;
            count = 0;
            continue;
        }

        current_time_to_shoot = calculate_fallduration(&olddata->y, &currentdata->y, &olddata->timestamp, &currentdata->timestamp);
        
        offsetFromFirstDetect = currentdata->timestamp - firstDetect + current_time_to_shoot;
        count++;

        sumfall = sumfall + offsetFromFirstDetect;
        avgfalltime = sumfall / count;

        calculated_deadlines[queue_index] = firstDetect + avgfalltime - trigger_time - PROJECTILE_TRAVEL_TIME;

        //Write data to queue
        snd_dtq(CALCDATAQUEUE, &calculated_deadlines[queue_index]);
        queue_index++;
        if(queue_index > CALCDATAQUEUESIZE)
            queue_index = 0;



#ifdef DEBUG
        syslog(LOG_NOTICE, "Calc shoot: %lu", calculated_deadlines[queue_index]);
        syslog(LOG_NOTICE, "Finished calculating on block!");
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

    SYSTIM now;
    SYSTIM *new_data;
    intptr_t pointer;
    SYSTIM time_to_shoot = 0;
    ER ercd;
    // Initialize the motor
    ev3_motor_config(EV3_PORT_A, LARGE_MOTOR);

    while(true) {
        
        ercd = trcv_dtq(CALCDATAQUEUE, &pointer, 2);
        new_data = (SYSTIM *)pointer;
        if(ercd != E_TMOUT){
            time_to_shoot = *new_data;
        }
        get_tim(&now); 

        if(time_to_shoot == 0) {
            continue;
        }
        if (now < time_to_shoot) {
            continue;
        }

        ev3_motor_rotate(EV3_PORT_A, 360 * GEARING, 100, true);
        time_to_shoot = 0;
        
    }

}
