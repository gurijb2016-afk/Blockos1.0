#include "effects.h"
#include "../kernel/generic.h"

/* Box blur */
void fx_blur(uint8_t* fb, int x, int y, int w, int h, int fw, int radius) {
    if (!fb || radius < 1) return;
    for (int py = y; py < y+h; py++) {
        for (int px = x; px < x+w; px++) {
            int r=0,g=0,b=0,cnt=0;
            for (int dy=-radius;dy<=radius;dy++) {
                for (int dx=-radius;dx<=radius;dx++) {
                    int nx=px+dx, ny=py+dy;
                    if (nx<0||nx>=fw||ny<0) continue;
                    int idx=(ny*fw+nx)*3;
                    r+=fb[idx]; g+=fb[idx+1]; b+=fb[idx+2]; cnt++;
                }
            }
            if (cnt>0){
                int idx=(py*fw+px)*3;
                fb[idx]=r/cnt; fb[idx+1]=g/cnt; fb[idx+2]=b/cnt;
            }
        }
    }
}

/* Árnyék */
void fx_shadow(uint8_t* fb, int x, int y, int w, int h, int fw, int radius, uint8_t alpha) {
    if (!fb) return;
    int sx=x+6, sy=y+6;
    for (int py=sy; py<sy+h+radius; py++) {
        for (int px=sx; px<sx+w+radius; px++) {
            if (px<0||px>=fw||py<0) continue;
            int idx=(py*fw+px)*3;
            fb[idx]   = fb[idx]   * (255-alpha) / 255;
            fb[idx+1] = fb[idx+1] * (255-alpha) / 255;
            fb[idx+2] = fb[idx+2] * (255-alpha) / 255;
        }
    }
    fx_blur(fb, sx, sy, w+radius, h+radius, fw, radius/2);
}

/* Lekerekített téglalap */
void fx_rounded_rect(uint8_t* fb, int x, int y, int w, int h, int fw, int r, uint32_t color) {
    if (!fb) return;
    uint8_t cr=(color>>16)&0xFF, cg=(color>>8)&0xFF, cb=color&0xFF;
    for (int py=y; py<y+h; py++) {
        for (int px=x; px<x+w; px++) {
            /* Sarok ellenőrzés */
            int dx=0, dy=0;
            if (px<x+r && py<y+r)       {dx=px-(x+r);  dy=py-(y+r);}
            else if (px>x+w-r && py<y+r) {dx=px-(x+w-r);dy=py-(y+r);}
            else if (px<x+r && py>y+h-r) {dx=px-(x+r);  dy=py-(y+h-r);}
            else if (px>x+w-r && py>y+h-r){dx=px-(x+w-r);dy=py-(y+h-r);}
            if (dx!=0||dy!=0) if (dx*dx+dy*dy > r*r) continue;
            if (px<0||px>=fw||py<0) continue;
            int idx=(py*fw+px)*3;
            fb[idx]=cr; fb[idx+1]=cg; fb[idx+2]=cb;
        }
    }
}

/* Gaussian blur (közelítés) */
void fx_gaussian_blur(uint8_t* fb, int x, int y, int w, int h, int fw) {
    fx_blur(fb, x, y, w, h, fw, 2);
    fx_blur(fb, x, y, w, h, fw, 2);
}

/* Frosted glass effekt */
void fx_frosted_glass(uint8_t* fb, int x, int y, int w, int h, int fw, float alpha) {
    if (!fb) return;
    fx_gaussian_blur(fb, x, y, w, h, fw);
    /* Fehér overlay */
    for (int py=y; py<y+h; py++) {
        for (int px=x; px<x+w; px++) {
            if (px<0||px>=fw||py<0) continue;
            int idx=(py*fw+px)*3;
            fb[idx]   = (uint8_t)(fb[idx]   + (255-fb[idx])   * alpha * 0.3f);
            fb[idx+1] = (uint8_t)(fb[idx+1] + (255-fb[idx+1]) * alpha * 0.3f);
            fb[idx+2] = (uint8_t)(fb[idx+2] + (255-fb[idx+2]) * alpha * 0.3f);
        }
    }
}
