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
    const sensor_port_t pixycamPort = EV3_PORT_1;
    const sensor_port_t motorPort = EV3_PORT_A;
    const uint8_t signature = SIGNATURE_1;
    const uint16_t triggerDuration = 250;
    const uint16_t yTargetLocation = 950; 
    ev3_sensor_config(pixycamPort, PIXYCAM_2);
    ev3_motor_config(motorPort, LARGE_MOTOR);

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
    printf("detectobject");
    uint8_t found = 0;
    pixycam2_block_response_t response;

    while (found != 1)
    {
        tslp_tsk(50);
        pixycam_2_get_blocks(pixycamPort, &response, signature, 1);
        if (response.header.payload_length == 14 &&
            response.blocks[0].signature == signature)
        {
            get_tim(&data->firstDetected);
            data->firstX = response.blocks[0].x_center;
            data->firstY = response.blocks[0].y_center;
            found = 1;
        }
    }

    tslp_tsk(16 * 4);
    pixycam_2_get_blocks(pixycamPort, &response, signature, 1);
    if(response.header.payload_length != 14) {
        return;
    }
    get_tim(&data->lastDetected);
    data->lastX = response.blocks[0].x_center;
    data->lastY = response.blocks[0].y_center;
}

void calculateIntersection(ShooterData_t data, uint16_t triggerDuration, uint16_t yTargetLocation, SYSTIM *dateTime)
{
    printf("calculateIntersection");
    if (data.lastDetected == NULL)
    {
        return;
        // Object not detected 2nd time.
    }
    int16_t xDifference = data.lastX - data.firstX;
    int16_t yDifference = data.lastY - data.firstY;
    int16_t fallSampleDuration = data.lastDetected - data.firstDetected;
    uint16_t gravityPixels = 2804.321321321; // TODO: Calculate more precisely
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

    int16_t calculatedMidpointX = data.firstY + 0.5 * gravityPixels * pow(midwayFallSamplePoint, 2);

    uint16_t delayBeforeShot = (sqrt(-2 * gravityPixels * calculatedMidpointX + 2 * gravityPixels * yTargetLocation + pow(avgFallVelocity, 2)) - avgFallVelocity) / gravityPixels;

    if (delayBeforeShot < triggerDuration)
    {
        // this is bad. We cannot shoot in time...
    }

    dateTime = data.firstDetected + midwayFallSamplePoint + (delayBeforeShot - triggerDuration);
}

void shootobj(motor_port_t motorPort, SYSTIM fireTime)
{
    printf("shootobj");
    if(fireTime == NULL) {
        return;
    }
    SYSTIM currentTime;
    get_tim(&currentTime);
    if (currentTime > fireTime)
    {
        // Once again... Too late.
    }
    else
    {
        tslp_tsk(currentTime - fireTime);
    }
    ev3_motor_rotate(motorPort, 5 * 360, 100, false);
}
void cleanData(ShooterData_t *data, SYSTIM *shootTime)
{
    printf("cleanData");
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
    main_shooter();

}
