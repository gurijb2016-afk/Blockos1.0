#include "../kernel/types.h"
#include "../kernel/generic.h"

/* VGA Driver - BlockOS */

#define VGA_WIDTH     320
#define VGA_HEIGHT    200
#define VGA_MEMORY    0xA0000
#define VGA_INDEX     0x3C4
#define VGA_DATA      0x3C5

uint8_t* g_framebuffer = (uint8_t*)VGA_MEMORY;

/* VGA Mode 13h beállítás */
void vga_init(void) {
    /* Mode 13h - 320x200x256 */
    __asm__ volatile (
        "mov $0x13, %ax\n"
        "int $0x10\n"
    );
    g_framebuffer = (uint8_t*)VGA_MEMORY;
}

/* Pixel rajzolás */
void vga_put_pixel(uint32_t x, uint32_t y, uint8_t color) {
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT) return;
    g_framebuffer[y * VGA_WIDTH + x] = color;
}

/* Képernyő törlés */
void vga_clear(uint8_t color) {
    memset(g_framebuffer, color, VGA_WIDTH * VGA_HEIGHT);
}

/* Téglalap rajzolás */
void vga_draw_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint8_t color) {
    for (uint32_t dy = 0; dy < h; dy++) {
        for (uint32_t dx = 0; dx < w; dx++) {
            vga_put_pixel(x + dx, y + dy, color);
        }
    }
}
