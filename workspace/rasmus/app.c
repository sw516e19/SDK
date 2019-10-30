#include "ev3api.h"
#include "app.h"

#include <t_stddef.h>
#include <t_syslog.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

void lcd_clear()
{
    ev3_lcd_fill_rect(0, 0, EV3_LCD_WIDTH - 1, EV3_LCD_HEIGHT - 1, EV3_LCD_WHITE);
}

void fire()
{

    ev3_led_set_color(LED_ORANGE);
    ev3_motor_rotate(EV3_PORT_A, 360, 100, 1);
    ev3_led_set_color(LED_OFF);
}

void cannon_physics_test()
{
    ev3_motor_config(EV3_PORT_A, LARGE_MOTOR);
    ev3_button_set_on_clicked(DOWN_BUTTON, fire, 0);

    while (1)
    {
    }
}

void measure_brick(block_signature_t signature)
{

    sensor_port_t pixy_port = EV3_PORT_1;

    pixycam2_block_response_t response;
    pixycam2_block_t block[1];
    response.blocks = &block;

    ev3_sensor_config(pixy_port, PIXYCAM_2);

    ev3_lcd_set_font(EV3_FONT_MEDIUM);

    char buffer[30];

    ev3_lcd_draw_string("Nothing read yet...", 0, EV3_LCD_HEIGHT / 2);

    while (1)
    {

        pixycam_2_get_blocks(pixy_port, &response, signature, 1);

        if (response.header.payload_length > 0)
        {
            sprintf(buffer, "W: %u | H: %u ", block[0].width, block[0].height);
            ev3_lcd_draw_string(buffer, 0, EV3_LCD_HEIGHT / 2);
            tslp_tsk(1000);
        }
    }
}

void count_colors()
{

    block_signature_t signature = SIGNATURE_1;
    SYSTIM start, end, prev_img, curr_img;
    uint8_t blocks_detected = 0;
    uint8_t pictures_taken = 0;
    char str[16][40];

    for (uint8_t i = 0; i < 16; i++)
    {
        sprintf(str[i], "                                        ");
    }

    pixycam2_block_response_t response;
    pixycam2_block_t block[1];
    response.blocks = &block;
    bool found = false;
    ev3_led_set_color(LED_RED);
    tslp_tsk(400);
    while (!found)
    {
        pixycam_2_get_blocks(EV3_PORT_1, &response, signature, 1);
        found = response.header.payload_length == 14;
    }
    ev3_led_set_color(LED_ORANGE);
    get_tim(&start);
    end = start;
    prev_img = start;
    while (end - start < 400)
    {
        pixycam_2_get_blocks(EV3_PORT_1, &response, signature, 1);
        pictures_taken++;
        if (response.header.payload_length == 14)
        {
            if (response.blocks[0].signature == 1)
            {
                blocks_detected++;
                if (blocks_detected > 0 && blocks_detected < 15)
                {
                    get_tim(&curr_img);
                    sprintf(str[blocks_detected + 1], "%i X:%u Y:%u %lu   ", blocks_detected, response.blocks[0].x_center, response.blocks[0].y_center, curr_img - prev_img);
                    prev_img = curr_img;
                }
                response.blocks[0].signature = 0;
                response.header.payload_length = 0;
            }
        }
        get_tim(&end);
    }
    ev3_led_set_color(LED_OFF);

    sprintf(str[0], "Pics taken: %u      ", pictures_taken);
    sprintf(str[1], "Count: %i           ", blocks_detected);
    for (int8_t i = 0; i < 16; i++)
    {
        ev3_lcd_draw_string(str[i], 0, i * 8);
    }
}

void setup_count(block_signature_t signature)
{

    ev3_sensor_config(EV3_PORT_1, PIXYCAM_2);

    ev3_lcd_set_font(EV3_FONT_SMALL);
    ev3_button_set_on_clicked(DOWN_BUTTON, count_colors, 0);
    while (1)
    {
    }
}

void main_shooter()
{

    detectobj();
    calculate_intersection();
    shootobj();
}

struct harddata
{
    uint16_t firstX;
    uint16_t firstY;
    SYSTIM firstDetected;
    uint16_t lastX;
    uint16_t lastY;
    SYSTIM lastDetected;
};

void detectobj(struct harddata data)
{
    uint8_t found = 0;
    pixycam2_block_response_t response;

    while (found != 1)
    {

        pixycam_2_get_blocks(EV3_PORT_1, &response, SIGNATURE_1, 1);
        if (response.header.payload_length == 14 && response.blocks[0].signature == 1)
        {
            get_tim(&data.firstDetected);
            data.firstX = response.blocks[0].x_center;
            data.firstY = response.blocks[0].y_center;
            found = 1;
        }
    }

    tslp_tsk(16 * 4);
    pixycam_2_get_blocks(EV3_PORT_1, &response, SIGNATURE_1, 1);
    get_tim(&data.lastDetected);
    data.lastX = response.blocks[0].x_center;
    data.lastY = response.blocks[0].y_center;
}

void calculate_intersection(struct harddata data, SYSTIM shoottime)
{
    if (data.lastDetected == NULL)
    {
        //Object not detected 2nd time.
    }
    int16_t xDifference = data.lastX - data.firstX;
    int16_t yDifference = data.lastY - data.firstY;
    int16_t fallSampleDuration = data.lastDetected - data.firstDetected;
    uint16_t gravityPixels = 2804.321321321; //TODO: Calculate more precisely
    if (yDifference < 1)
    {
        //Movement not calculatable.
    }
    int16_t avgFallVelocity = yDifference / fallSampleDuration;
    if (avgFallVelocity <= 0)
    {
        //WRONG worng wrong
    }

    int16_t midwayFallSamplePoint = fallSampleDuration / 2;

    int16_t calculatedMidpointX = data.firstY + 0.5 * gravityPixels * pow(midwayFallSamplePoint, 2);
    uint16_t yTarget = 950;

    uint8_t timeThingy = (sqrt(-2 * gravityPixels * calculatedMidpointX + 2 * gravityPixels * yTarget + pow(avgFallVelocity, 2)) - avgFallVelocity) / gravityPixels;
    uint8_t triggeringDuration = 250;
    if(timeThingy < triggeringDuration) {
        // this is bad. We cannot shoot in time...
    }

    shoottime = data.firstDetected + midwayFallSamplePoint + (timeThingy - triggeringDuration);

    // x = x_0 +v_0*t + 0.5*a*t^2
    //known: x, x_0, v_0, a
    // x = yTarget
    // x_0 == lastY
    // v_0 == avgFallVelocity
    // a = 2804 pixel pr s^2

    // t = t - (chargetime + projectile time);
}

void shootobj(SYSTIM fireTime)
{
    SYSTIM currentTime;
    get_tim(&currentTime);
    if (currentTime > fireTime) {
        //Once again... Too late.
    } else {
        tslp_tsk(currentTime - fireTime);
    }
    ev3_motor_rotate(EV3_PORT_A, 5 * 360, 100, false);

}

void main_task(intptr_t unused)
{

    setup_count(SIGNATURE_1);
}