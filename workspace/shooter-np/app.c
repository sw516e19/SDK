#include "app.h"
#include "ev3api.h"

#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <t_stddef.h>
#include <t_syslog.h>

#define MOTOR_GEAR_RATIO 5
const motor_port_t MOTOR_PORT = EV3_PORT_A; //Which port is the motor on
const sensor_port_t PIXY_PORT = EV3_PORT_1; //Which port is the pixy cam on
//Fall = tan(20) * 1 meter = 0.364 meter * 2 = 0.728 m.
//Pixels: 0.728 m / 208 pixels = 0,0035 m per pixel (Or 0,35 cm per pixel)
//Gravity acceleration = 9,815 m/s^2 / 0,0035 m / pixel = 2804.285714285714 pixel/s^2
//Changed to pixel / ms^2  = 2804,285714285714 pixel/s^2 / (1000*1000) = 0,002804285714285714
const double GRAVITY_PIXELS = 0.002804285714285714; // Measured in pixels/ms
const uint8_t PIXY_SIGNATURE = SIGNATURE_1;
const uint8_t FLIGHT_TIME = 135;      //Flight time of projectile, measured in milliseconds.
const uint8_t TRIGGER_MECHANISM = 90; //Time for the trigger mechanism to fire.
#define CALCPIXEL (104 + (66 + 9) / 0.35)
const uint16_t Y_TARGET_LOCATION = CALCPIXEL;          //Target location in Y-pixel. Calculation: shooting window 104 pixel + (53.1 cm + 9 cm) / 0.35 cm per pixel = 281
const int16_t SHOOT_ROTATION = MOTOR_GEAR_RATIO * 360; //Make 1 revolution when shooting
const int8_t SHOOT_SPEED = 100;                        //Shoot speed
int16_t TOTAL_FIRE_TIME;                               //Set in main_shooter()
int8_t SHOT_COUNT = 0;

// Enable debug
#define DEBUG

typedef struct
{
    uint16_t firstX;
    uint16_t firstY;
    SYSTIM firstDetected;
    uint16_t lastX;
    uint16_t lastY;
    SYSTIM lastDetected;
} ShooterData_t;

void detectobj(sensor_port_t pixycamPort, uint8_t signature, ShooterData_t *data)
{
    uint8_t found = 0;
    pixycam2_block_response_t response;
    pixycam2_block_t block[1];
    response.blocks = block;

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

    tslp_tsk(16 * 4);
    pixycam_2_get_blocks(pixycamPort, &response, signature, 1);
    if (response.header.payload_length != 14)
    {
        //2nd object not found.
#ifdef DEBUG
        syslog(LOG_NOTICE, "NO-OP: detectobj, 2ndobj");
#endif
        return;
    }
    SYSTIM detectTime2;
    get_tim(&detectTime2);
    data->lastDetected = detectTime2;
    data->lastX = response.blocks[0].x_center;
    data->lastY = response.blocks[0].y_center;
}

void calculateIntersection(ShooterData_t *data, int16_t triggerDuration, uint16_t yTargetLocation, SYSTIM *dateTime)
{
    if (data->lastDetected == 0)
    {
        // Object not detected 2nd time.
#ifdef DEBUG
        syslog(LOG_NOTICE, "NO-OP: calcI, lastdect");
#endif
        return;
    }
    //int16_t xDifference = data->lastX - data->firstX;
    int16_t yDifference = data->lastY - data->firstY;
    int16_t fallSampleDuration = data->lastDetected - data->firstDetected;

    if (yDifference < 1)
    {
        //Y-difference less than 1.
#ifdef DEBUG
        syslog(LOG_NOTICE, "NO-OP: calcI, y-diff");
#endif
        return;
    }
    double avgFallVelocity = (double)yDifference / fallSampleDuration;
    if (avgFallVelocity <= 0)
    {
        //Fall velocity less than or equal to 0.
#ifdef DEBUG
        syslog(LOG_NOTICE, "NO-OP: calcI, fallVel");
#endif
        return;
    }

    double midwayFallSamplePoint = fallSampleDuration / 2;

    double addedFallVelocity = GRAVITY_PIXELS * midwayFallSamplePoint;
    double initialFallVelocity = avgFallVelocity - addedFallVelocity;

    uint16_t delayBeforeShot; //Delay in ms before the shot.

    delayBeforeShot = (sqrt(-2 * GRAVITY_PIXELS * data->firstY + 2 * GRAVITY_PIXELS * yTargetLocation + pow(initialFallVelocity, 2)) - initialFallVelocity) / GRAVITY_PIXELS;

    *dateTime = ((ulong_t)data->firstDetected + (ulong_t)delayBeforeShot) - (ulong_t)triggerDuration;
}

