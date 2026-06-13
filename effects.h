#ifndef EFFECTS_H
#define EFFECTS_H
#include "../kernel/types.h"

void fx_blur(uint8_t* fb, int x, int y, int w, int h, int fb_w, int radius);
void fx_shadow(uint8_t* fb, int x, int y, int w, int h, int fb_w, int radius, uint8_t alpha);
void fx_rounded_rect(uint8_t* fb, int x, int y, int w, int h, int fb_w, int r, uint32_t color);
void fx_gaussian_blur(uint8_t* fb, int x, int y, int w, int h, int fb_w);
void fx_frosted_glass(uint8_t* fb, int x, int y, int w, int h, int fb_w, float alpha);
#endif
