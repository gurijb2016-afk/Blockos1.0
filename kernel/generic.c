#include "types.h"
#include "generic.h"

/* ============================================
   BlockOS Generic Kernel Utilities
   ============================================ */

/* Egyszerű memória allokátor */
static uint8_t heap[1024 * 1024 * 8]; // 8MB heap
static uint32_t heap_ptr = 0;

void* malloc(size_t size) {
    if (heap_ptr + size > sizeof(heap)) return NULL;
    void* ptr = &heap[heap_ptr];
    heap_ptr += size;
    /* Alignment */
    heap_ptr = (heap_ptr + 3) & ~3;
    return ptr;
}

void* realloc(void* ptr, size_t size) {
    if (!ptr) return malloc(size);
    void* new_ptr = malloc(size);
    if (!new_ptr) return NULL;
    memcpy(new_ptr, ptr, size);
    return new_ptr;
}

void free(void* ptr) {
    /* Egyszerű allokátornál nem szabadítunk fel */
    (void)ptr;
}

/* ============================================
   String függvények
   ============================================ */

void* memset(void* ptr, int value, size_t size) {
    uint8_t* p = (uint8_t*)ptr;
    for (size_t i = 0; i < size; i++) p[i] = (uint8_t)value;
    return ptr;
}

void* memcpy(void* dst, const void* src, size_t size) {
    uint8_t* d = (uint8_t*)dst;
    const uint8_t* s = (const uint8_t*)src;
    for (size_t i = 0; i < size; i++) d[i] = s[i];
    return dst;
}

int memcmp(const void* a, const void* b, size_t size) {
    const uint8_t* pa = (const uint8_t*)a;
    const uint8_t* pb = (const uint8_t*)b;
    for (size_t i = 0; i < size; i++) {
        if (pa[i] != pb[i]) return pa[i] - pb[i];
    }
    return 0;
}

size_t strlen(const char* s) {
    size_t len = 0;
    while (s[len]) len++;
    return len;
}

int strcmp(const char* a, const char* b) {
    while (*a && *b && *a == *b) { a++; b++; }
    return *(uint8_t*)a - *(uint8_t*)b;
}

int strncmp(const char* a, const char* b, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (a[i] != b[i]) return (uint8_t)a[i] - (uint8_t)b[i];
        if (!a[i]) return 0;
    }
    return 0;
}

char* strcpy(char* dst, const char* src) {
    char* d = dst;
    while ((*d++ = *src++));
    return dst;
}

char* strncpy(char* dst, const char* src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i]; i++) dst[i] = src[i];
    for (; i < n; i++) dst[i] = 0;
    return dst;
}

char* strdup(const char* s) {
    size_t len = strlen(s) + 1;
    char* copy = (char*)malloc(len);
    if (copy) memcpy(copy, s, len);
    return copy;
}

char* strcat(char* dst, const char* src) {
    char* d = dst + strlen(dst);
    while ((*d++ = *src++));
    return dst;
}

char* strncat(char* dst, const char* src, size_t n) {
    char* d = dst + strlen(dst);
    size_t i;
    for (i = 0; i < n && src[i]; i++) d[i] = src[i];
    d[i] = 0;
    return dst;
}

char* strchr(const char* s, int c) {
    while (*s) {
        if (*s == (char)c) return (char*)s;
        s++;
    }
    return NULL;
}

char* strrchr(const char* s, int c) {
    const char* last = NULL;
    while (*s) {
        if (*s == (char)c) last = s;
        s++;
    }
    return (char*)last;
}

/* strtok */
static char* strtok_ptr = NULL;
char* strtok(char* str, const char* delim) {
    if (str) strtok_ptr = str;
    if (!strtok_ptr) return NULL;

    /* Skip delimiters */
    while (*strtok_ptr && strchr(delim, *strtok_ptr)) strtok_ptr++;
    if (!*strtok_ptr) return NULL;

    char* token = strtok_ptr;
    while (*strtok_ptr && !strchr(delim, *strtok_ptr)) strtok_ptr++;
    if (*strtok_ptr) *strtok_ptr++ = 0;

    return token;
}

/* ============================================
   I/O port függvények
   ============================================ */

void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" :: "a"(value), "Nd"(port));
}

uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

void outw(uint16_t port, uint16_t value) {
    __asm__ volatile ("outw %0, %1" :: "a"(value), "Nd"(port));
}

