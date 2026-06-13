#include "libc.h"

int abs(int x) { return x < 0 ? -x : x; }
int min(int a, int b) { return a < b ? a : b; }
int max(int a, int b) { return a > b ? a : b; }

bool isdigit(char c) { return c >= '0' && c <= '9'; }
bool isalpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
bool isspace(char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }
bool isupper(char c) { return c >= 'A' && c <= 'Z'; }
bool islower(char c) { return c >= 'a' && c <= 'z'; }
char toupper(char c) { return islower(c) ? c - 32 : c; }
char tolower(char c) { return isupper(c) ? c + 32 : c; }

int atoi(const char* s) {
    int result = 0, sign = 1;
    if (*s == '-') { sign = -1; s++; }
    while (isdigit(*s)) result = result * 10 + (*s++ - '0');
    return result * sign;
}

char* itoa(int n, char* buf, int base) {
    char tmp[32]; int i = 0; bool neg = false;
    if (n == 0) { buf[0] = '0'; buf[1] = 0; return buf; }
    if (n < 0 && base == 10) { neg = true; n = -n; }
    while (n > 0) {
        int r = n % base;
        tmp[i++] = r < 10 ? '0' + r : 'a' + r - 10;
        n /= base;
    }
    if (neg) tmp[i++] = '-';
    int j = 0;
    while (i > 0) buf[j++] = tmp[--i];
    buf[j] = 0;
    return buf;
}

void* memmove(void* dst, const void* src, size_t n) {
    uint8_t* d = (uint8_t*)dst;
    const uint8_t* s = (const uint8_t*)src;
    if (d < s) { for (size_t i = 0; i < n; i++) d[i] = s[i]; }
    else { for (size_t i = n; i > 0; i--) d[i-1] = s[i-1]; }
    return dst;
}

int memcmp(const void* a, const void* b, size_t n) {
    const uint8_t* pa = (const uint8_t*)a;
    const uint8_t* pb = (const uint8_t*)b;
    for (size_t i = 0; i < n; i++) {
        if (pa[i] != pb[i]) return pa[i] - pb[i];
    }
    return 0;
}
