#include "tss.h"
#include "gdt.h"
#include "string.h"

struct tss_entry tss;

void init_tss() {
  memset(&tss, 0, sizeof(tss));

  tss.ss0 = DATA_SEGMENT;
  tss.esp0 = 0x9000;

  tss.ldt = LDT_SEGMENT;

  tss.iomap_base = sizeof(tss);
}

void load_tss() { asm volatile("ltr %0" : : "r"((uint16_t)TSS_SEGMENT)); }
