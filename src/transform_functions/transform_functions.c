#include "transform_functions.h"
#include <math.h>

#include <immintrin.h>

inline void scale(float x[144], float y[144], float z[144], float s){
    const __m256 S = _mm256_set1_ps(s);
    for(size_t i=0;i<144;i+=8){

        __m256 X   = _mm256_loadu_ps(x+i);
        __m256 Y   = _mm256_loadu_ps(y+i);
        __m256 Z   = _mm256_loadu_ps(z+i);

        _mm256_storeu_ps(x+i, _mm256_mul_ps(X, S));
        _mm256_storeu_ps(y+i, _mm256_mul_ps(Y, S));
        _mm256_storeu_ps(z+i, _mm256_mul_ps(Z, S));
    }
}

static inline void rotate_y_axis(float x[144], float y[144], float z[144], float angle){

    const __m256 C = _mm256_set1_ps(cosf(angle));
    const __m256 S = _mm256_set1_ps(sinf(angle));

    for(size_t i=0;i<144;i+=8){
        
        __m256 X   = _mm256_loadu_ps(x+i);
        __m256 Z   = _mm256_loadu_ps(z+i);

        _mm256_storeu_ps(x+i, _mm256_sub_ps(_mm256_mul_ps(X, C),_mm256_mul_ps(Z,S)));
        _mm256_storeu_ps(z+i, _mm256_add_ps(_mm256_mul_ps(X, S),_mm256_mul_ps(Z,C)));

    }

}

static inline void rotate_x_axis(float x[144], float y[144], float z[144], float angle){
    
    const __m256 C = _mm256_set1_ps(cosf(angle));
    const __m256 S = _mm256_set1_ps(sinf(angle));

    for(size_t i=0;i<144;i+=8){
        
        __m256 Y   = _mm256_loadu_ps(y+i);
        __m256 Z   = _mm256_loadu_ps(z+i);

        _mm256_storeu_ps(y+i, _mm256_sub_ps(_mm256_mul_ps(Y, C),_mm256_mul_ps(Z,S)));
        _mm256_storeu_ps(z+i, _mm256_add_ps(_mm256_mul_ps(Y, S),_mm256_mul_ps(Z,C)));

    }
    
}

static inline void rotate_z_axis(float x[144], float y[144], float z[144], float angle){
    
    const __m256 C = _mm256_set1_ps(cosf(angle));
    const __m256 S = _mm256_set1_ps(sinf(angle));

    for(size_t i=0;i<144;i+=8){
        
        __m256 X   = _mm256_loadu_ps(x+i);
        __m256 Y   = _mm256_loadu_ps(y+i);

        _mm256_storeu_ps(x+i, _mm256_sub_ps(_mm256_mul_ps(X, C),_mm256_mul_ps(Y,S)));
        _mm256_storeu_ps(y+i, _mm256_add_ps(_mm256_mul_ps(X, S),_mm256_mul_ps(Y,C)));

    }
}

inline void rotate_yxz(float x[144], float y[144], float z[144], vec3 angle){
    angle.x = angle.x * (M_PI/180);
    angle.y = angle.y * (M_PI/180);
    angle.z = angle.z * (M_PI/180);
    
    rotate_y_axis(x,y,z,angle.x);
    rotate_x_axis(x,y,z,angle.y);
    rotate_z_axis(x,y,z,angle.z);

}

inline void translate_vec3(float x[144], float y[144], float z[144], vec3 dv){
    
    const __m256 DX = _mm256_set1_ps(dv.x);
    const __m256 DY = _mm256_set1_ps(dv.y);
    const __m256 DZ = _mm256_set1_ps(dv.z);

    for(size_t i=0;i<144;i+=8){
        __m256 X   = _mm256_loadu_ps(x+i);
        __m256 Y   = _mm256_loadu_ps(y+i);
        __m256 Z   = _mm256_loadu_ps(z+i);
        
        _mm256_storeu_ps(x+i, _mm256_add_ps(X,DX));
        _mm256_storeu_ps(y+i, _mm256_add_ps(Y,DY));
        _mm256_storeu_ps(z+i, _mm256_add_ps(Z,DZ));
    }

}

inline void sub_vec3(float x[144], float y[144], float z[144], vec3 v1){
    const __m256 DX = _mm256_set1_ps(v1.x);
    const __m256 DY = _mm256_set1_ps(v1.y);
    const __m256 DZ = _mm256_set1_ps(v1.z);

    for(size_t i=0;i<144;i+=8){
        __m256 X   = _mm256_loadu_ps(x+i);
        __m256 Y   = _mm256_loadu_ps(y+i);
        __m256 Z   = _mm256_loadu_ps(z+i);
        
        _mm256_storeu_ps(x+i, _mm256_sub_ps(X,DX));
        _mm256_storeu_ps(y+i, _mm256_sub_ps(Y,DY));
        _mm256_storeu_ps(z+i, _mm256_sub_ps(Z,DZ));
    }
}

inline vec4 qmul(vec4 a, vec4 b){
    return (vec4){
        a.w*b.x + a.x*b.w + a.y*b.z - a.z*b.y,
        a.w*b.y - a.x*b.z + a.y*b.w + a.z*b.x,
        a.w*b.z + a.x*b.y - a.y*b.x + a.z*b.w,
        a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z };
}

