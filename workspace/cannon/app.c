#include "ev3api.h"
#include "app.h"

#include "libcpp-test.h"

#include <t_stddef.h>
#include <t_syslog.h>
#include <string.h>
#include "platform_interface_layer.h"
#include "api_common.h"

int8_t direction = 1;



void fire1() {
    ev3_led_set_color(LED_ORANGE);
    ev3_motor_rotate(EV3_PORT_A, 1 * 360 * direction, 100, 1);
    ev3_led_set_color(LED_OFF);
}

void fire2() {
    ev3_led_set_color(LED_ORANGE);
    ev3_motor_rotate(EV3_PORT_A, 2 * 360 * direction, 100, 1);
    ev3_led_set_color(LED_OFF);
}

void fire5() {
    ev3_led_set_color(LED_ORANGE);
    ev3_motor_rotate(EV3_PORT_A, 5 * 360 * direction, 100, 1);
    ev3_led_set_color(LED_OFF);
}

void fire10() {
    ev3_led_set_color(LED_ORANGE);
    ev3_motor_rotate(EV3_PORT_A, 10 * 360 * direction, 100, 1);
    ev3_led_set_color(LED_OFF);
}

void changedirection() {
    direction = direction * -1;
}



void main_task(intptr_t unused) {
    ev3_motor_config(EV3_PORT_A, LARGE_MOTOR);
    ev3_button_set_on_clicked(UP_BUTTON, fire1, 0);
    ev3_button_set_on_clicked(RIGHT_BUTTON, fire2, 0);
    ev3_button_set_on_clicked(DOWN_BUTTON, fire5, 0);
    ev3_button_set_on_clicked(LEFT_BUTTON, fire10, 0);
    ev3_button_set_on_clicked(ENTER_BUTTON, changedirection, 0);

    while(1) {

    }
}
