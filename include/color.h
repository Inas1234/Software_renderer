#pragma once
#include <stdint.h>

typedef struct {
    uint8_t b, g, r, a;
} Color32;


uint8_t u8_clamp_int(int v);
float srgb_to_linear1(float c);
float linear_to_srgb1(float c);
uint8_t linear_to_u8(float c);
Color32 RGBAf_linear(float r, float g, float b, float a);
#define RGBA8(R,G,B,A) \
    ((Color32){ \
        (uint8_t)(( (B)*(A) + 127) / 255), \
        (uint8_t)(( (G)*(A) + 127) / 255), \
        (uint8_t)(( (R)*(A) + 127) / 255), \
        (uint8_t)(A) \
    })

#define RGB8(R,G,B) RGBA8((R),(G),(B),255)
