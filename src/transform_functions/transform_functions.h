#pragma once

#include "../renderer_structs/renderer_structs.h"

void scale(float x[144], float y[144], float z[144], float s);

void rotate_yxz(float x[144], float y[144], float z[144], vec3 angle);

void translate_vec3(float x[144], float y[144], float z[144], vec3 dv);

void sub_vec3(float x[144], float y[144], float z[144], vec3 v1);

void quat_rotate(float x[144], float y[144], float z[144], const vec4 q);

vec4 qmul(vec4 a, vec4 b);

void triangle_normal(float x[144], float y[144], float z[144], float x_normal[144/3], float y_normal[144/3], float z_normal[144/3]);

void triangle_dot(const float x[144], const float y[144], const float z[144],
                  const float x_normal[48], const float y_normal[48], const float z_normal[48],
                  float dot[48]);

