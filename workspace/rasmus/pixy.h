
#pragma once

typedef struct{
    unsigned short sync;
    unsigned char packet_type;
    unsigned char payload_length;
    unsigned char sigmap;
    unsigned char max_blocks;
} pixycam2_get_blocks_request;

typedef struct{

    short sync;
    unsigned char packet_type;
    unsigned char length_of_payload;
    unsigned short checksum;
    unsigned short signature;
    unsigned short x_block_center;
    unsigned short y_block_center;
    unsigned short block_width;
    unsigned short block_height;
    short angle_of_color; //in degrees
    unsigned char block_tracking_index;
    unsigned char block_age;
} pixycam2_get_blocks_response;

void pixycam_get_blocks(pixycam2_get_blocks_request *request, unsigned char *response);

