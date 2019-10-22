#include "ev3api.h"
#include "app.h"

#include "libcpp-test.h"

#include <t_stddef.h>
#include <t_syslog.h>
#include <string.h>
#include "platform_interface_layer.h"
#include "api_common.h"

#define CHECK_PORT(port) CHECK_COND((port) >= EV3_PORT_1 && (port) <= EV3_PORT_4, E_ID)

#define DEBUG

#ifdef DEBUG
#define _debug(x) (x)
#else
#define _debug(x)
#endif

static sensor_type_t sensors[TNUM_SENSOR_PORT];

static const uart_data_t *pUartSensorData = NULL;
static const analog_data_t *pAnalogSensorData = NULL;
static const i2c_data_t *pI2CSensorData = NULL;


bool_t pixycam_test(sensor_port_t port) {
	ER ercd;

	CHECK_PORT(port);
	//CHECK_COND(ev3_sensor_get_type(port) == HT_NXT_ACCEL_SENSOR, E_OBJ);
	CHECK_COND(*pI2CSensorData[port].status == I2C_TRANS_IDLE, E_OBJ);

    sensor_port_t pixycam_port = EV3_PORT_1;
    motor_port_t motor_port = EV3_PORT_2;

    // initialize sensor and motor ports
    ev3_sensor_config(pixycam_port, PIXYCAM_2);
    ev3_motor_config(motor_port, LARGE_MOTOR);

    // create block response var
    pixycam2_block_response_t response;

    uint32_t see_count = 0;

    while (true) {
        // call get blocks with the signatures
        pixycam_2_get_blocks(pixycam_port, &response, 8, 1);

        // get block count
        uint8_t block_count = response.header.payload_length / sizeof(pixycam2_block_t);

        // do stuff if there is a block
        if (block_count > 0) {
            ++see_count;
            char test[4];
            sprintf(test, "%u", see_count);
            ev3_lcd_draw_string(test, 0, 0);

            ev3_motor_set_power(motor_port, 50);
        }
    }

	return true;

error_exit:
	syslog(LOG_WARNING, "%s(): ercd %d", __FUNCTION__, ercd);
	return false;
}

// Detect an object with the PixyCam and write to a buffer to be further processed
void detect_task(intptr_t unused) {

    // Get the block of the falling object. Write it to a data structure that calculate_task can read
    pixycam_test(EV3_PORT_1); // TODO: REWRITE DIS

}

// Perform calculations on the data that the pixycam detected, and estimate when to shoot the target
void calculate_task(intptr_t unused) {

    // Read data from detect_task, perform calculations and eventually pass shooting request to shoot_task

}

// Fire the cannon, and (hopefully) hit the target
void shoot_task(intptr_t unused) {

    // Use the motor to fire the projectile

};