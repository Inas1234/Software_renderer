#include "draw.h"
#include <stdlib.h>
#include <math.h>

void clear(Surface* s, Color32 c){
    int pitch_px = s->stride / 4;
    for (int y=0; y<s->height; ++y){
        Color32* row = s->pixels + y * pitch_px;
        for (int x=0; x<s->width; ++x) row[x] = c;
    }
}

void rect_fill(Surface* s, int x0, int y0, int w, int h, Color32 c){
    int x1 = x0 + (w>0? w:0), y1 = y0 + (h>0? h:0);
    if (x0 < 0) x0 = 0; if (y0 < 0) y0 = 0;
    if (x1 > s->width)  x1 = s->width;
    if (y1 > s->height) y1 = s->height;
    int pitch_px = s->stride / 4;
    for (int y = y0; y < y1; ++y){
        Color32* row = s->pixels + y * pitch_px;
        for (int x = x0; x < x1; ++x) row[x] = c;
    }
}

void line(Surface* s, int x0,int y0,int x1,int y1, Color32 c){
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    for(;;){
        plot(s, x0, y0, c);
        int e2 = err << 1;
        if (e2 >= dy) { if (x0 == x1) break; err += dy; x0 += sx; }
        if (e2 <= dx) { if (y0 == y1) break; err += dx; y0 += sy; }
    }
}

void circle_fill(Surface* s, int cx,int cy,int r, Color32 c){
    int r2 = r*r;
    for (int y = cy - r; y <= cy + r; ++y)
        for (int x = cx - r; x <= cx + r; ++x)
            if ((x-cx)*(x-cx) + (y-cy)*(y-cy) <= r2) plot(s,x,y,c);
}


static inline void swap_int(int* a, int* b){ int t = *a; *a = *b; *b = t; }


static void tri_fill_flat_bottom(Surface* s,
                                 int x0,int y0, int x1,int y1, int x2,int y2,
                                 Color32 c)
{
    if (y1 <= y0) return;
    float invL = (x1 - x0) / (float)(y1 - y0);
    float invR = (x2 - x0) / (float)(y2 - y0); 
    float curL = (float)x0;
    float curR = (float)x0;

    int yStart = y0 < 0 ? 0 : y0;
    int yEnd   = y1;               
    if (yEnd > s->height) yEnd = s->height;

    curL += invL * (yStart - y0);
    curR += invR * (yStart - y0);

    int pitch_px = s->stride / 4;
    for (int y = yStart; y < yEnd; ++y) {
        int xL = (int)ceilf(fminf(curL, curR));
        int xR = (int)ceilf(fmaxf(curL, curR));
        if (xL < 0) xL = 0;
        if (xR > s->width) xR = s->width;
        Color32* row = s->pixels + y * pitch_px;
        for (int x = xL; x < xR; ++x) row[x] = c;

        curL += invL;
        curR += invR;
    }
}

static void tri_fill_flat_top(Surface* s,
                              int x0,int y0, int x1,int y1, int x2,int y2,
                              Color32 c)
{
    if (y2 <= y0) return;
    float invL = (x2 - x0) / (float)(y2 - y0);  
    float invR = (x2 - x1) / (float)(y2 - y1);
    float curL = (float)x2;
    float curR = (float)x2;

    int yStart = y0;                
    int yEnd   = y2;                
    if (yStart < 0) {
        curL += invL * (0 - yStart);
        curR += invR * (0 - yStart);
        yStart = 0;
    }
    if (yEnd > s->height) yEnd = s->height;

    int pitch_px = s->stride / 4;
    curL = (float)x0 + invL * (yStart - y0);
    curR = (float)x1 + invR * (yStart - y1);

    for (int y = yStart; y < yEnd; ++y) {
        int xL = (int)ceilf(fminf(curL, curR));
        int xR = (int)ceilf(fmaxf(curL, curR));
        if (xL < 0) xL = 0;
        if (xR > s->width) xR = s->width;
        Color32* row = s->pixels + y * pitch_px;
        for (int x = xL; x < xR; ++x) row[x] = c;

        curL += invL;
        curR += invR;
    }
}