uint16_t inw(uint16_t port) {
    uint16_t value;
    __asm__ volatile ("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

void outl(uint16_t port, uint32_t value) {
    __asm__ volatile ("outl %0, %1" :: "a"(value), "Nd"(port));
}

uint32_t inl(uint16_t port) {
    uint32_t value;
    __asm__ volatile ("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

void io_wait(void) {
    outb(0x80, 0);
}

/* ============================================
   snprintf implementáció
   ============================================ */

static void int_to_str(int64_t n, char* buf, int base) {
    if (n == 0) { buf[0] = '0'; buf[1] = 0; return; }
    char tmp[32]; int i = 0; bool neg = false;
    if (n < 0) { neg = true; n = -n; }
    while (n > 0) {
        int r = n % base;
        tmp[i++] = r < 10 ? '0' + r : 'a' + r - 10;
        n /= base;
    }
    if (neg) tmp[i++] = '-';
    int j = 0;
    while (i > 0) buf[j++] = tmp[--i];
    buf[j] = 0;
}

static void uint_to_str(uint64_t n, char* buf, int base) {
    if (n == 0) { buf[0] = '0'; buf[1] = 0; return; }
    char tmp[32]; int i = 0;
    while (n > 0) {
        int r = n % base;
        tmp[i++] = r < 10 ? '0' + r : 'a' + r - 10;
        n /= base;
    }
    int j = 0;
    while (i > 0) buf[j++] = tmp[--i];
    buf[j] = 0;
}

int snprintf(char* buf, size_t size, const char* fmt, ...) {
    /* Va_list implementáció */
    uint8_t* args = (uint8_t*)(&fmt) + sizeof(fmt);
    size_t pos = 0;
    char tmp[64];

    while (*fmt && pos < size - 1) {
        if (*fmt != '%') { buf[pos++] = *fmt++; continue; }
        fmt++;
        switch (*fmt) {
            case 'd': case 'i': {
                int32_t val = *(int32_t*)args; args += 4;
                int_to_str(val, tmp, 10);
                for (int i = 0; tmp[i] && pos < size-1; i++) buf[pos++] = tmp[i];
                break;
            }
            case 'u': {
                uint32_t val = *(uint32_t*)args; args += 4;
                uint_to_str(val, tmp, 10);
                for (int i = 0; tmp[i] && pos < size-1; i++) buf[pos++] = tmp[i];
                break;
            }
            case 'x': {
                uint32_t val = *(uint32_t*)args; args += 4;
                uint_to_str(val, tmp, 16);
                for (int i = 0; tmp[i] && pos < size-1; i++) buf[pos++] = tmp[i];
                break;
            }
            case 'l': {
                fmt++;
                if (*fmt == 'l') {
                    fmt++;
                    uint64_t val = *(uint64_t*)args; args += 8;
                    uint_to_str(val, tmp, 10);
                    for (int i = 0; tmp[i] && pos < size-1; i++) buf[pos++] = tmp[i];
                }
                break;
            }
            case 's': {
                char* s = *(char**)args; args += 4;
                if (!s) s = "(null)";
                while (*s && pos < size-1) buf[pos++] = *s++;
                break;
            }
            case 'c': {
                char c = *(char*)args; args += 4;
                buf[pos++] = c;
                break;
            }
            case '%': buf[pos++] = '%'; break;
        }
        fmt++;
    }
    buf[pos] = 0;
    return pos;
}

/* ============================================
   Kernel ticks
   ============================================ */
static uint64_t g_ticks = 0;

void kernel_tick(void) { g_ticks++; }
uint64_t kernel_get_ticks(void) { return g_ticks; }

/* ============================================
   Kernel panic
   ============================================ */
void kernel_panic(const char* msg) {
    /* VGA piros képernyő */
    uint16_t* vga = (uint16_t*)0xB8000;
    for (int i = 0; i < 80*25; i++) vga[i] = 0x4F20;

    /* Üzenet kiírása */
    const char* prefix = "BLOCKOS KERNEL PANIC: ";
    int i = 0;
    while (prefix[i]) { vga[i] = 0x4F00 | prefix[i]; i++; }
    int j = 0;
    while (msg[j]) { vga[i+j] = 0x4F00 | msg[j]; j++; }

    /* Megállás */
    __asm__ volatile ("cli; hlt");
    while(1);
}
