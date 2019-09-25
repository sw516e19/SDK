#include "ev3api.h"
#include "app.h"

#include "libcpp-test.h"
//#include "Clock.h"
//#include "Motor.h"

#include <t_stddef.h>
#include <t_syslog.h>
#include <string.h>
#include "platform_interface_layer.h"
#include "api_common.h"

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





void main_task(intptr_t unused) {

    ev3_lcd_set_font(EV3_FONT_MEDIUM);


    ev3_lcd_draw_string("HELLO WORLD", 0, 0);
    sprintf("awer", 0, 0);

    brickinfo_t brickinfo;
    ER ercd = fetch_brick_info(&brickinfo);
    pUartSensorData = brickinfo.uart_sensors;
    pAnalogSensorData = brickinfo.analog_sensors;
    pI2CSensorData = brickinfo.i2c_sensors;

    ev3_lcd_draw_string("SWAG", 0, 0);




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