void tri_fill(Surface* s, int x0,int y0,int x1,int y1,int x2,int y2, Color32 c)
{
    if (y0 > y1) { swap_int(&y0,&y1); swap_int(&x0,&x1); }
    if (y1 > y2) { swap_int(&y1,&y2); swap_int(&x1,&x2); }
    if (y0 > y1) { swap_int(&y0,&y1); swap_int(&x0,&x1); }

    if (y0 == y2) {
        int xL = x0, xM = x1, xR = x2;
        if (xL > xM) swap_int(&xL, &xM);
        if (xM > xR) swap_int(&xM, &xR);
        rect_fill(s, xL, y0, xR - xL, 1, c);
        return;
    }

    if (y1 == y2) {
        tri_fill_flat_bottom(s, x0,y0, x1,y1, x2,y2, c);
    } else if (y0 == y1) {
        tri_fill_flat_top(s, x0,y0, x1,y1, x2,y2, c);
    } else {
        float t = (float)(y1 - y0) / (float)(y2 - y0);
        float xf = x0 + t * (float)(x2 - x0);
        int   xi = (int)floorf(xf);  

        tri_fill_flat_bottom(s, x0,y0, x1,y1, xi,y1, c);
        tri_fill_flat_top   (s, x1,y1, xi,y1, x2,y2, c);
    }
}

void tri_wire(Surface* s, int x0,int y0,int x1,int y1,int x2,int y2, Color32 c)
{
    line(s, x0,y0, x1,y1, c);
    line(s, x1,y1, x2,y2, c);
    line(s, x2,y2, x0,y0, c);
}

void depth_clear(Surface* s, float zfar){
    if (!s->depth) return;
    int N = s->width * s->height;
    for (int i=0; i<N; ++i) s->depth[i] = zfar;
}

static inline int is_top_left(float x0,float y0, float x1,float y1){
    float dx = x1 - x0, dy = y1 - y0;
    return (dy < 0.0f) || (dy == 0.0f && dx > 0.0f);
}

static inline float edge_fn(float ax,float ay, float bx,float by, float px,float py){
    return (py - ay)*(bx - ax) - (px - ax)*(by - ay);
}

void tri_fill_z(
    Surface* s,
    float x0,float y0,float z0,
    float x1,float y1,float z1,
    float x2,float y2,float z2,
    Color32 color)
{
    int w = s->width, h = s->height;
    int pitch_px = s->stride / 4;

    float minx_f = floorf(fminf(x0, fminf(x1, x2)));
    float miny_f = floorf(fminf(y0, fminf(y1, y2)));
    float maxx_f = ceilf (fmaxf(x0, fmaxf(x1, x2)));
    float maxy_f = ceilf (fmaxf(y0, fmaxf(y1, y2)));
    int minx = (int)fmaxf(0.0f, minx_f);
    int miny = (int)fmaxf(0.0f, miny_f);
    int maxx = (int)fminf((float)w, maxx_f);
    int maxy = (int)fminf((float)h, maxy_f);
    if (minx >= maxx || miny >= maxy) return;

    float area = edge_fn(x0,y0, x1,y1, x2,y2);
    if (area == 0.0f) return; 

    float invArea = 1.0f / area;

    int tl0 = is_top_left(x1,y1,x2,y2);
    int tl1 = is_top_left(x2,y2,x0,y0);
    int tl2 = is_top_left(x0,y0,x1,y1);

    float b0 = tl0 ? 0.0f : 1e-6f;
    float b1 = tl1 ? 0.0f : 1e-6f;
    float b2 = tl2 ? 0.0f : 1e-6f;

    float px0 = (float)minx + 0.5f;
    float py0 = (float)miny + 0.5f;

    float A0x = x1 - x2, A0y = y1 - y2; 
    float A1x = x2 - x0, A1y = y2 - y0; 
    float A2x = x0 - x1, A2y = y0 - y1; 

    float E0_row = edge_fn(x1,y1, x2,y2, px0, py0);
    float E1_row = edge_fn(x2,y2, x0,y0, px0, py0);
    float E2_row = edge_fn(x0,y0, x1,y1, px0, py0);

    float dE0dx = -(A0y), dE0dy = (A0x);
    float dE1dx = -(A1y), dE1dy = (A1x);
    float dE2dx = -(A2y), dE2dy = (A2x);

    for (int y = miny; y < maxy; ++y) {
        float E0 = E0_row;
        float E1 = E1_row;
        float E2 = E2_row;

        Color32* rowC = s->pixels + y * pitch_px;
        float*   rowZ = s->depth ? (s->depth + y * w) : NULL;

        for (int x = minx; x < maxx; ++x) {
            if (E0 >= -b0 && E1 >= -b1 && E2 >= -b2) {
                float w0 = E0 * invArea;
                float w1 = E1 * invArea;
                float w2 = E2 * invArea;

                float z = w0*z0 + w1*z1 + w2*z2;

                int idxC = x;
                int idxZ = x;
                if (!rowZ || z < rowZ[idxZ]) {
                    if (rowZ) rowZ[idxZ] = z;
                    rowC[idxC] = color;
                }
            }
            E0 += dE0dx; E1 += dE1dx; E2 += dE2dx;
        }
        E0_row += dE0dy; E1_row += dE1dy; E2_row += dE2dy;
    }
}

