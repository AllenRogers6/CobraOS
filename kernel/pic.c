#include "include/io.h"
#include "include/terminal.h"

#define PIC_COMMAND_SLAVE 0xA0
#define PIC_DATA_MASTER 0x21
#define PIC_COMMAND_MASTER 0x20
#define PIC_DATA_SLAVE 0xA1

#define ICW1 0x11
#define ICW4 0x1

#define PIC_OFFSET_MASTER 0x20
#define PIC_OFFSET_SLAVE 0x28

#define PIC_MASTER_ICW3 0x04
#define PIC_SLAVE_ICW3 0x02

// remapping
void remap_pic() {
  outb(PIC_COMMAND_MASTER, ICW1);
  outb(PIC_COMMAND_SLAVE, ICW1);

  outb(PIC_DATA_MASTER, PIC_OFFSET_MASTER);
  outb(PIC_DATA_SLAVE, PIC_OFFSET_SLAVE);

  outb(PIC_DATA_MASTER, PIC_MASTER_ICW3);
  outb(PIC_DATA_SLAVE, PIC_SLAVE_ICW3);

  outb(PIC_DATA_MASTER, ICW4);
  outb(PIC_DATA_SLAVE, ICW4);

  outb(PIC_DATA_MASTER, 0x0);
  outb(PIC_DATA_SLAVE, 0x0);
}

void pic_disable(void) {
  outb(PIC_DATA_SLAVE, 0xff);
  outb(PIC_DATA_MASTER, 0xff);
}

void IRQ_set_mask(uint8_t IRQline) {
  uint16_t port;
  uint8_t value;

  if (IRQline < 8) {
    port = PIC_DATA_MASTER;
  } else {
    port = PIC_DATA_SLAVE;
    IRQline -= 8;
  }
  value = inb(port) | (1 << IRQline);
  outb(port, value);
}

void IRQ_clear_mask(uint8_t IRQline) {
  uint16_t port;
  uint8_t value;

  if (IRQline < 8) {
    port = PIC_DATA_SLAVE;
  } else {
    port = PIC_DATA_MASTER;
    IRQline -= 8;
  }
  value = inb(port) & ~(1 << IRQline);
  outb(port, value);
}
