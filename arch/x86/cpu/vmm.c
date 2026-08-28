#include "vmm.h"
#include "pmm.h"
#include <string.h>

static uint32_t *current_page_directory = NULL;

void vmm_init(uint32_t *page_dir) { current_page_directory = page_dir; }

bool map_page(uint32_t virt, uint32_t phys, uint32_t flags) {
  uint32_t pd_index = virt >> 22;
  uint32_t pt_index = (virt >> 12) & 0x3FF;

  uint32_t *page_dir = current_page_directory;

  if (!(page_dir[pd_index] & 1)) {
    uint32_t *new_pt = (uint32_t *)pmm_alloc_page();
    if (!new_pt)
      return false;
    memset(new_pt, 0, PAGE_SIZE);
    page_dir[pd_index] = (uint32_t)new_pt | (flags & 0xFFF) | 1;
  }

  uint32_t *page_table = (uint32_t *)(page_dir[pd_index] & ~0xFFF);
  // Page tables are in physical memory, but we access them via higher-half
  // mapping Since we identity-mapped the first 4 MB, if the table is within
  // 0-4MB, it's okay. Otherwise, we need to temporarily map it. For simplicity,
  // we'll restrict early heap to low memory.
  page_table[pt_index] = phys | flags | 1;
  return true;
}

void unmap_page(uint32_t virt) {}

bool is_mapped(uint32_t virt) {
  uint32_t pd_index = virt >> 22;
  uint32_t pt_index = (virt >> 12) & 0x3FF;
  uint32_t *page_dir = current_page_directory;
  if (!(page_dir[pd_index] & 1))
    return false;
  uint32_t *page_table = (uint32_t *)(page_dir[pd_index] & ~0xFFF);
  return (page_table[pt_index] & 1) != 0;
}
