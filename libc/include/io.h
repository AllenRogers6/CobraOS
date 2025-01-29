#ifndef IO_H
#define IO_H
#include <stdint.h>

uint8_t inb(uint16_t port);
void outb(uint16_t port, uint8_t val);
void io_wait(void);
void outl(uint16_t port, uint32_t value);
uint32_t inl(uint16_t port);

#endif // !IO_H
