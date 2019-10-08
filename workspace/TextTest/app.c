#include "ev3api.h"
#include "app.h"

#include <t_stddef.h>
#include <t_syslog.h>
#include <string.h>

#define CHECK_PORT(port) CHECK_COND((port) >= EV3_PORT_1 && (port) <= EV3_PORT_4, E_ID)

#define DEBUG

#ifdef DEBUG
#define _debug(x) (x)
#else
#define _debug(x)
#endif

int main_task(intptr_t unused){
    int lcdx = 0;
    int lcdy = 0;
    ev3_font_get_size(EV3_FONT_SMALL,lcdx,lcdy);
    
    
    int MainCounter = 0;
    ev3_lcd_set_font(EV3_FONT_SMALL);
    ev3_lcd_draw_string("Starting priority tasks",0,0);

    act_tsk(HIGHPRIO_TASK);

    while(1)
    {   
        if (MainCounter > 100)
        {
            ev3_lcd_draw_string("main loop",0,0);
            MainCounter++;
            break;
        }
        //ev3_lcd_draw_string("main loop: %d",0,1);
    }
    return 1;
}

void highprio_task(intptr_t unused){
    act_tsk(MIDPRIO_TASK);
    
    int lcdxhigh = 0;
    int lcdyhigh = 0;
    ev3_font_get_size(EV3_FONT_SMALL,lcdxhigh,lcdyhigh);
    lcdxhigh = lcdyhigh * 2;
    lcdyhigh = lcdxhigh * 2;

    ev3_lcd_draw_string("High prio task initial",lcdxhigh,lcdyhigh);
    
    int HighPrioCounter = 0;
    int HighPrioSleepCounter = 0;
    while (1){
        HighPrioCounter++;
        //ev3_lcd_draw_string("High priority task has run for %d iterations, slept %d times",HighPrioCounter, HighPrioSleepCounter,0,2);

        if(HighPrioCounter > 10){
            HighPrioSleepCounter++;
            HighPrioCounter = 0;
            ev3_lcd_draw_string("Highpriotask running",lcdxhigh,lcdyhigh);
            //tslp_tsk(1000);
        }
        if(HighPrioSleepCounter > 10){
            ev3_lcd_draw_string("Suspending highpriotask",lcdxhigh,lcdyhigh);
            sus_tsk(HIGHPRIO_TASK);
        }
    }
}

void midprio_task(intptr_t unused){
    act_tsk(LOWPRIO_TASK);

    int lcdxmid = 0;
    int lcdymid = 0;
    ev3_font_get_size(EV3_FONT_SMALL,lcdxmid,lcdymid);
    lcdxmid = lcdymid * 2;
    lcdymid = lcdxmid * 2;


    int MidPrioCounter = 0;
    int MidPrioSleepCounter = 0;
    while (1)
    { 
        MidPrioCounter++;
        //ev3_lcd_draw_string("MPT has run for %d iterations, slept %d times",MidPrioCounter, MidPrioSleepCounter,0,3);
        if(MidPrioCounter > 100){
            MidPrioSleepCounter++;
            MidPrioCounter = 0;
            ev3_lcd_draw_string("Mid Prio task slept",15,15);
            //slp_tsk();
            //tslp_tsk(500);
        }
        if(MidPrioSleepCounter > 50){
            ev3_lcd_draw_string("Mid Prio Task Suspended",40,40);
            //sus_tsk(MIDPRIO_TASK);
            slp_tsk();
        }
    }
}

void lowprio_task(intptr_t unused){

    int lcdxlow = 0;
    int lcdylow = 0;
    ev3_font_get_size(EV3_FONT_SMALL,lcdxlow,lcdylow);
    lcdxlow = lcdylow * 2;
    lcdylow = lcdxlow * 2;

    int LowPrioCounter = 0;
    while(1)
    {
        LowPrioCounter++;
        ev3_lcd_draw_string("lowprio running",lcdxlow,lcdylow);
        //ev3_lcd_draw_string("LPT has run for %d iterations",LowPrioCounter,0,4);
        if(LowPrioCounter > 100){
            //sus_tsk(LOWPRIO_TASK);
            slp_tsk();
        }
    }
}