void tri_fill_z_color(
    Surface* s,
    float x0,float y0,float z0, Color32 c0,
    float x1,float y1,float z1, Color32 c1,
    float x2,float y2,float z2, Color32 c2)
{
    int w = s->width, h = s->height;
    int pitch_px = s->stride / 4;

    float minx_f = floorf(fminf(x0, fminf(x1, x2)));
    float miny_f = floorf(fminf(y0, fminf(y1, y2)));
    float maxx_f = ceilf (fmaxf(x0, fmaxf(x1, x2)));
    float maxy_f = ceilf (fmaxf(y0, fmaxf(y1, y2)));
    int minx = (int)fmaxf(0.0f, minx_f);
    int miny = (int)fmaxf(0.0f, miny_f);
    int maxx = (int)fminf((float)w, maxx_f);
    int maxy = (int)fminf((float)h, maxy_f);
    if (minx >= maxx || miny >= maxy) return;

    float area = edge_fn(x0,y0, x1,y1, x2,y2);
    if (area == 0.0f) return;                 
    if (area < 0.0f) {
        float tx, ty, tz;
        Color32 tc;
        tx=x1; x1=x2; x2=tx;
        ty=y1; y1=y2; y2=ty;
        tz=z1; z1=z2; z2=tz;
        tc=c1; c1=c2; c2=tc;                  
        area = -area;
    }
    float invArea = 1.0f / area;

    int tl0 = is_top_left(x1,y1,x2,y2);
    int tl1 = is_top_left(x2,y2,x0,y0);
    int tl2 = is_top_left(x0,y0,x1,y1);
    float b0 = tl0 ? 0.0f : 1e-6f;
    float b1 = tl1 ? 0.0f : 1e-6f;
    float b2 = tl2 ? 0.0f : 1e-6f;

    float px0 = (float)minx + 0.5f;
    float py0 = (float)miny + 0.5f;

    float A0x = x1 - x2, A0y = y1 - y2;
    float A1x = x2 - x0, A1y = y2 - y0;
    float A2x = x0 - x1, A2y = y0 - y1;

    float E0_row = edge_fn(x1,y1, x2,y2, px0, py0);
    float E1_row = edge_fn(x2,y2, x0,y0, px0, py0);
    float E2_row = edge_fn(x0,y0, x1,y1, px0, py0);

    float dE0dx = -(A0y), dE0dy = (A0x);
    float dE1dx = -(A1y), dE1dy = (A1x);
    float dE2dx = -(A2y), dE2dy = (A2x);

    for (int y = miny; y < maxy; ++y) {
        float E0 = E0_row, E1 = E1_row, E2 = E2_row;

        Color32* rowC = s->pixels + y * pitch_px;
        float*   rowZ = s->depth ? (s->depth + y * w) : NULL;

        for (int x = minx; x < maxx; ++x) {
            if (E0 >= -b0 && E1 >= -b1 && E2 >= -b2) {
                float w0 = E0 * invArea;
                float w1 = E1 * invArea;
                float w2 = E2 * invArea;

                float z = w0*z0 + w1*z1 + w2*z2;

                int idx = x;
                if (!rowZ || z < rowZ[idx]) {
                    if (rowZ) rowZ[idx] = z;

                    float bf = w0*c0.b + w1*c1.b + w2*c2.b;
                    float gf = w0*c0.g + w1*c1.g + w2*c2.g;
                    float rf = w0*c0.r + w1*c1.r + w2*c2.r;
                    float af = w0*c0.a + w1*c1.a + w2*c2.a;

                    Color32 out = {
                        (uint8_t)fminf(fmaxf(bf, 0.f), 255.f),
                        (uint8_t)fminf(fmaxf(gf, 0.f), 255.f),
                        (uint8_t)fminf(fmaxf(rf, 0.f), 255.f),
                        (uint8_t)fminf(fmaxf(af, 0.f), 255.f)
                    };
                    rowC[idx] = out;
                }
            }
            E0 += dE0dx; E1 += dE1dx; E2 += dE2dx;
        }
        E0_row += dE0dy; E1_row += dE1dy; E2_row += dE2dy;
    }
}


