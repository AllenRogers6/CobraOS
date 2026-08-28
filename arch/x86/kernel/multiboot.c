#include "multiboot.h"
#include "pmm.h"
#include "screen.h"

static uint32_t mem_map_addr = 0;
static uint32_t mem_map_length = 0;

extern uint32_t _kernel_end; // physical end of kernel

void parse_multiboot_info(uint32_t multiboot_info_ptr) {
  multiboot_info_t *mbi = (multiboot_info_t *)multiboot_info_ptr;

  if (!(mbi->flags & (1 << 6))) {
    viprint("Error: Multiboot memory map not provided!\n");
    while (1)
      asm("hlt");
  }

  mem_map_addr = mbi->mmap_addr;
  mem_map_length = mbi->mmap_length;

  uint32_t total_available = 0;
  multiboot_memory_map_t *mmap = (multiboot_memory_map_t *)mem_map_addr;
  while ((uint32_t)mmap < mem_map_addr + mem_map_length) {
    if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
      total_available += mmap->length_low;
    }
    mmap = (multiboot_memory_map_t *)((uint32_t)mmap + mmap->size +
                                      sizeof(mmap->size));
  }

  pmm_init(total_available);

  uint32_t kernel_phys_start = 0x100000; // 1 MiB
  uint32_t kernel_phys_end = (uint32_t)&_kernel_end;
  viprint("Kernel physical end: 0x");
  hexprint(kernel_phys_end);
  viprint("\n");

  mmap = (multiboot_memory_map_t *)mem_map_addr;
  while ((uint32_t)mmap < mem_map_addr + mem_map_length) {
    if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
      uint32_t base = mmap->base_addr_low;
      uint32_t length = mmap->length_low;
      for (uint32_t addr = base; addr < base + length; addr += PAGE_SIZE) {
        if (addr >= kernel_phys_start && addr < kernel_phys_end)
          continue;
        pmm_free_page((void *)addr);
      }
    }
    mmap = (multiboot_memory_map_t *)((uint32_t)mmap + mmap->size +
                                      sizeof(mmap->size));
  }
  viprint("PMM: free pages after init: ");
  hexprint(pmm_get_free_page_count());
  viprint("\n");

  viprint("Memory map parsed.\n");
}

void print_memory_map(void) {
  if (mem_map_addr == 0) {
    viprint("No memory map available.\n");
    return;
  }

  multiboot_memory_map_t *mmap = (multiboot_memory_map_t *)mem_map_addr;
  viprint("Memory Map:\n");

  while ((uint32_t)mmap < mem_map_addr + mem_map_length) {
    viprint("  Base: ");
    hexprint(mmap->base_addr_low);
    viprint("  Length: ");
    hexprint(mmap->length_low);
    viprint("  Type: ");
    if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
      viprint("Available\n");
    } else {
      viprint("Reserved\n");
    }
    mmap = (multiboot_memory_map_t *)((uint32_t)mmap + mmap->size +
                                      sizeof(mmap->size));
  }
}
