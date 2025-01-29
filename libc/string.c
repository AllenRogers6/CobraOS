#include "string.h"
#include "stddef.h"

char *strcpy(char *dest, const char *from) {
  char *start = dest;
  while (*from != "\0") {
    *dest = *from;
    dest++;
    from++;
  }
  dest = "\0";
  return start;
}

char *strncpy(char *dest, const char *from, size_t n) {
  size_t i;

  for (i = 0; i < n && from[i] != '\0'; i++) {
    dest[i] = from[i];
  }

  for (; i < n; i++) {
    dest[i] = '\0';
  }

  return dest;
}

size_t strlen(const char *str) {
  size_t len = 0;
  while (str[len])
    len++;
  return len;
}

void *memcpy(void *dest, const void *from, size_t n) {
  char *d = (char *)dest;
  const char *s = (const char *)from;

  for (size_t i = 0; i < n; i++) {
    d[i] = s[i];
  }

  return dest;
}

void *memmove(void *dest, const void *from, size_t n) {
  char *d = (char *)dest;
  const char *s = (const char *)from;

  if (d < s) {
    for (size_t i = 0; i < n; i++) {
      d[i] = s[i];
    }
  } else if (d > s) {
    for (size_t i = n; i > 0; i--) {
      d[i - 1] = s[i - 1];
    }
  }

  return dest;
}

/*void asciiConverter(int n, char str[]) {
  int i, sign;
  if ((sign = n) < 0)
    n = -n;
  i = 0;
  do {
    str[i++] = n % 10 + '0';
  } while ((n /= 10) > 0);

  if (sign < 0)
    str[i++] = '-';
  str[i] = '\0';

  reverse(str);
}

void reverse(char s[]) {
  int c, i, j;
  for (i = 0, j = strlen(s) - 1; i < j; i++, j--) {
    c = s[i];
    s[i] = s[j];
    s[j] = c;
  }
}

void append(char s[], char n) {
  int len = strlen(s);
  s[len] = n;
  s[len + 1] = '\0';
}

void backspace(char s[]) {
  int len = strlen(s);
  s[len - 1] = '\0';
}

int strcmp(char s1[], char s2[]) {
  int i;
  for (i = 0; s1[i] == s2[i]; i++) {
    if (s1[i] == '\0')
      return 0;
  }
  return s1[i] - s2[i];
}*/
