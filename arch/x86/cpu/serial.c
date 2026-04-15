#include "io.h"

#define SERIAL_PORT 0x3F8
void serial_write_char(char c) {
  while ((inb(SERIAL_PORT + 5) & 0x20) == 0)
    ;
  outb(SERIAL_PORT, c);
}

void serial_write_string(const char *s) {
  while (*s)
    serial_write_char(*s++);
}

void serial_write_hex(uint32_t value) {
  char hex[] = "00000000";
  for (int i = 7; i >= 0; i--) {
    hex[i] = "0123456789ABCDEF"[value & 0xF];
    value >>= 4;
  }
  serial_write_string("0x");
  serial_write_string(hex);
}
