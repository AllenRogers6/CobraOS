#include "stdio.h"
#include "screen.h"
#include "stddef.h"
#include "stdlib.h"
#include <stdarg.h>
#include <stdbool.h>

int snprintf(char *buffer, size_t size, const char *format, ...) {
  va_list args;
  va_start(args, format);

  size_t written = 0;
  const char *p = format;

  while (*p && written < size) {
    if (*p == '%' && *(p + 1)) {
      p++;
      switch (*p) {
      case 'd': { // Integer (decimal)
        int num = va_arg(args, int);
        char temp[32];
        int len = 0;

        // Handle negative numbers
        if (num < 0) {
          if (written < size) {
            buffer[written++] = '-';
          }
          num = -num;
        }

        // Handle zero
        if (num == 0) {
          if (written < size) {
            buffer[written++] = '0';
          }
          break;
        }

        // Convert integer to string
        while (num > 0) {
          temp[len++] = '0' + (num % 10);
          num /= 10;
        }

        // Write the number to the buffer
        for (int i = len - 1; i >= 0 && written < size; i--) {
          buffer[written++] = temp[i];
        }
        break;
      }
      case 's': { // String
        const char *str = va_arg(args, const char *);
        while (*str && written < size) {
          buffer[written++] = *str++;
        }
        break;
      }
      case 'c': { // Character
        char c = va_arg(args, int);
        if (written < size) {
          buffer[written++] = c;
        }
        break;
      }
      case 'x': { // Hexadecimal (lowercase)
        unsigned int num = va_arg(args, unsigned int);
        char temp[32];
        int len = 0;

        // Handle zero
        if (num == 0) {
          if (written < size) {
            buffer[written++] = '0';
          }
          break;
        }

        // Convert integer to hexadecimal string
        while (num > 0) {
          int digit = num % 16;
          temp[len++] = (digit < 10) ? ('0' + digit) : ('a' + (digit - 10));
          num /= 16;
        }

        // Write the hexadecimal number to the buffer
        for (int i = len - 1; i >= 0 && written < size; i--) {
          buffer[written++] = temp[i];
        }
        break;
      }
      case '%': { // Literal '%'
        if (written < size) {
          buffer[written++] = '%';
        }
        break;
      }
      default: { // Unsupported format specifier
        if (written < size) {
          buffer[written++] = '%';
        }
        if (written < size) {
          buffer[written++] = *p;
        }
        break;
      }
      }
    } else {
      if (written < size) {
        buffer[written++] = *p;
      }
    }
    p++;
  }

  va_end(args);

  // Null-terminate the buffer
  if (size > 0) {
    buffer[written < size ? written : size - 1] = '\0';
  }

  return written;
}

void printf(const char *format, ...) {
  va_list args;
  va_start(args, format);

  while (*format) {
    if (*format == '%') {
      format++;
      switch (*format) {
      case 'd': { // Integer (decimal)
        int num = va_arg(args, int);
        char buffer[32];
        int len = 0;

        // Handle negative numbers
        if (num < 0) {
          cprint('-');
          num = -num;
        }

        // Handle zero
        if (num == 0) {
          cprint('0');
          break;
        }

        // Convert integer to string
        while (num > 0) {
          buffer[len++] = '0' + (num % 10);
          num /= 10;
        }

        // Write the number to the screen
        for (int i = len - 1; i >= 0; i--) {
          cprint(buffer[i]);
        }
        break;
      }
      case 's': { // String
        char *str = va_arg(args, char *);
        viprint(str);
        break;
      }
      case 'c': { // Character
        char c = va_arg(args, int);
        cprint(c);
        break;
      }
      case 'x':   // Hexadecimal (lowercase)
      case 'X': { // Hexadecimal (uppercase)
        unsigned int num = va_arg(args, unsigned int);
        char buffer[32];
        int len = 0;
        bool uppercase = (*format == 'X');

        // Handle zero
        if (num == 0) {
          cprint('0');
          break;
        }

        // Convert integer to hexadecimal string
        while (num > 0) {
          int digit = num % 16;
          buffer[len++] = (digit < 10) ? ('0' + digit)
                                       : (uppercase ? ('A' + (digit - 10))
                                                    : ('a' + (digit - 10)));
          num /= 16;
        }

        // Write the hexadecimal number to the screen
        for (int i = len - 1; i >= 0; i--) {
          cprint(buffer[i]);
        }
        break;
      }
      case '%': { // Literal '%'
        cprint('%');
        break;
      }
      default: { // Unsupported format specifier
        cprint('%');
        cprint(*format);
        break;
      }
      }
    } else {
      cprint(*format);
    }
    format++;
  }

  va_end(args);
}
