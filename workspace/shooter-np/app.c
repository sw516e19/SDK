#include "app.h"
#include "ev3api.h"

#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <t_stddef.h>
#include <t_syslog.h>

typedef struct
{
    uint16_t firstX;
    uint16_t firstY;
    SYSTIM firstDetected;
    uint16_t lastX;
    uint16_t lastY;
    SYSTIM lastDetected;
} ShooterData_t;

void main_shooter()
{
    syslog(LOG_WARNING, "main_shooter");
    const sensor_port_t pixycamPort = EV3_PORT_1; //Which port is the pixy cam on
    const motor_port_t motorPort = EV3_PORT_A;    //Which port is the motor on
    const uint8_t signature = SIGNATURE_1;        //Which signature to shoot
    const uint16_t fireDuration = 92;             //Time to fire a shot
    const uint16_t flightTime = 135;              //Time in flight (depends on distance of the projectile)
    const uint16_t yTargetLocation = 208;         //Target location of shooting window
    const int16_t rotation = 5 * 360;             //Rotation in degrees.
    const int8_t speed = 100;                     //Percentage of speed (-100 to 100). Negative is reverse.

    const uint16_t totalTriggerTime = fireDuration + flightTime;
    ev3_sensor_config(pixycamPort, PIXYCAM_2);
    ev3_motor_config(motorPort, LARGE_MOTOR);

    ShooterData_t data;
    SYSTIM shootTime;
    while (1)
    {
        detectobj(pixycamPort, signature, &data);
        calculateIntersection(data, totalTriggerTime, yTargetLocation, &shootTime);
        shootobj(motorPort, shootTime, rotation, speed);
        if (shootTime != NULL)
        {
            tslp_tsk(5 * 1000); //Sleep for 5 seconds after each shot.
        }
        cleanData(&data, &shootTime);
    }
}

void detectobj(sensor_port_t pixycamPort, uint8_t signature, ShooterData_t *data)
{
    syslog(LOG_WARNING, "detectobj");
    uint8_t found = 0;
    pixycam2_block_response_t response;
    pixycam2_block_t block[1];
    response.blocks = &block;

    while (found != 1)
    {
        tslp_tsk(50);
        pixycam_2_get_blocks(pixycamPort, &response, signature, 1);
        if (response.header.payload_length == 14)
        {
            if (response.blocks[0].signature == signature)
            {
                SYSTIM detectTime;
                get_tim(&detectTime);
                data->firstDetected = detectTime;
                data->firstX = response.blocks[0].x_center;
                data->firstY = response.blocks[0].y_center;
                found = 1;
            }
        }
    }
    syslog(LOG_WARNING, "1st detect");

    tslp_tsk(16 * 4);
    pixycam_2_get_blocks(pixycamPort, &response, signature, 1);
    if (response.header.payload_length != 14)
    {
        syslog(LOG_WARNING, "2nd missing");
        return;
    }
    get_tim(&data->lastDetected);
    data->lastX = response.blocks[0].x_center;
    data->lastY = response.blocks[0].y_center;
    syslog(LOG_WARNING, "2nd detected");
}

