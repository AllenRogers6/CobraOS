#ifndef PMM_H
#define PMM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PAGE_SIZE 4096

void pmm_init(uint32_t total_memory);
void *pmm_alloc_page(void);
void pmm_free_page(void *page);
size_t pmm_get_free_page_count(void);

#endif
