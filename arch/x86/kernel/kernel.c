#include "alignment_check.h"
#include "apic.h"
#include "checking_int.h"
#include "cornucopia_pic.h"
#include "cornucopia_serial.h"
#include "gdt.h"
#include "heap.h"
#include "interrupts.h"
#include "io.h"
#include "keyboard.h"
#include "ldt.h"
#include "multiboot.h"
#include "paging.h"
#include "pic.h"
#include "pit.h"
#include "ps2.h"
#include "ramfs.h"
#include "screen.h"
#include "serial.h"
#include "shell.h"
#include "stdbool.h"
#include "stdio.h"
#include "task.h"
#include "tss.h"
#include "vmm.h"
#include <stdint.h>

#define QUEUE_SIZE 128

#define BUFFER_SIZE 256

extern char keyboard_buffer[256];
extern volatile int buffer_index;
extern uint32_t *kernel_page_directory;

/*function to setup gdt, tss, and ldt*/
void setup_gdt_and_tss() {
  init_gdt();
  load_gdt();
  init_ldt();
  load_ldt();
  init_tss();
  load_tss();
  reload_segment_registers();
}

/*function to verify which masks there is currently*/
void verify_mask() {
  uint8_t mask = inb(PIC_DATA_MASTER);
  viprint("Master PIC mask: ");
  hexprint(mask);
  viprint("\n");
}

/*get irr and isr and print them*/
void get_things() {
  uint16_t irr = pic_get_irr();
  uint16_t isr = pic_get_isr();
  viprint("IRR: ");
  hexprint(irr);
  viprint("\nISR: ");
  hexprint(isr);
  viprint("\n");
}

/*check mode for cr0*/
/*int check_cpu_mode_cr0() {
  unsigned int cr0;

  __asm__ volatile("mov %%cr0, %%eax\n\t"
                   "mov %%eax, %0"
                   : "=m"(cr0)
                   :
                   : "%eax");

  if (cr0 & 0x1) {

    return 1;
  } else {

    return 0;
  }
}*/

/*get pic*/
uint8_t get_pic(uint8_t ocw3) {
  outb(0x20, ocw3);

  return inb(0x20);
}

/*checkwhat ints were sent*/
void check_int_sent() {
  uint8_t irr = get_pic(0x0A);

  uint8_t isr = get_pic(0x0B);

  printf("IRR: %X\n", irr);

  printf("ISR: %X\n", isr);
}

/*check mode for cpuid*/
int check_cpu_mode_cpuid() {
  unsigned int eax, ebx, ecx, edx;

  __asm__ volatile("xor %%eax, %%eax\n"
                   "cpuid"
                   : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                   :
                   : "cc");

  return 1;
}

/*check mode for CS*/
int check_cpu_mode_cs_reg() {
  unsigned short cs_register;

  __asm__ volatile("mov %%cs, %0" : "=r"(cs_register));

  if (cs_register < 0x1000) {

    return 0;
  } else {

    return 1;
  }
}

/*function to check current mode for cpuid, cs, and cr0*/
void check_state() {
  /*if (check_cpu_mode_cr0()) {

    viprint("Protected Mode for CR\n");

  } else {

    viprint("Real Mode for CR\n");
  }*/

  if (check_cpu_mode_cpuid()) {

    viprint("Protected Mode for CPUID\n");
  } else {

    viprint("Real Mode for CPUID\n");
  }

  if (check_cpu_mode_cs_reg()) {

    viprint("Protected Mode for CS\n");

  } else {

    viprint("Real Mode for CS\n");
  }
}

/*kb listener vars*/
uint8_t scancode_queue[QUEUE_SIZE];
uint8_t queue_head = 0;
uint8_t queue_tail = 0;

/*enqueue the scancode*/
void enqueue_scancode(uint8_t scancode) {
  scancode_queue[queue_tail] = scancode;
  queue_tail = (queue_tail + 1) % QUEUE_SIZE;
}

/*dequeue the scancode*/
uint8_t dequeue_scancode() {
  uint8_t scancode = scancode_queue[queue_head];
  queue_head = (queue_head + 1) % QUEUE_SIZE;
  return scancode;
}

/*check if the queue if empty*/
bool queue_is_empty() { return queue_head == queue_tail; }

/*wait and listen for the keyboard to send*/
void keyboard_listener() {
  viprint("Keyboard listener started\n");

  while (true) {
    while (!queue_is_empty()) {
      uint8_t scancode = dequeue_scancode();
      hexprint(scancode);
    }

    asm volatile("hlt");
  }
}

/*x86 (32-bit) kernel*/
void kernel(uint32_t magic, uint32_t multiboot_info_ptr) {
  init_screen();

  has_loaded();

  viprint("Content load...\n");
  viprint("Set patch v0.4\n");

  viprint("Magic: ");
  hexprint(magic);
  viprint("\n");

  if (magic != 0x2BADB002) {
    viprint("Error: Invalid Multiboot magic!\n");
    while (1)
      asm("hlt");
  }

  viprint("Kernel running at virtual address: ");
  hexprint((uint32_t)&kernel);
  viprint("\n");

  setup_gdt_and_tss();

  init_ints();

  pit_init(100);
  irq_register_handler(0, pit_handler);
  irq_register_handler(1, keyboard_handler);
  asm_ints_on();

  paging_init();

  parse_multiboot_info(multiboot_info_ptr);

  // vmm_init(kernel_page_directory);
  heap_init();

  unmask_kb();

  clear_mask(0);

  has_loaded();
  set_keyboard_layout(QWERTY);

  has_loaded();
  ramfs_init();
  ramfs_set_cwd(ramfs_get_root());

  has_loaded();

  has_loaded();
  tasking_init();

  has_loaded();
  viprint("Content load complete\n");
  viprint("\nCobra\n\n");

  has_loaded();
  shell_init();

  for (;;) {

    asm volatile("sti; hlt; cli");

    while (keyboard_has_char()) {
      char c = keyboard_get_char();
      shell_feed_char(c);
    }
  }
}
