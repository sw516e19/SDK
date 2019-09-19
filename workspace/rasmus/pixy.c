#include "pixy.h"
#include "ev3api.h"


void pixycam_get_blocks(pixycam2_get_blocks_request *request, pixycam2_get_blocks_response *response){

    
    FILE *file = ev3_serial_open_file(EV3_SERIAL_UART);

    fwrite(request, sizeof(pixycam2_get_blocks_request), 1, file);

    tslp_tsk(16);

    fread(response, sizeof(pixycam2_get_blocks_response), 1, file);

    fclose(*file);
}



