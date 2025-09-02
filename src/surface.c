#include "surface.h"

int in_bounds(const Surface* s, int x, int y){
    return (unsigned)x < (unsigned)s->width && (unsigned)y < (unsigned)s->height;
}

void plot(Surface* s, int x, int y, Color32 c){
    if (!in_bounds(s,x,y)) return;
    s->pixels[y * (s->stride/4) + x] = c;
}
