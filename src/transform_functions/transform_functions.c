#include "transform_functions.h"
#include <math.h>

#include <immintrin.h>

// inline void scale(float x[24], float y[24], float z[24], float s){
//     for(size_t i = 0; i < 24; i++){
//         x[i] *= s;
//         y[i] *= s;
//         z[i] *= s;
//     }
// }
// 
// static inline void rotate_y_axis(float x[24], float y[24], float z[24], float angle){
//     const float C = cosf(angle);
//     const float S = sinf(angle);
// 
//     for(size_t i = 0; i < 24; i++){
//         const float X = x[i];
//         const float Z = z[i];
// 
//         x[i] = X * C - Z * S;
//         z[i] = X * S + Z * C;
//     }
// }
// 
// static inline void rotate_x_axis(float x[24], float y[24], float z[24], float angle){
//     const float C = cosf(angle);
//     const float S = sinf(angle);
// 
//     for(size_t i = 0; i < 24; i++){
//         const float Y = y[i];
//         const float Z = z[i];
// 
//         y[i] = Y * C - Z * S;
//         z[i] = Y * S + Z * C;
//     }
// }
// 
// static inline void rotate_z_axis(float x[24], float y[24], float z[24], float angle){
//     const float C = cosf(angle);
//     const float S = sinf(angle);
// 
//     for(size_t i = 0; i < 24; i++){
//         const float X = x[i];
//         const float Y = y[i];
// 
//         x[i] = X * C - Y * S;
//         y[i] = X * S + Y * C;
//     }
// }
// 
// inline void rotate_yxz(float x[24], float y[24], float z[24], vec3 angle){
//     angle.x = angle.x * (M_PI / 180);
//     angle.y = angle.y * (M_PI / 180);
//     angle.z = angle.z * (M_PI / 180);
// 
//     rotate_y_axis(x, y, z, angle.x);
//     rotate_x_axis(x, y, z, angle.y);
//     rotate_z_axis(x, y, z, angle.z);
// }
// 
// inline void translate_vec3(float x[24], float y[24], float z[24], vec3 dv){
//     for(size_t i = 0; i < 24; i++){
//         x[i] += dv.x;
//         y[i] += dv.y;
//         z[i] += dv.z;
//     }
// }
// 
// inline void sub_vec3(float x[24], float y[24], float z[24], vec3 v1){
//     for(size_t i = 0; i < 24; i++){
//         x[i] -= v1.x;
//         y[i] -= v1.y;
//         z[i] -= v1.z;
//     }
// }

inline void scale(float x[24], float y[24], float z[24], float s){
    const __m256 S = _mm256_set1_ps(s);
    for(size_t i=0;i<24;i+=8){

        __m256 X   = _mm256_loadu_ps(x+i);
        __m256 Y   = _mm256_loadu_ps(y+i);
        __m256 Z   = _mm256_loadu_ps(z+i);

        _mm256_storeu_ps(x+i, _mm256_mul_ps(X, S));
        _mm256_storeu_ps(y+i, _mm256_mul_ps(Y, S));
        _mm256_storeu_ps(z+i, _mm256_mul_ps(Z, S));
    }
}

static inline void rotate_y_axis(float x[24], float y[24], float z[24], float angle){

    const __m256 C = _mm256_set1_ps(cosf(angle));
    const __m256 S = _mm256_set1_ps(sinf(angle));

    for(size_t i=0;i<24;i+=8){
        
        __m256 X   = _mm256_loadu_ps(x+i);
        __m256 Z   = _mm256_loadu_ps(z+i);

        _mm256_storeu_ps(x+i, _mm256_sub_ps(_mm256_mul_ps(X, C),_mm256_mul_ps(Z,S)));
        _mm256_storeu_ps(z+i, _mm256_add_ps(_mm256_mul_ps(X, S),_mm256_mul_ps(Z,C)));

    }

}

static inline void rotate_x_axis(float x[24], float y[24], float z[24], float angle){
    
    const __m256 C = _mm256_set1_ps(cosf(angle));
    const __m256 S = _mm256_set1_ps(sinf(angle));

    for(size_t i=0;i<24;i+=8){
        
        __m256 Y   = _mm256_loadu_ps(y+i);
        __m256 Z   = _mm256_loadu_ps(z+i);

        _mm256_storeu_ps(y+i, _mm256_sub_ps(_mm256_mul_ps(Y, C),_mm256_mul_ps(Z,S)));
        _mm256_storeu_ps(z+i, _mm256_add_ps(_mm256_mul_ps(Y, S),_mm256_mul_ps(Z,C)));

    }
    
}

