#pragma once
#include "surface.h"


void clear(Surface* s, Color32 c);
void rect_fill(Surface* s, int x0, int y0, int w, int h, Color32 c);
void line(Surface* s, int x0,int y0,int x1,int y1, Color32 c);
void circle_fill(Surface* s, int cx,int cy,int r, Color32 c);
void tri_fill(Surface* s, int x0,int y0,int x1,int y1,int x2,int y2, Color32 c);
void tri_wire(Surface* s, int x0,int y0,int x1,int y1,int x2,int y2, Color32 c);
void depth_clear(Surface* s, float zfar); 
void tri_fill_z(
    Surface* s,
    float x0,float y0,float z0,
    float x1,float y1,float z1,
    float x2,float y2,float z2,
    Color32 color);
void tri_fill_z_color(
    Surface* s,
    float x0,float y0,float z0, Color32 c0,
    float x1,float y1,float z1, Color32 c1,
    float x2,float y2,float z2, Color32 c2);
    
void tri_fill_persp_z(
    Surface* s,
    float x0,float y0,float z0_over_w,float q0,
    float x1,float y1,float z1_over_w,float q1,
    float x2,float y2,float z2_over_w,float q2,
    Color32 color);