void calculateIntersection(ShooterData_t data, uint16_t triggerDuration, uint16_t yTargetLocation, SYSTIM *dateTime)
{
    syslog(LOG_WARNING, "calculateIntersection");
    if (data.lastDetected == NULL)
    {
        return;
        // Object not detected 2nd time.
    }
    int16_t xDifference = data.lastX - data.firstX;
    int16_t yDifference = data.lastY - data.firstY;
    int16_t fallSampleDuration = data.lastDetected - data.firstDetected;
    //Fall = tan(20) * 1 meter = 0.364 meter * 2 = 0.728 m.
    //Pixels: 0.728 m / 208 pixels = 0,0035 m per pixel (Or 0,35 cm per pixel)
    //Gravity acceleration = 9,815 m/s^2 / 0,0035 m / pixel = 2804.285714285714 pixel/s^2
    //Changed to pixel / ms^2  = 2804,285714285714 pixel/s^2 / (1000*1000) = 0,002804285714285714

    double gravityPixels = 0.002804285714285714; // Measured in pixels/ms
    if (yDifference < 1)
    {
        syslog(LOG_WARNING, "yDifference less than 1: %d!", yDifference);
        return;
    }
    int16_t avgFallVelocity = yDifference / fallSampleDuration;
    if (avgFallVelocity <= 0)
    {
        syslog(LOG_WARNING, "Velocity negative!");
        syslog(LOG_WARNING, "avgFallVelocity: %d", avgFallVelocity);
        return;
    }

    int16_t midwayFallSamplePoint = fallSampleDuration / 2;
    syslog(LOG_WARNING, "midwayFallSamplePoint: %d", midwayFallSamplePoint);

    int16_t calculatedMidpointX = data.firstY + 0.5 * gravityPixels * pow(midwayFallSamplePoint, 2);
    syslog(LOG_WARNING, "calculatedMidpointX: %d", calculatedMidpointX);

    uint16_t delayBeforeShot = (sqrt(-2 * gravityPixels * calculatedMidpointX + 2 * gravityPixels * yTargetLocation + pow(avgFallVelocity, 2)) - avgFallVelocity) / gravityPixels;
    syslog(LOG_WARNING, "delayBeforeShot: %u", delayBeforeShot);

    //delayBeforeShot = 6000;

    if (delayBeforeShot < triggerDuration)
    {
        // this is bad. We cannot shoot in time...
    }

    *dateTime = data.firstDetected + midwayFallSamplePoint + (delayBeforeShot - triggerDuration);

    syslog(LOG_WARNING, "delayBeforeShot - triggerDuration: %u", (delayBeforeShot - triggerDuration));

    SYSTIM currentSystim;
    get_tim(&currentSystim);
    syslog(LOG_WARNING, "current: %lu", currentSystim);
    syslog(LOG_WARNING, "datetime: %lu", *dateTime);
}

void shootobj(motor_port_t motorPort, SYSTIM fireTime, int rotation, int8_t speed)
{
    syslog(LOG_WARNING, "shootobj");
    if (fireTime == NULL)
    {
        return;
    }

    SYSTIM currentTime;
    get_tim(&currentTime);
    uint16_t counter = 0;
    syslog(LOG_WARNING, "fireTime2: %lu", fireTime);
    while (fireTime > currentTime)
    {
        get_tim(&currentTime);
    }

    ev3_motor_rotate(motorPort, rotation, speed, false);
}
void cleanData(ShooterData_t *data, SYSTIM *shootTime)
{
    data->firstDetected = NULL;
    data->firstX = NULL;
    data->firstY = NULL;
    data->lastDetected = NULL;
    data->lastX = NULL;
    data->lastY = NULL;
    shootTime = NULL;
}

void nonblockpixycam(){
    const sensor_port_t pixycamPort = EV3_PORT_1; //Which port is the pixy cam on
    pixycam2_block_response_t response;
    pixycam2_block_t block[1];
    response.blocks = &block;
    ev3_sensor_config(pixycamPort,PIXYCAM_2);

    pixycam_2_sendblocks(pixycamPort, SIGNATURE_1, 1);

    tslp_tsk(16);

    pixycam_2_fetch(pixycamPort, &response,1);

    syslog(LOG_WARNING, "nbpc: %d",response.header.payload_length);
}

void pixycamblocked(){
    const sensor_port_t pixycamPort = EV3_PORT_1; //Which port is the pixy cam on
    pixycam2_block_response_t response;
    pixycam2_block_t block[1];
    response.blocks = &block;
    ev3_sensor_config(pixycamPort,PIXYCAM_2);
    
    pixycam_2_get_blocks(pixycamPort,&response,1,1);

    syslog(LOG_WARNING, "bpc: %d",response.header.payload_length);
}

void main_task(intptr_t unused)
{
    syslog(LOG_WARNING, "main_task");
    ev3_button_set_on_clicked(UP_BUTTON,nonblockpixycam,0);
    ev3_button_set_on_clicked(DOWN_BUTTON,pixycamblocked,0);

    while (1){}
    
    //main_shooter();
}

