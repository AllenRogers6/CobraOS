#include "interrupts.h"
#include "keyboard.h"
#include "terminal.h"

void kernel_main(void) {

  terminal_initialize();

  terminal_writestring("CobraOS ver 1");

  enable_interrupts();

  init_interrupts();

  keyboard_interrupt_handler();
}