void shootobj(motor_port_t motorPort, SYSTIM *fireTime, int rotation, int8_t speed)
{
    if (*fireTime == 0)
    {
#ifdef DEBUG
        syslog(LOG_NOTICE, "NO-OP: shootobj, firetime");
#endif
        return;
    }

    SYSTIM currentTime;
    get_tim(&currentTime);
    while (*fireTime > currentTime)
    {
        get_tim(&currentTime);
    }

    ev3_motor_rotate(motorPort, rotation, speed, false);
}
void cleanData(ShooterData_t *data, SYSTIM *shootTime)
{
    data->firstDetected = 0;
    data->firstX = 0;
    data->firstY = 0;
    data->lastDetected = 0;
    data->lastX = 0;
    data->lastY = 0;
    shootTime = 0;
}

typedef struct
{
    uint16_t degrees;
    int8_t speed;
} rot_data_t;

void rotateMotor(intptr_t datapointer)
{
    rot_data_t *rotdata = (rot_data_t *)datapointer;

    int16_t rotation = rotdata->degrees * MOTOR_GEAR_RATIO;

    ev3_motor_rotate(MOTOR_PORT, rotation, rotdata->speed, false);
}

void printFireTime()
{

    char str[] = "                    ";
    ev3_lcd_draw_string(str, 0, 16);
    sprintf(str, "Fire: %d ms", TOTAL_FIRE_TIME);
    ev3_lcd_draw_string(str, 0, 16);
}

void setFireTime(intptr_t datapointer)
{
    int8_t fireTime = (int8_t)datapointer;

    TOTAL_FIRE_TIME = TOTAL_FIRE_TIME + fireTime;

    printFireTime();
}

void printshot(ShooterData_t *data, SYSTIM *shootTime)
{
#ifdef DEBUG
    uint8_t loglvl = LOG_NOTICE;
    syslog(loglvl, "  ");
    syslog(loglvl, "==SHOT %d DETECTED==", SHOT_COUNT);
    syslog(loglvl, "FirstY: %d", data->firstY);
    syslog(loglvl, "FirstDetected: %lu", data->firstDetected);
    syslog(loglvl, "LastY: %d", data->lastY);
    syslog(loglvl, "LastDetected: %lu", data->lastDetected);
    syslog(loglvl, "ShootTime: %lu", *shootTime);
    syslog(loglvl, "Diff Shoot-First: %d", *shootTime - data->firstDetected);
    syslog(loglvl, "TOTAL_FIRE_TIME: %d", TOTAL_FIRE_TIME);
#endif
}

void main_shooter()
{
    TOTAL_FIRE_TIME = FLIGHT_TIME + TRIGGER_MECHANISM; //Set the initial totalFireTime

    //Data for processing
    ShooterData_t data;
    SYSTIM shootTime;
    cleanData(&data, &shootTime);
    printFireTime();
    while (1)
    {
        detectobj(PIXY_PORT, PIXY_SIGNATURE, &data);
        if (data.lastDetected != 0) //If image was found.
        {
            calculateIntersection(&data, TOTAL_FIRE_TIME, Y_TARGET_LOCATION, &shootTime);
            if (shootTime != 0) //If a shootTime was able to be determined.
            {
                shootobj(MOTOR_PORT, &shootTime, SHOOT_ROTATION, SHOOT_SPEED);
                SHOT_COUNT = SHOT_COUNT + 1;
                printshot(&data, &shootTime);
                tslp_tsk(5 * 1000); //Sleep for 5 seconds after each shot.
            }
        }
        cleanData(&data, &shootTime);
    }
}

void main_task(intptr_t unused)
{
    //Setup
    ev3_lcd_set_font(EV3_FONT_MEDIUM);
    ev3_motor_config(MOTOR_PORT, LARGE_MOTOR);
    ev3_sensor_config(PIXY_PORT, PIXYCAM_2);

    //Handle cog mechanism
    rot_data_t dataSml = {.degrees = 10, .speed = 100};
    rot_data_t dataMed = {.degrees = 20, .speed = 100};
    rot_data_t dataLrg = {.degrees = 30, .speed = 100};

    ev3_button_set_on_clicked(LEFT_BUTTON, rotateMotor, (intptr_t)&dataSml);
    ev3_button_set_on_clicked(ENTER_BUTTON, rotateMotor, (intptr_t)&dataMed);
    ev3_button_set_on_clicked(RIGHT_BUTTON, rotateMotor, (intptr_t)&dataLrg);

    //Handle firetime

    int8_t msIncrement = 5;
    int8_t msDecrement = -5;

    ev3_button_set_on_clicked(UP_BUTTON, setFireTime, msIncrement);
    ev3_button_set_on_clicked(DOWN_BUTTON, setFireTime, msDecrement);

    main_shooter();
}