inline void quat_rotate(float* restrict x, float* restrict y, float* restrict z, const vec4 q){
    const __m256 UX = _mm256_set1_ps(q.x);
    const __m256 UY = _mm256_set1_ps(q.y);
    const __m256 UZ = _mm256_set1_ps(q.z);
    const __m256 W  = _mm256_set1_ps(q.w);
    const __m256 U2X = _mm256_set1_ps(2.0f*q.x);
    const __m256 U2Y = _mm256_set1_ps(2.0f*q.y);
    const __m256 U2Z = _mm256_set1_ps(2.0f*q.z);

    for(size_t i = 0; i < 144; i += 8){
        const __m256 VX = _mm256_loadu_ps(x + i);
        const __m256 VY = _mm256_loadu_ps(y + i);
        const __m256 VZ = _mm256_loadu_ps(z + i);

        const __m256 TX = _mm256_fmsub_ps(U2Y, VZ, _mm256_mul_ps(U2Z, VY));
        const __m256 TY = _mm256_fmsub_ps(U2Z, VX, _mm256_mul_ps(U2X, VZ));
        const __m256 TZ = _mm256_fmsub_ps(U2X, VY, _mm256_mul_ps(U2Y, VX));

        __m256 RX = _mm256_fmadd_ps (W,  TX, VX);
        __m256 RY = _mm256_fmadd_ps (W,  TY, VY);
        __m256 RZ = _mm256_fmadd_ps (W,  TZ, VZ);

        RX = _mm256_fmadd_ps (UY, TZ, RX);
        RY = _mm256_fmadd_ps (UZ, TX, RY);
        RZ = _mm256_fmadd_ps (UX, TY, RZ);

        RX = _mm256_fnmadd_ps(UZ, TY, RX);
        RY = _mm256_fnmadd_ps(UX, TZ, RY);
        RZ = _mm256_fnmadd_ps(UY, TX, RZ);

        _mm256_storeu_ps(x + i, RX);
        _mm256_storeu_ps(y + i, RY);
        _mm256_storeu_ps(z + i, RZ);
    }
}

void triangle_normal(float x[144], float y[144], float z[144], float x_normal[144/3], float y_normal[144/3], float z_normal[144/3]){
    const __m256i stride = _mm256_setr_epi32(0, 3, 6, 9, 12, 15, 18, 21);

    for(size_t t = 0; t < 48; t += 8){
        const size_t base = 3 * t;

        const __m256 x0 = _mm256_i32gather_ps(x + base,     stride, 4);
        const __m256 x1 = _mm256_i32gather_ps(x + base + 1, stride, 4);
        const __m256 x2 = _mm256_i32gather_ps(x + base + 2, stride, 4);
        const __m256 y0 = _mm256_i32gather_ps(y + base,     stride, 4);
        const __m256 y1 = _mm256_i32gather_ps(y + base + 1, stride, 4);
        const __m256 y2 = _mm256_i32gather_ps(y + base + 2, stride, 4);
        const __m256 z0 = _mm256_i32gather_ps(z + base,     stride, 4);
        const __m256 z1 = _mm256_i32gather_ps(z + base + 1, stride, 4);
        const __m256 z2 = _mm256_i32gather_ps(z + base + 2, stride, 4);

        const __m256 ax = _mm256_sub_ps(x1, x0), ay = _mm256_sub_ps(y1, y0), az = _mm256_sub_ps(z1, z0);
        const __m256 bx = _mm256_sub_ps(x2, x0), by = _mm256_sub_ps(y2, y0), bz = _mm256_sub_ps(z2, z0);

        _mm256_storeu_ps(x_normal + t, _mm256_fmsub_ps(ay, bz, _mm256_mul_ps(az, by)));
        _mm256_storeu_ps(y_normal + t, _mm256_fmsub_ps(az, bx, _mm256_mul_ps(ax, bz)));
        _mm256_storeu_ps(z_normal + t, _mm256_fmsub_ps(ax, by, _mm256_mul_ps(ay, bx)));
    }}

void triangle_dot(const float x[144], const float y[144], const float z[144],
                      const float x_normal[48], const float y_normal[48], const float z_normal[48],
                          float dot[48]){

    const __m256i stride = _mm256_setr_epi32(0, 3, 6, 9, 12, 15, 18, 21);

    for(size_t t = 0; t < 48; t += 8){
        const size_t base = 3 * t;

        const __m256 vx = _mm256_i32gather_ps(x + base, stride, 4);
        const __m256 vy = _mm256_i32gather_ps(y + base, stride, 4);
        const __m256 vz = _mm256_i32gather_ps(z + base, stride, 4);

        const __m256 nx = _mm256_loadu_ps(x_normal + t);
        const __m256 ny = _mm256_loadu_ps(y_normal + t);
        const __m256 nz = _mm256_loadu_ps(z_normal + t);

        __m256 d = _mm256_mul_ps(nx, vx);
        d = _mm256_fmadd_ps(ny, vy, d);
        d = _mm256_fmadd_ps(nz, vz, d);

        _mm256_storeu_ps(dot + t, d);
    }
}

