#ifndef STRING_H
#define STRING_H

#include "stddef.h"

char *strcpy(char *dest, const char *from);
char *strncpy(char *dest, const char *from, size_t n);
size_t strlen(const char *str);
void *memcpy(void *dest, const void *from, size_t n);
void *memmove(void *dest, const void *from, size_t n);
/*void asciiConverter(int n, char str[]);
void reverse(char s[]);
void append(char s[], char n);
void backspace(char s[]);
int strcmp(char s1[], char s2[]);*/

#endif // !STRING_J
