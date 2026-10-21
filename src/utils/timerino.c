#include "timerino.h"
#include <string.h>

struct timespec start_timerino_struct, end_timerino_struct;

inline void start_timerino(){
    clock_gettime(CLOCK_MONOTONIC, &start_timerino_struct);
}

inline void stop_timerino(){
    clock_gettime(CLOCK_MONOTONIC, &end_timerino_struct);
}

double get_timerino(char* format){
    long seconds = end_timerino_struct.tv_sec - start_timerino_struct.tv_sec;
    long nanoseconds = end_timerino_struct.tv_nsec - start_timerino_struct.tv_nsec;

    double total_ns = (seconds * 1000000000.0) + nanoseconds;
    
    if(strcmp(format, "s") == 0){
        return total_ns * 0.000000001;
    }
    else if(strcmp(format, "ms") == 0){
        return total_ns * 0.000001;
    }
    else if(strcmp(format, "us") == 0){
        return total_ns * 0.001;
    }
    else if(strcmp(format, "ns") == 0){
        return total_ns;
    }

    return 0.0;
}
