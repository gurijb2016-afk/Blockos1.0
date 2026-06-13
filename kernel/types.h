#ifndef TYPES_H
#define TYPES_H

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;
typedef signed char        int8_t;
typedef signed short       int16_t;
typedef signed int         int32_t;
typedef signed long long   int64_t;
typedef uint32_t           size_t;
typedef int32_t            ssize_t;
typedef int                bool;

#define true  1
#define false 0
#define NULL  ((void*)0)

/* Memory alloc */
void* malloc(size_t size);
void* realloc(void* ptr, size_t size);
void  free(void* ptr);

/* String */
void* memset(void* ptr, int value, size_t size);
void* memcpy(void* dst, const void* src, size_t size);
int   strcmp(const char* a, const char* b);
int   strncmp(const char* a, const char* b, size_t n);
char* strcpy(char* dst, const char* src);
char* strncpy(char* dst, const char* src, size_t n);
char* strdup(const char* s);
char* strtok(char* str, const char* delim);
size_t strlen(const char* s);
int   snprintf(char* buf, size_t size, const char* fmt, ...);

#endif
