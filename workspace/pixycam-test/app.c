#include "ev3api.h"
#include "app.h"

#include <t_stddef.h>
#include <t_syslog.h>
#include <string.h>

void main_task(intptr_t unused) {

    ev3_lcd_set_font(EV3_FONT_MEDIUM);


    ev3_sensor_config(EV3_PORT_1, PIXYCAM_2);

    pixycam_2_block blocks[2];
    pixycam_2_block_response response;
    response.blocks = blocks;
    char block_1_coords[20];
    char block_2_coords[20];
    char payload_buf[20];
    int counter = 0;

    while(1){
        

        pixycam_2_get_blocks(EV3_PORT_1, &response, 3, 2);

        unsigned int val = response.header.payload_length == 0 ? 0 : (response.header.payload_length / 14);

        //sprintf(&payload_buf, "%u    ", val);

        if(val > 0){
            sprintf(&block_1_coords, "co: %u,%u s: %d     ", response.blocks[0].x_center, response.blocks[0].y_center, blocks[0].signature);
            ev3_lcd_draw_string(block_1_coords, 0, 0);
        }

        if(val > 1){
            sprintf(&block_2_coords, "co: %u %u s: %d     ", blocks[1].x_center, blocks[1].y_center, blocks[1].signature);
            ev3_lcd_draw_string(block_2_coords, 0, 16);
        }

        /*sprintf(&age_buf, "ind: %u age: %u   ", block.tracking_index, block.age);
        sprintf(&coord_buf, "Coord: %u,%u    ", block.x_center, block.y_center);
        sprintf(&h_buf, "height: %u    ", block.height);
        sprintf(&w_buf, "Width: %u    ", block.width);

        ev3_lcd_draw_string(age_buf, 0, 0);
        ev3_lcd_draw_string(coord_buf, 0, 14);
        ev3_lcd_draw_string(h_buf, 0, 28);
        ev3_lcd_draw_string(w_buf, 0, 42);
     */  
    }

    //pixycam_2_uart_test();

    ev3_lcd_draw_string("Done         ", 0, EV3_LCD_HEIGHT/2);

}

typedef struct{

    pixycam2_header header;
	uint8_t signature;
	uint8_t blocks;

} pixy_req;

void pixycam_2_uart_test(){

    FILE *file;
    file = ev3_serial_open_file(EV3_SERIAL_UART);

    pixycam_2_block blocks[3];
    pixycam_2_block_response response;
    
    response.blocks = blocks;

    pixy_req req;  
    req.header.sync = 0xc1ae;
	req.header.packet_type = 32;
	req.header.payload_length = 2;
	req.signature = 255;
	req.blocks = 3;

    char block_1_coords[20];
    char block_2_coords[20];
    unsigned long s = sizeof(pixycam2_header) + sizeof(short) + (sizeof(pixycam_2_block) * 3);
    uint8_t buffer[s];

    while(1){
        ev3_lcd_draw_string("1", 0, 0);
        fwrite(&req, sizeof(uint8_t), sizeof(pixy_req), file);
        fseek(file, 0, SEEK_SET);
        tslp_tsk(20);
        ev3_lcd_draw_string("2", 0, 0);
        fread(&buffer, sizeof(uint8_t), s, file);
        ev3_lcd_draw_string("3", 0, 0);
        unsigned int val = response.header.payload_length == 0 ? 0 : (response.header.payload_length / 14);
            //sprintf(&payload_buf, "%u    ", val);

        if(val > 0){
            sprintf(&block_1_coords, "co: %u,%u s: %d     ", response.blocks[0].x_center, response.blocks[0].y_center, blocks[0].signature);
            ev3_lcd_draw_string(block_1_coords, 0, 16);
        }

        /*if(val > 1){
            sprintf(&block_2_coords, "co: %u %u s: %d     ", blocks[1].x_center, blocks[1].y_center, blocks[1].signature);
            ev3_lcd_draw_string(block_2_coords, 0, 32);
        }*/
    }
    

}

