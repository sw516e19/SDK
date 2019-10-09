#include "ev3api.h"
#include "app.h"


int main_task(intptr_t unused){
    //setup ev3 font
    ev3_lcd_set_font(EV3_FONT_MEDIUM);

    //setup char array
    char lcdprint[30];
    //test statement, get overwritten near instantly
    ev3_lcd_draw_string("start",0,0);

    //activate tasks lower priority first
    act_tsk(LOW_TASK);
    act_tsk(HIGH_TASK);
    

    int counter = 0;
    while (1)
    {
        counter++;
        sprintf(lcdprint,"main line: %d",counter);
        ev3_lcd_draw_string(lcdprint,0,25);

        if(counter > 1000){
            counter = 0;
            ev3_lcd_draw_string("main line:          ",0,0);
            tslp_tsk(2000);
        }
    }
}

void high_task(intptr_t unused){
    //Initialitzation of counter and char array
    int highcounter = 0;
    char lcdprinthigh[30];

    //Y axis position for HIGH PRIORITY TASK
    int lcdy = 25;

    while (1)
    {
        sprintf(lcdprinthigh,"high line: %d",highcounter);
        ev3_lcd_draw_string(lcdprinthigh,0,0);
        
        highcounter++;
        
        if(highcounter > 1000){
            highcounter = 0;
            ev3_lcd_draw_string("high line:          ",0,lcdy);
            tslp_tsk(5000);
        }
    }
}

void low_task(intptr_t unused){
    //same as other tasks
    int lowcounter = 0;
    char lcdprintlow[30];
    //Y axis postiton for low task.
    int lcdy = 50;

    while (1)
    {
        sprintf(lcdprintlow,"low  line: %d",lowcounter);
        ev3_lcd_draw_string(lcdprintlow,0,lcdy);
        lowcounter++;
        if(lowcounter > 10000){
            //Clear screen line after 10k for cleanliness
            ev3_lcd_draw_string("                                      ",0,50);
            lowcounter = 0;
        }
    }
}
