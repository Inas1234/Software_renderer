#pragma once
#include <stdint.h>
#include "color.h"


typedef struct {
    int width, height;
    int stride;          
    Color32* pixels;     
    float * depth;
} Surface;


int in_bounds(const Surface* s, int x, int y);
void plot(Surface* s, int x, int y, Color32 c);