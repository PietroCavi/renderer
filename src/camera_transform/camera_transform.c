#include "camera_transform.h"

#include "../renderer/renderer.h"

#include <math.h>

static inline vec3 rotate_y_axis(vec3 v, float angle){
    
    float c = cosf(angle);
    float s = sinf(angle);

    return (vec3){
        .x = v.x*c-v.z*s,
        .y = v.y,
        .z = v.x*s+v.z*c
    };
}

static inline vec3 rotate_z_axis(vec3 v, float angle){
    float c = cosf(angle);
    float s = sinf(angle);

    return (vec3){
        .x = v.x*c-v.y*s,
        .y = v.x*s+v.y*c,
        .z = v.z
    };
}

static inline vec3 rotate_x_axis(vec3 v, float angle){
    float c = cosf(angle);
    float s = sinf(angle);
    return (vec3){
        v.x,
        v.y*c - v.z*s,
        v.y*s + v.z*c
    };
}

static inline vec3 rotate_zxy(vec3 v, vec3 angle){
    angle.x = angle.x * (M_PI/180);
    angle.y = angle.y * (M_PI/180);
    angle.z = angle.z * (M_PI/180);
    return rotate_y_axis(rotate_x_axis(rotate_z_axis(v,angle.z),angle.x),angle.y);

}

static inline vec3 rotate_yxz(vec3 v, vec3 angle){
    angle.x = angle.x * (M_PI/180);
    angle.y = angle.y * (M_PI/180);
    angle.z = angle.z * (M_PI/180);
    return rotate_z_axis(rotate_x_axis(rotate_y_axis(v,angle.y),angle.x),angle.z);

}
vec3 get_forward_incr(float speed){
    vec3 v = (vec3){0, 0, speed};
    v = rotate_x_axis(v, -camera.rotation.x);
    v = rotate_y_axis(v,  camera.rotation.y);
    return v;
}

vec3 get_right_incr(float speed){
    vec3 v = (vec3){speed, 0, 0};
    v = rotate_y_axis(v, camera.rotation.y);
    return v;
}
