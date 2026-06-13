#ifndef LIBC_H
#define LIBC_H

#include "../kernel/types.h"

/* Math */
int    abs(int x);
int    min(int a, int b);
int    max(int a, int b);

/* String */
int    atoi(const char* s);
char*  itoa(int n, char* buf, int base);

/* Char */
bool   isdigit(char c);
bool   isalpha(char c);
bool   isspace(char c);
bool   isupper(char c);
bool   islower(char c);
char   toupper(char c);
char   tolower(char c);

/* Memory */
void*  memmove(void* dst, const void* src, size_t n);
int    memcmp(const void* a, const void* b, size_t n);

#endif
