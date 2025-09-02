#include "color.h"
#include "math.h"
#include <stdlib.h>

uint8_t u8_clamp_int(int v){ return (uint8_t)(v < 0 ? 0 : v > 255 ? 255 : v); }

float srgb_to_linear1(float c){
    return (c <= 0.04045f) ? (c / 12.92f) : powf((c + 0.055f) / 1.055f, 2.4f);
}
float linear_to_srgb1(float c){
    return (c <= 0.0031308f) ? (12.92f * c) : (1.055f * powf(c, 1.0f/2.4f) - 0.055f);
}
uint8_t linear_to_u8(float c){
    int v = (int)(linear_to_srgb1(c) * 255.0f + 0.5f);
    return u8_clamp_int(v);
}


Color32 RGBAf_linear(float r, float g, float b, float a){
    uint8_t R = linear_to_u8(r);
    uint8_t G = linear_to_u8(g);
    uint8_t B = linear_to_u8(b);
    uint8_t A = (uint8_t)u8_clamp_int((int)(a * 255.0f + 0.5f));
    return RGBA8(R,G,B,A);
}

