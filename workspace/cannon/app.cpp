/**
 * This sample program balances a two-wheeled Segway type robot such as Gyroboy in EV3 core set.
 *
 * References:
 * http://www.hitechnic.com/blog/gyro-sensor/htway/
 * http://www.cs.bgu.ac.il/~ami/teaching/Lejos-2013/classes/src/lejos/robotics/navigation/Segoway.java
 */

#include "ev3api.h"
#include "app.h"

#include "libcpp-test.h"
#include "Clock.h"
#include "Motor.h"

#define DEBUG

#ifdef DEBUG
#define _debug(x) (x)
#else
#define _debug(x)
#endif

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

void main_task(intptr_t unused) {

    motor_type_t type = LARGE_MOTOR;

    ev3api::Motor test(ePortM::PORT_A);





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
