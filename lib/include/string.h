#ifndef STRING_H
#define STRING_H

#include "stddef.h"

char *strcpy(char *dest, const char *from);
char *strncpy(char *dest, const char *from, size_t n);
size_t strlen(const char *str);
void *memcpy(void *dest, const void *from, size_t n);
void *memmove(void *dest, const void *from, size_t n);
void *memset(void *dest, int ch, size_t count);

#endif // !STRING_J