static inline void rotate_z_axis(float x[24], float y[24], float z[24], float angle){
    
    const __m256 C = _mm256_set1_ps(cosf(angle));
    const __m256 S = _mm256_set1_ps(sinf(angle));

    for(size_t i=0;i<24;i+=8){
        
        __m256 X   = _mm256_loadu_ps(x+i);
        __m256 Y   = _mm256_loadu_ps(y+i);

        _mm256_storeu_ps(x+i, _mm256_sub_ps(_mm256_mul_ps(X, C),_mm256_mul_ps(Y,S)));
        _mm256_storeu_ps(y+i, _mm256_add_ps(_mm256_mul_ps(X, S),_mm256_mul_ps(Y,C)));

    }
}

inline void rotate_yxz(float x[24], float y[24], float z[24], vec3 angle){
    angle.x = angle.x * (M_PI/180);
    angle.y = angle.y * (M_PI/180);
    angle.z = angle.z * (M_PI/180);
    
    rotate_y_axis(x,y,z,angle.x);
    rotate_x_axis(x,y,z,angle.y);
    rotate_z_axis(x,y,z,angle.z);

}

inline void translate_vec3(float x[24], float y[24], float z[24], vec3 dv){
    
    const __m256 DX = _mm256_set1_ps(dv.x);
    const __m256 DY = _mm256_set1_ps(dv.y);
    const __m256 DZ = _mm256_set1_ps(dv.z);

    for(size_t i=0;i<24;i+=8){
        __m256 X   = _mm256_loadu_ps(x+i);
        __m256 Y   = _mm256_loadu_ps(y+i);
        __m256 Z   = _mm256_loadu_ps(z+i);
        
        _mm256_storeu_ps(x+i, _mm256_add_ps(X,DX));
        _mm256_storeu_ps(y+i, _mm256_add_ps(Y,DY));
        _mm256_storeu_ps(z+i, _mm256_add_ps(Z,DZ));
    }

}

inline void sub_vec3(float x[24], float y[24], float z[24], vec3 v1){
    const __m256 DX = _mm256_set1_ps(v1.x);
    const __m256 DY = _mm256_set1_ps(v1.y);
    const __m256 DZ = _mm256_set1_ps(v1.z);

    for(size_t i=0;i<24;i+=8){
        __m256 X   = _mm256_loadu_ps(x+i);
        __m256 Y   = _mm256_loadu_ps(y+i);
        __m256 Z   = _mm256_loadu_ps(z+i);
        
        _mm256_storeu_ps(x+i, _mm256_sub_ps(X,DX));
        _mm256_storeu_ps(y+i, _mm256_sub_ps(Y,DY));
        _mm256_storeu_ps(z+i, _mm256_sub_ps(Z,DZ));
    }
}

/*

vec3 triangle_normal(vec3 v0, vec3 v1, vec3 v2){
    vec3 A = {.x = v1.x - v0.x, .y = v1.y - v0.y, .z = v1.z - v0.z};
    vec3 B = {.x = v2.x - v0.x, .y = v2.y - v0.y, .z = v2.z - v0.z};
    
    vec3 ret = {
        .x = A.y * B.z - A.z * B.y,
        .y = A.z * B.x - A.x * B.z,
        .z = A.x * B.y - A.y * B.x
    };

    return ret;
}

vec3 normalize(vec3 v){
    float magnitude = sqrt(v.x*v.x + v.y*v.y + v.z*v.z);

    vec3 ret = {
        .x = v.x / magnitude,
        .y = v.y / magnitude,
        .z = v.z / magnitude
    };

    return ret;

}

float dot(vec3 a, vec3 b){
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

vec3 cross_vec3(vec3 v0, vec3 v1){
    return (vec3){
                  .x = v0.y*v1.z - v0.z*v1.y,
                  .y = v0.z*v1.x - v0.x*v1.z,
                  .z = v0.x+v1.y - v0.y*v1.x 
                 };

}

vec3 get_forward_incr(vec3 angles){
    return rotate_zxy((vec3){0,0,MOVEMENT_SPEED},angles);
}

vec3 get_right_incr(vec3 angles){
    return rotate_zxy((vec3){MOVEMENT_SPEED,0,0},angles);
}
*/
