#ifndef PS2_H
#define PS2_H

#include <stdint.h>

#define PS2_DATA_PORT 0x60

void scanning_on();
uint8_t ps2_read();
void ps2_write(uint8_t port, uint8_t data);
void ps2_init();

#endif // !DEBUG
