#include "ev3api.h"
#include "app.h"
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include "syssvc/serial.h"
#include "pixy.h"
#include "time.h"

#define VIEW_SENSOR(func) do { \
	while(1) { \
		{func;} \
		if(ev3_button_is_pressed(BACK_BUTTON)) { \
			while(ev3_button_is_pressed(BACK_BUTTON)); \
			break; \
		} \
	} \
} while(0)

void main_task(intptr_t unused){

    ev3_lcd_set_font(EV3_FONT_MEDIUM);

    //ev3_lcd_draw_string("Before Scan", 0,0);

    pixycam2_get_blocks_response resp;
    pixycam2_get_blocks_request req;

    req.max_blocks = 255;
    req.packet_type = 32;
    req.payload_length = 2;
    req.sync = 0xc1ae;
    req.sigmap = 255;

    //pixycam_get_blocks(&req, &resp);
    
    FILE *file = ev3_serial_open_file(EV3_SERIAL_UART);

    fwrite(&req, sizeof(pixycam2_get_blocks_request), 1, file);

    long size = 0; 
    
    do{

        fseek(file, 0, SEEK_END);
        size = ftell(file);
        rewind(file);
        ev3_lcd_draw_string("checking file length...", 0, 0);
        tslp_tsk(1);

    }while(!size);
    

    char buff[10];
 
    sprintf(buff, "%ld", size);

    ev3_lcd_draw_string(buff, 0, 0);

    return;
}

/*void pixycam_get_blocks(pixycam2_get_blocks_request *request, unsigned char *response){

    FILE *file = ev3_serial_open_file(EV3_SERIAL_UART);

    char 

    fgets
    int size_before = file

    size_t wr = fwrite(request, sizeof(pixycam2_get_blocks_request), 1, file);
    
    if(wr == 0){
        ev3_lcd_draw_string("Written", 0,0);

    }

    tslp_tsk(100);

    size_t re = fread(response, sizeof(pixycam2_get_blocks_response), 1, file);

    if(re == NULL || re < 0){
        ev3_lcd_draw_string("Read failed       ", 0, 0);
    }

    tslp_tsk(10);


    fclose(file);
}*/

