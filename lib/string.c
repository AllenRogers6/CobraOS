#include "string.h"
#include <stddef.h>
#include <stdint.h>

char *strcpy(char *dest, const char *from) {
  char *start = dest;
  while (*from != '\0') { // Compare with '\0' instead of "\0"
    *dest = *from;
    dest++;
    from++;
  }
  *dest = '\0'; // Correctly null-terminate the destination string
  return start;
}

char *strncpy(char *dest, const char *from, size_t n) {
  size_t i;

  // Copy up to n characters from 'from' to 'dest'
  for (i = 0; i < n && from[i] != '\0'; i++) {
    dest[i] = from[i];
  }

  // Pad the remaining space in 'dest' with '\0'
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
    // Copy forward
    for (size_t i = 0; i < n; i++) {
      d[i] = s[i];
    }
  } else if (d > s) {
    // Copy backward
    for (size_t i = n; i > 0; i--) {
      d[i - 1] = s[i - 1];
    }
  }
  // If d == s, no copying is needed

  return dest;
}

void *memset(void *dest, int ch, size_t count) {
  uint8_t *p = (uint8_t *)dest;
  uint8_t c = (uint8_t)ch;

  // Align the destination to the nearest word boundary
  while (count > 0 && ((uintptr_t)p & (sizeof(uintptr_t) - 1))) {
    *p++ = c;
    count--;
  }

  // Fill in words
  uintptr_t word = 0;
  for (size_t i = 0; i < sizeof(uintptr_t); i++) {
    word |= (uintptr_t)c << (8 * i);
  }

  uintptr_t *wp = (uintptr_t *)p;
  while (count >= sizeof(uintptr_t)) {
    *wp++ = word;
    count -= sizeof(uintptr_t);
  }

  // Fill in remaining bytes
  p = (uint8_t *)wp;
  while (count > 0) {
    *p++ = c;
    count--;
  }

  return dest;
}
