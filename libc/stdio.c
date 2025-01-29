#include "stdio.h"
#include "stdarg.h"
#include "stddef.h"

int snprintf(char *buffer, size_t size, const char *format, ...) {
  va_list args;
  size_t written = 0;
  const char *p = format;

  va_start(args, format);

  while (*p) {
    if (*p == '%' && *(p + 1)) {
      p++;
      if (*p == 'd') {
        int num = va_arg(args, int);
        char temp[20];
        int len = snprintf(temp, sizeof(temp), "%d", num);
        for (int i = 0; i < len && written < size - 1; i++) {
          buffer[written++] = temp[i];
        }
      } else if (*p == 's') {
        const char *str = va_arg(args, const char *);
        while (*str && written < size - 1) {
          buffer[written++] = *str++;
        }
      }
    } else {
      if (written < size - 1) {
        buffer[written++] = *p;
      }
    }
    p++;
  }

  va_end(args);

  if (size > 0) {
    buffer[written] = '\0';
  }

  return written;
}
