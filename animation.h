#ifndef ANIMATION_H
#define ANIMATION_H
#include "../kernel/types.h"

#define ANIM_MAX        64
#define ANIM_EASE_LINEAR    0
#define ANIM_EASE_IN        1
#define ANIM_EASE_OUT       2
#define ANIM_EASE_IN_OUT    3
#define ANIM_EASE_BOUNCE    4

typedef struct {
    bool     active;
    float    start, end, current;
    uint32_t duration_ms;
    uint32_t elapsed_ms;
    uint8_t  easing;
    void (*on_update)(float value, void* userdata);
    void (*on_complete)(void* userdata);
    void*    userdata;
} anim_t;

void  anim_init(void);
int   anim_start(float from, float to, uint32_t ms, uint8_t easing,
                 void (*on_update)(float, void*),
                 void (*on_complete)(void*), void* userdata);
void  anim_stop(int id);
void  anim_tick(uint32_t delta_ms);
float anim_ease(float t, uint8_t easing);

/* Előre gyártott animációk */
void anim_window_open(int x, int y, int w, int h);
void anim_window_close(int x, int y, int w, int h);
void anim_fade_in(float* opacity);
void anim_fade_out(float* opacity);
void anim_slide_in(int* pos, int target);
#endif
