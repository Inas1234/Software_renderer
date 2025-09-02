#pragma once
#include <math.h>

typedef struct { float x,y,z; } Vec3;
typedef struct { float x,y,z,w; } Vec4;
typedef struct { float m[16]; } Mat4; 

static inline Vec4 vec4(float x,float y,float z,float w){ Vec4 v={x,y,z,w}; return v; }
static inline Vec3 vec3(float x,float y,float z){ Vec3 v={x,y,z}; return v; }

static inline Mat4 mat4_identity(void){
    Mat4 r = { .m = {1,0,0,0,  0,1,0,0,  0,0,1,0,  0,0,0,1} };
    return r;
}

static inline Mat4 mat4_mul(Mat4 a, Mat4 b){
    Mat4 r; 
    for(int c=0;c<4;++c) for(int r0=0;r0<4;++r0){
        r.m[c*4+r0] = a.m[0*4+r0]*b.m[c*4+0] + a.m[1*4+r0]*b.m[c*4+1] +
                      a.m[2*4+r0]*b.m[c*4+2] + a.m[3*4+r0]*b.m[c*4+3];
    }
    return r;
}

static inline Vec4 mat4_mul_v4(Mat4 m, Vec4 v){
    Vec4 r;
    r.x = m.m[0]*v.x + m.m[4]*v.y + m.m[8]*v.z  + m.m[12]*v.w;
    r.y = m.m[1]*v.x + m.m[5]*v.y + m.m[9]*v.z  + m.m[13]*v.w;
    r.z = m.m[2]*v.x + m.m[6]*v.y + m.m[10]*v.z + m.m[14]*v.w;
    r.w = m.m[3]*v.x + m.m[7]*v.y + m.m[11]*v.z + m.m[15]*v.w;
    return r;
}

static inline Mat4 mat4_translation(float tx,float ty,float tz){
    Mat4 r = mat4_identity();
    r.m[12]=tx; r.m[13]=ty; r.m[14]=tz;
    return r;
}

static inline Mat4 mat4_rotation_y(float a){
    float c = cosf(a), s = sinf(a);
    Mat4 r = { .m = { c,0,-s,0,  0,1,0,0,  s,0,c,0,  0,0,0,1 } };
    return r;
}

static inline Mat4 mat4_perspective(float fovy_rad, float aspect, float znear, float zfar){
    float f = 1.0f / tanf(fovy_rad * 0.5f);
    Mat4 r = {0};
    r.m[0] = f / aspect;
    r.m[5] = f;
    r.m[10] = (zfar + znear) / (znear - zfar);
    r.m[11] = -1.0f;
    r.m[14] = (2.0f * zfar * znear) / (znear - zfar);
    return r;
}


static inline Vec3 v3_sub(Vec3 a, Vec3 b){ Vec3 r={a.x-b.x, a.y-b.y, a.z-b.z}; return r; }
static inline float v3_dot(Vec3 a, Vec3 b){ return a.x*b.x + a.y*b.y + a.z*b.z; }
static inline Vec3 v3_cross(Vec3 a, Vec3 b){
    Vec3 r={ a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x }; return r;
}
static inline Vec3 v3_norm(Vec3 v){
    float len = sqrtf(v3_dot(v,v)); 
    if (len > 0.0f){ float k = 1.0f/len; Vec3 r={v.x*k,v.y*k,v.z*k}; return r; }
    return v;
}
static inline Vec3 mat3_mul_v3(Mat4 m, Vec3 v){
    Vec3 r;
    r.x = m.m[0]*v.x + m.m[4]*v.y + m.m[8] *v.z;
    r.y = m.m[1]*v.x + m.m[5]*v.y + m.m[9] *v.z;
    r.z = m.m[2]*v.x + m.m[6]*v.y + m.m[10]*v.z;
    return r;
}
