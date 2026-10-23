#pragma once

#include <linux/input.h>

int start_polly();
int stop_polly();

int is_down(int key);
