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
    const sensor_port_t pixycamPort = EV3_PORT_1;
    const sensor_port_t motorPort = EV3_PORT_A;
    const uint8_t signature = SIGNATURE_1;
    const uint16_t triggerDuration = 50;
    const uint16_t yTargetLocation = 950;
    ev3_sensor_config(pixycamPort, PIXYCAM_2);
    ev3_motor_config(motorPort, LARGE_MOTOR);
    //set_tim(0);

    ShooterData_t data;
    SYSTIM shootTime;
    while (1)
    {

        detectobj(pixycamPort, signature, &data);
        calculateIntersection(data, triggerDuration, yTargetLocation, &shootTime);
        shootobj(motorPort, shootTime);
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
        //syslog(LOG_WARNING, "while");
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
    syslog(LOG_WARNING, "2nd done");
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
    double gravityPixels = 0.2804321321321; // TODO: Calculate more precisely
    if (yDifference < 1)
    {
        // Movement not calculatable.
    }
    int16_t avgFallVelocity = yDifference / fallSampleDuration;
    if (avgFallVelocity <= 0)
    {
        // WRONG worng wrong
    }

    int16_t midwayFallSamplePoint = fallSampleDuration / 2;
    syslog(LOG_WARNING, "midwayFallSamplePoint: %d", midwayFallSamplePoint);

    int16_t calculatedMidpointX = data.firstY + 0.5 * gravityPixels * pow(midwayFallSamplePoint, 2);
    syslog(LOG_WARNING, "calculatedMidpointX: %d", calculatedMidpointX);

    uint16_t delayBeforeShot = (sqrt(-2 * gravityPixels * calculatedMidpointX + 2 * gravityPixels * yTargetLocation + pow(avgFallVelocity, 2)) - avgFallVelocity) / gravityPixels;
    syslog(LOG_WARNING, "delayBeforeShot: %u", delayBeforeShot);

    delayBeforeShot = 6000;

    if (delayBeforeShot < triggerDuration)
    {
        // this is bad. We cannot shoot in time...
    }


    dateTime = data.firstDetected + midwayFallSamplePoint + (delayBeforeShot - triggerDuration);

    syslog(LOG_WARNING, "delayBeforeShot - triggerDuration: %u", (delayBeforeShot - triggerDuration));

    SYSTIM currentSystim;
    get_tim(&currentSystim);
    syslog(LOG_WARNING, "current: %lu", currentSystim);
    syslog(LOG_WARNING, "datetime: %lu", dateTime);
    syslog(LOG_WARNING, "diff: %d", (int64_t) dateTime - currentSystim);
}

void shootobj(motor_port_t motorPort, SYSTIM fireTime)
{
    SYSTIM fireTime2;
    syslog(LOG_WARNING, "shootobj");
    if (fireTime == NULL)
    {
        get_tim(&fireTime2);
    }
    else
    {
        fireTime2 = fireTime;
    }
    SYSTIM currentTime;
    get_tim(&currentTime);
    if (currentTime > fireTime)
    {
        // Once again... Too late.
    }
    else
    {
    }
    while(fireTime > currentTime) {}
    
    ev3_motor_rotate(motorPort, 5 * 360, 100, false);
}
void cleanData(ShooterData_t *data, SYSTIM *shootTime)
{
    syslog(LOG_WARNING, "cleanData");
    data->firstDetected = NULL;
    data->firstX = NULL;
    data->firstY = NULL;
    data->lastDetected = NULL;
    data->lastX = NULL;
    data->lastY = NULL;
    shootTime = NULL;
}

void main_task(intptr_t unused)
{
    syslog(LOG_WARNING, "main_task");

    main_shooter();
}
