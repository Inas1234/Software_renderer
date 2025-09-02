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

static inline uint8_t u8_sat_int(int v){ return (uint8_t)(v<0?0:v>255?255:v); }
static inline Color32 color_scale(Color32 c, float s){
    int b = (int)(c.b * s + 0.5f);
    int g = (int)(c.g * s + 0.5f);
    int r = (int)(c.r * s + 0.5f);
    Color32 o = { u8_sat_int(b), u8_sat_int(g), u8_sat_int(r), c.a };
    return o;
}