void tri_fill_persp_z(
    Surface* s,
    float x0,float y0,float z0ow,float q0,
    float x1,float y1,float z1ow,float q1,
    float x2,float y2,float z2ow,float q2,
    Color32 color)
{
    int W = s->width, H = s->height, pitch_px = s->stride/4;

    float minx_f = floorf(fminf(x0, fminf(x1, x2)));
    float miny_f = floorf(fminf(y0, fminf(y1, y2)));
    float maxx_f = ceilf (fmaxf(x0, fmaxf(x1, x2)));
    float maxy_f = ceilf (fmaxf(y0, fmaxf(y1, y2)));
    int minx = (int)fmaxf(0.f, minx_f), miny = (int)fmaxf(0.f, miny_f);
    int maxx = (int)fminf((float)W, maxx_f), maxy = (int)fminf((float)H, maxy_f);
    if (minx >= maxx || miny >= maxy) return;

    float area = edge_fn(x0,y0, x1,y1, x2,y2);
    if (area == 0.0f) return;
    if (area < 0.0f){
        float tx,ty,tz,tq;
        tx=x1; x1=x2; x2=tx;  ty=y1; y1=y2; y2=ty;
        tz=z1ow; z1ow=z2ow; z2ow=tz; tq=q1; q1=q2; q2=tq;
        area = -area;
    }
    float invArea = 1.0f / area;

    float b0 = is_top_left(x1,y1,x2,y2) ? 0.0f : 1e-6f;
    float b1 = is_top_left(x2,y2,x0,y0) ? 0.0f : 1e-6f;
    float b2 = is_top_left(x0,y0,x1,y1) ? 0.0f : 1e-6f;

    float sx = (float)minx + 0.5f, sy = (float)miny + 0.5f;

    float E0_row = edge_fn(x1,y1, x2,y2, sx, sy);
    float E1_row = edge_fn(x2,y2, x0,y0, sx, sy);
    float E2_row = edge_fn(x0,y0, x1,y1, sx, sy);

    float dE0dx = (y1 - y2), dE0dy = (x2 - x1);
    float dE1dx = (y2 - y0), dE1dy = (x0 - x2);
    float dE2dx = (y0 - y1), dE2dy = (x1 - x0);

    for (int y = miny; y < maxy; ++y) {
        float E0 = E0_row, E1 = E1_row, E2 = E2_row;
        Color32* rowC = s->pixels + y * pitch_px;
        float*   rowZ = s->depth ? (s->depth + y * W) : NULL;

        for (int x = minx; x < maxx; ++x) {
            if (E0 >= -b0 && E1 >= -b1 && E2 >= -b2) {
                float w0 = E0 * invArea, w1 = E1 * invArea, w2 = E2 * invArea;

                float q = w0*q0 + w1*q1 + w2*q2;      
                if (q > 0.0f){
                    float z_ndc = (w0*z0ow + w1*z1ow + w2*z2ow) / q;
                    if (!rowZ || z_ndc < rowZ[x]) {
                        if (rowZ) rowZ[x] = z_ndc;
                        rowC[x] = color;
                    }
                }
            }
            E0 += dE0dx; E1 += dE1dx; E2 += dE2dx;
        }
        E0_row += dE0dy; E1_row += dE1dy; E2_row += dE2dy;
    }
}
