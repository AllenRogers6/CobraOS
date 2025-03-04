#include "ps2.h"
#include "io.h"
#include "screen.h"
#include <stdint.h>

#define PS2_STATUS_PORT 0x64
#define PS2_COMMAND_PORT 0x64

void check_ps2_status() {
  uint8_t status = inb(PS2_STATUS_PORT);
  if (status & 0x20) {
    viprint("Timeout error\n");
  }
  if (status & 0x40) {
    viprint("Parity error\n");
  }
}

void scanning_on() {
  outb(PS2_DATA_PORT, 0xF4);
  has_loaded();
  viprint("Scanning on\n");
}

/*uint8_t ps2_read() {
  while (!(inb(PS2_STATUS_PORT) & 1))
    ;
  return inb(PS2_DATA_PORT);
}*/

uint8_t ps2_read() {
  while (!(inb(PS2_STATUS_PORT) & 1))
    ;
  uint8_t data = inb(PS2_DATA_PORT);
  viprint("Scancode: ");
  hexprint(data);
  viprint("\n");
  return data;
}

void ps2_write(uint8_t port, uint8_t data) {
  while (inb(PS2_STATUS_PORT) & 2)
    ;
  outb(port, data);
}

void ps2_init() {

  ps2_write(PS2_COMMAND_PORT, 0xAD);
  ps2_write(PS2_COMMAND_PORT, 0xA7);

  while (inb(PS2_STATUS_PORT) & 1) {
    inb(PS2_DATA_PORT);
  }

  ps2_write(PS2_COMMAND_PORT, 0x20);
  uint8_t config = ps2_read();

  config |= 0x03;

  ps2_write(PS2_COMMAND_PORT, 0x60);
  ps2_write(PS2_DATA_PORT, config);

  ps2_write(PS2_COMMAND_PORT, 0xAE);
  ps2_write(PS2_COMMAND_PORT, 0xA8);

  ps2_write(PS2_DATA_PORT, 0xFF);
  while (ps2_read() != 0xFA)
    ;

  scanning_on();

  check_ps2_status();
}
