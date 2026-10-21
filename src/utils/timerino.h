#pragma once

#include <time.h>

extern struct timespec start_timerino_struct, end_timerino_struct;

void start_timerino();

void stop_timerino();

double get_timerino(char* format);
