#include <stdint.h>

#define PAGE_PRESENT 0x1
#define PAGE_RW 0x2
#define PAGE_SIZE_4MB 0x80

uint32_t page_directory[1024] __attribute__((aligned(4096)));

uint32_t page_table[1024] __attribute__((aligned(4096)));

void setup_paging() {

  for (int i = 0; i < 1024; i++) {
    page_table[i] = (i * 0x1000) | PAGE_PRESENT | PAGE_RW;
  }

  page_directory[0] = ((uint32_t)page_table) | PAGE_PRESENT | PAGE_RW;

  __asm__ volatile("mov %0, %%cr3" ::"r"(page_directory));

  __asm__ volatile("mov %cr0, %eax\n"
                   "or $0x80000000, %eax\n"
                   "mov %eax, %cr0\n");
}
