#include "animation.h"
#include "../kernel/generic.h"
#include <math.h>

static anim_t g_anims[ANIM_MAX];

void anim_init(void) { memset(g_anims, 0, sizeof(g_anims)); }

float anim_ease(float t, uint8_t easing) {
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    switch (easing) {
        case ANIM_EASE_LINEAR:  return t;
        case ANIM_EASE_IN:      return t * t;
        case ANIM_EASE_OUT:     return 1.0f - (1.0f-t)*(1.0f-t);
        case ANIM_EASE_IN_OUT:  return t < 0.5f ? 2*t*t : 1-2*(1-t)*(1-t);
        case ANIM_EASE_BOUNCE: {
            float d = 1.0f;
            if (t < 1/2.75f)      return d * 7.5625f * t * t;
            else if (t < 2/2.75f) { t -= 1.5f/2.75f;   return d*(7.5625f*t*t + 0.75f); }
            else if (t < 2.5/2.75f){ t -= 2.25f/2.75f; return d*(7.5625f*t*t + 0.9375f); }
            else                   { t -= 2.625f/2.75f; return d*(7.5625f*t*t + 0.984375f); }
        }
        default: return t;
    }
}

int anim_start(float from, float to, uint32_t ms, uint8_t easing,
               void (*on_update)(float, void*),
               void (*on_complete)(void*), void* userdata) {
    for (int i = 0; i < ANIM_MAX; i++) {
        if (!g_anims[i].active) {
            g_anims[i].active      = true;
            g_anims[i].start       = from;
            g_anims[i].end         = to;
            g_anims[i].current     = from;
            g_anims[i].duration_ms = ms;
            g_anims[i].elapsed_ms  = 0;
            g_anims[i].easing      = easing;
            g_anims[i].on_update   = on_update;
            g_anims[i].on_complete = on_complete;
            g_anims[i].userdata    = userdata;
            return i;
        }
    }
    return -1;
}

void anim_stop(int id) {
    if (id >= 0 && id < ANIM_MAX) g_anims[id].active = false;
}

void anim_tick(uint32_t delta_ms) {
    for (int i = 0; i < ANIM_MAX; i++) {
        if (!g_anims[i].active) continue;
        g_anims[i].elapsed_ms += delta_ms;
        float t = (float)g_anims[i].elapsed_ms / (float)g_anims[i].duration_ms;
        if (t >= 1.0f) { t = 1.0f; g_anims[i].active = false; }
        float et = anim_ease(t, g_anims[i].easing);
        g_anims[i].current = g_anims[i].start + (g_anims[i].end - g_anims[i].start) * et;
        if (g_anims[i].on_update) g_anims[i].on_update(g_anims[i].current, g_anims[i].userdata);
        if (!g_anims[i].active && g_anims[i].on_complete)
            g_anims[i].on_complete(g_anims[i].userdata);
    }
}

void anim_fade_in(float* opacity)  { anim_start(0.0f, 1.0f, 200, ANIM_EASE_OUT, NULL, NULL, opacity); }
void anim_fade_out(float* opacity) { anim_start(1.0f, 0.0f, 200, ANIM_EASE_IN,  NULL, NULL, opacity); }
void anim_slide_in(int* pos, int target) { anim_start((float)*pos, (float)target, 250, ANIM_EASE_OUT, NULL, NULL, pos); }
void anim_window_open(int x, int y, int w, int h)  { (void)x;(void)y;(void)w;(void)h; }
void anim_window_close(int x, int y, int w, int h) { (void)x;(void)y;(void)w;(void)h; }
