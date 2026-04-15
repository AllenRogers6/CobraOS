#ifndef SERIAL_H
#define SERIAL_H

void serial_write_char(char c);
void serial_write_string(const char *s);
void serial_write_hex(uint32_t value);

#endif // !DEBUG
