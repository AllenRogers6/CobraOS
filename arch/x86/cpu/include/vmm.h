#ifndef VMM_H
#define VMM_H

#include <stdbool.h>
#include <stdint.h>

#define PAGE_PRESENT 1
#define PAGE_RW 2
#define PAGE_USER 4

void vmm_init(uint32_t *page_dir);
bool map_page(uint32_t virt, uint32_t phys, uint32_t flags);
void unmap_page(uint32_t virt);
bool is_mapped(uint32_t virt);

#endif
