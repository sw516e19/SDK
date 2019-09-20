#include "wait.h"

//TODO: Must take into account diff. in seconds between current and now
void wait(unsigned int milliseconds){
    struct timespec spec;
    
    unsigned int current = get_current_miliseconds(&spec);
    unsigned int now = get_current_miliseconds(&spec);

    while ((now - current) < milliseconds);
}


unsigned int get_current_miliseconds(struct timespec *spec){
    clock_gettime(1, spec);
    return (spec->tv_nsec * 1.0e6);
}
