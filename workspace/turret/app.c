#include "ev3api.h"
#include "app.h"
//GANT? - period ms |                 
//PIXY              |                                
//CALC              |                                                
//FIRE              | 
//                  |
//       



//GANT? - period ms |      17          3  2       17         3  2                 
//PIXY              |*****************     *****************  
//CALC              |                 ***                   ***
//FIRE              |                    **                    **
//                  |       
// Problems: we cant fire other than in 20-22 second intervals.      



//Global variables

void main_task(intptr_t unused){
    //Variables
    static uint8_t NUM_BLOCKS = 1;
    block_signature_t signatures = SIGNATURE_1;
    pixycam2_block_t blocks[NUM_BLOCKS];
    pixycam2_block_response_t response[25];
    //response[0].blocks = blocks;
    
    //Debug timers
    SYSTIM mainStart;
    SYSTIM mainEnd;
    get_tim(&mainStart);
    //Initialize ev3 ports
    ev3_motor_config(EV3_PORT_1,LARGE_MOTOR);
    ev3_lcd_set_font(EV3_FONT_MEDIUM);
    ev3_sensor_config(EV3_PORT_4, PIXYCAM_2);

    get_tim(&mainEnd);
    char a[30];
    sprintf(a,"Main took %lu", mainEnd-mainStart);
    ev3_lcd_draw_string(a,0,0);

    act_tsk(fire_task);
    act_tsk(calc_task);
    act_tsk(pixy_task);
    slp_tsk();
}
void pixy_task(block_signature_t *signatures, pixycam2_block_response_t *response, uint8_t *NUM_BLOCKS){
    int8_t count = 0;
    while (1)
    {
        pixycam_2_get_blocks(EV3_PORT_4, &response[count], &signatures, NUM_BLOCKS);
        count++;
        tslp_tsk(5);  
    }

}
void calc_task(pixycam2_block_response_t *response, int8_t *count){
    int8_t fireTimer = 0;
    fireTimer = 100;
    for (int8_t i = 0; i < count; i++)
    {
        
    }
}
void fire_task(int8_t *fireTimer){
    //Timer from task started
    SYSTIM startTimer;
    //bool for fire test
    int8_t didItFire = 0;
    
    //Rotate motor to fire projectile
    if((fireTimer - startTimer)<= 0 && didItFire == 0){
        ev3_motor_rotate(EV3_PORT_1,350,100,0);
        didItFire = 1;
    }
    tslp_tsk(3);
}
