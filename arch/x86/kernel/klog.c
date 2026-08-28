#include "klog.h"
#include "io.h"
#include "screen.h"
#include <stdint.h>

#define COM1 0x3F8

static void serial_init_com1(void) {
  outb(COM1 + 1, 0x00);
  outb(COM1 + 3, 0x80);
  outb(COM1 + 0, 0x03);
  outb(COM1 + 1, 0x00);
  outb(COM1 + 3, 0x03);
  outb(COM1 + 2, 0xC7);
  outb(COM1 + 4, 0x0B);
}

static void serial_putc(char c) {
  while ((inb(COM1 + 5) & 0x20) == 0)
    ;
  if (c == '\n') {
    outb(COM1, '\r');
    while ((inb(COM1 + 5) & 0x20) == 0)
      ;
  }
  outb(COM1, c);
}

static void serial_puts(const char *s) {
  while (*s)
    serial_putc(*s++);
}

static void serial_put_hex(uint32_t v) {
  serial_puts("0x");
  for (int i = 7; i >= 0; i--) {
    char digit = "0123456789ABCDEF"[(v >> (i * 4)) & 0xF];
    serial_putc(digit);
  }
}

static void serial_put_dec(uint32_t v) {
  if (v == 0) {
    serial_putc('0');
    return;
  }
  char buf[12];
  int i = sizeof(buf) - 1;
  buf[i] = '\0';
  while (v > 0) {
    buf[--i] = '0' + (v % 10);
    v /= 10;
  }
  serial_puts(&buf[i]);
}

void klog_init(void) {
  serial_init_com1();
  serial_puts("\r\n--- klog initialised ---\r\n");
}

static const char *level_tag(klog_level_t level) {
  switch (level) {
  case KLOG_DEBUG:
    return "[DBG] ";
  case KLOG_INFO:
    return "[INF] ";
  case KLOG_WARN:
    return "[WRN] ";
  case KLOG_ERROR:
    return "[ERR] ";
  case KLOG_PANIC:
    return "[!!!] ";
  default:
    return "[???] ";
  }
}

void klog(klog_level_t level, const char *module, const char *msg) {
  if (level < KLOG_MIN_LEVEL)
    return;
  serial_puts(level_tag(level));
  serial_puts(module);
  serial_puts(": ");
  serial_puts(msg);
  serial_putc('\n');
}

void klog_hex(klog_level_t level, const char *module, const char *msg,
              uint32_t val) {
  if (level < KLOG_MIN_LEVEL)
    return;
  serial_puts(level_tag(level));
  serial_puts(module);
  serial_puts(": ");
  serial_puts(msg);
  serial_putc(' ');
  serial_put_hex(val);
  serial_putc('\n');
}

void klog_dec(klog_level_t level, const char *module, const char *msg,
              uint32_t val) {
  if (level < KLOG_MIN_LEVEL)
    return;
  serial_puts(level_tag(level));
  serial_puts(module);
  serial_puts(": ");
  serial_puts(msg);
  serial_putc(' ');
  serial_put_dec(val);
  serial_putc('\n');
}

void klog_panic(const char *module, const char *msg) {
  uint32_t eax, ebx, ecx, edx, esi, edi, ebp, esp, eflags;
  asm volatile("mov %%eax, %0\n"
               "mov %%ebx, %1\n"
               "mov %%ecx, %2\n"
               "mov %%edx, %3\n"
               "mov %%esi, %4\n"
               "mov %%edi, %5\n"
               "mov %%ebp, %6\n"
               "mov %%esp, %7\n"
               "pushf; pop %8\n"
               : "=m"(eax), "=m"(ebx), "=m"(ecx), "=m"(edx), "=m"(esi),
                 "=m"(edi), "=m"(ebp), "=m"(esp), "=m"(eflags));

  asm volatile("cli");

  serial_puts("\r\n========================================\r\n");
  serial_puts("[!!!] KERNEL PANIC\r\n");
  serial_puts("Module : ");
  serial_puts(module);
  serial_puts("\r\n");
  serial_puts("Reason : ");
  serial_puts(msg);
  serial_puts("\r\n");
  serial_puts("----------------------------------------\r\n");
  serial_puts("EAX=");
  serial_put_hex(eax);
  serial_puts("  ");
  serial_puts("EBX=");
  serial_put_hex(ebx);
  serial_puts("\r\n");
  serial_puts("ECX=");
  serial_put_hex(ecx);
  serial_puts("  ");
  serial_puts("EDX=");
  serial_put_hex(edx);
  serial_puts("\r\n");
  serial_puts("ESI=");
  serial_put_hex(esi);
  serial_puts("  ");
  serial_puts("EDI=");
  serial_put_hex(edi);
  serial_puts("\r\n");
  serial_puts("EBP=");
  serial_put_hex(ebp);
  serial_puts("  ");
  serial_puts("ESP=");
  serial_put_hex(esp);
  serial_puts("\r\n");
  serial_puts("EFLAGS=");
  serial_put_hex(eflags);
  serial_puts("\r\n");
  serial_puts("========================================\r\n");

  set_text_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
  viprint("\n\n*** KERNEL PANIC ***\n");
  viprint(module);
  viprint(": ");
  viprint(msg);
  viprint("\n");
  viprint("System halted. Check serial output for register dump.\n");
  set_text_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);

  for (;;)
    asm volatile("cli; hlt");
}
