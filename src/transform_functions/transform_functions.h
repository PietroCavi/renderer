#pragma once

#include "../renderer_structs/renderer_structs.h"

void scale(float x[24], float y[24], float z[24], float s);

void rotate_yxz(float x[24], float y[24], float z[24], vec3 angle);

void translate_vec3(float x[24], float y[24], float z[24], vec3 dv);

void sub_vec3(float x[24], float y[24], float z[24], vec3 v1);
