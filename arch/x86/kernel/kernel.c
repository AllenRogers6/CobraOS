#include "apic.h"
#include "interrupts.h"
#include "io.h"
#include "keyboard.h"
#include "screen.h"
#include "timer.h"

void kernel(void) {
  initScreen();
  writeStrToScreen("Turning on kernel\n");

  enable_lapic();
  init_lapic_timer();
  init_ioapic();
  setKeyboardLayout(QWERTY);

  initInts();

  asmIntsOn();

  writeStrToScreen("Kernel on\n");
}
