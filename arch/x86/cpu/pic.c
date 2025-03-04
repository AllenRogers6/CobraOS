#include "pic.h"
#include "cornucopia_pic.h"
#include "io.h"
#include "screen.h"
#include <stdint.h>

void pic_eoi(uint8_t irq) {
  if (irq >= 8)
    outb(PIC_COMMAND_SLAVE, 0x20);
  outb(PIC_COMMAND_MASTER, 0x20);
}

void remap() {
  uint8_t master_mask = inb(PIC_DATA_MASTER);
  uint8_t slave_mask = inb(PIC_DATA_SLAVE);

  outb(PIC_COMMAND_MASTER, ICW1_INIT | ICW1_ICW4);
  io_wait();
  outb(PIC_DATA_MASTER, PIC_OFFSET_MASTER);
  io_wait();
  outb(PIC_DATA_MASTER, PIC_MASTER_ICW3);
  io_wait();
  outb(PIC_DATA_MASTER, ICW4_8086);
  io_wait();

  outb(PIC_COMMAND_SLAVE, ICW1_INIT | ICW1_ICW4);
  io_wait();
  outb(PIC_DATA_SLAVE, PIC_OFFSET_SLAVE);
  io_wait();
  outb(PIC_DATA_SLAVE, PIC_SLAVE_ICW3);
  io_wait();
  outb(PIC_DATA_SLAVE, ICW4_8086);
  io_wait();

  outb(PIC_DATA_MASTER, master_mask);
  io_wait();
  outb(PIC_DATA_SLAVE, slave_mask);
  io_wait();

  viprint("Remap done\n");

  viprint("Masking all\n");

  outb(PIC_DATA_MASTER, 0xFF);

  outb(PIC_DATA_SLAVE, 0xFF);
  viprint("Masked\n");
}

void disable_pic(void) {
  set_mask(0);
  outb(PIC_DATA_SLAVE, 0xFF);
  io_wait();
  outb(PIC_DATA_MASTER, 0xFF);
  viprint("Disabled PIC\n");
}

void set_mask(uint8_t IRQline) {
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

void clear_mask(uint8_t IRQline) {
  uint16_t port;
  uint8_t value;

  if (IRQline < 8) {
    port = PIC_DATA_MASTER;
  } else {
    port = PIC_DATA_SLAVE;
    IRQline -= 8;
  }
  value = inb(port) & ~(1 << IRQline);
  outb(port, value);
}

static uint16_t __pic_get_irq_reg(int ocw3) {
  outb(PIC_COMMAND_MASTER, ocw3);
  outb(PIC_COMMAND_SLAVE, ocw3);
  return (inb(PIC_COMMAND_SLAVE) << 8) | inb(PIC_COMMAND_MASTER);
}

uint16_t pic_get_irr(void) { return __pic_get_irq_reg(PIC_READ_IRR); }

uint16_t pic_get_isr(void) { return __pic_get_irq_reg(PIC_READ_ISR); }

void unmask_kb() {

  uint8_t master_mask = inb(PIC_DATA_MASTER);
  master_mask &= ~(1 << 1);
  outb(PIC_DATA_MASTER, master_mask);
}
