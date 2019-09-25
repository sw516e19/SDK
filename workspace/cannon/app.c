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

/*
class TestClass {

    int member;

public:
    TestClass() {
        member = 0x12345678;
    }

    void test_method() {
        static char buf[256];
        sprintf(buf, "Member is 0x%08x.", member);
        ev3_lcd_draw_string(buf, 0, 32);
    }
};

auto obj2 = new TestClass();
*/

static sensor_type_t sensors[TNUM_SENSOR_PORT];

static const uart_data_t *pUartSensorData = NULL;
static const analog_data_t *pAnalogSensorData = NULL;
static const i2c_data_t *pI2CSensorData = NULL;


bool_t pixycam_test(sensor_port_t port) {
	ER ercd;

    ev3_lcd_draw_string("3", 0, 0);

	CHECK_PORT(port);
	//CHECK_COND(ev3_sensor_get_type(port) == HT_NXT_ACCEL_SENSOR, E_OBJ);
	CHECK_COND(*pI2CSensorData[port].status == I2C_TRANS_IDLE, E_OBJ);

    ev3_lcd_draw_string("4", 0, 0);

	ercd = start_i2c_transaction(port, 0x54, "\xAE\xC1\x20\x2\xFF\xFF", 6, 20);//"\xAE\xC1\xE\x00", 4, 13);

    ev3_lcd_draw_string("5", 0, 0);

	assert(ercd == E_OK);

    ev3_lcd_draw_string("6", 0, 0);
    pixycam_2_block b;
    // spin while waiting for i2c to be idle again
    while(!((*pI2CSensorData[port].status) == I2C_TRANS_IDLE));

    if (pI2CSensorData[port].raw[0] == 175 && pI2CSensorData[port].raw[1] == 193)
    {
        char test[2];
        b = pI2CSensorData[port].raw;
        

        sprintf(test, "%hu", b.width);

        ev3_lcd_draw_string(test, 0, 0);

        //ev3_lcd_draw_string("7", 0, 0);
    }

	return true;

error_exit:
	syslog(LOG_WARNING, "%s(): ercd %d", __FUNCTION__, ercd);
	return false;
}




void main_task(intptr_t unused) {

    ev3_lcd_set_font(EV3_FONT_MEDIUM);


    ev3_lcd_draw_string("1", 0, 0);

    brickinfo_t brickinfo;
    ER ercd = fetch_brick_info(&brickinfo);
    pUartSensorData = brickinfo.uart_sensors;
    pAnalogSensorData = brickinfo.analog_sensors;
    pI2CSensorData = brickinfo.i2c_sensors;
    assert(pUartSensorData != NULL);
    assert(pAnalogSensorData != NULL);
    assert(pI2CSensorData != NULL);

    ev3_lcd_draw_string("2", 0, 0);

    pixycam_test(EV3_PORT_1);







    //motor_type_t type = LARGE_MOTOR;

    //ev3api::Motor test(ePortM::PORT_A);

/*
    // Test global constructor
    obj2->test_method();

    // Test function in static library
    libcpp_test_c_echo_function(777);

    // Test class in static library
    LibSampleClass a;
    a.draw();
*/
}
