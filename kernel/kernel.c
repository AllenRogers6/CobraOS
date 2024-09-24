#include "include/interrupts.h"
#include "include/terminal.h"
#include "keyboard.h"

void kernel_main(void) {

  terminal_initialize();

  terminal_writestring("CobraOS ver 1");

  enable_interrupts();

  init_interrupts();

  void keyboard_interrupt_handler();
}
