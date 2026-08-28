#include "pmm.h"
#include "screen.h"
#include <stdbool.h>
#include <stdint.h>

#define PAGE_SIZE 4096
#define MAX_FRAMES 32768 // 128 MB

static uint32_t pmm_bitmap[MAX_FRAMES / 32 + 1];

static uint32_t free_stack[MAX_FRAMES];
static uint32_t stack_top = 0;
static uint32_t total_frames = 0;

static size_t free_page_count = 0;

extern uint32_t _kernel_end;

static inline uint32_t page_to_index(void *page) {
  return ((uintptr_t)page) / PAGE_SIZE;
}

static inline void bitmap_set(uint32_t index) {
  pmm_bitmap[index / 32] |= (1U << (index % 32));
}

static inline void bitmap_clear(uint32_t index) {
  pmm_bitmap[index / 32] &= ~(1U << (index % 32));
}

static inline bool bitmap_test(uint32_t index) {
  return (pmm_bitmap[index / 32] & (1U << (index % 32))) != 0;
}

void pmm_init(uint32_t total_memory) {
  total_frames = total_memory / PAGE_SIZE;
  stack_top = 0;
  free_page_count = 0;

  for (uint32_t i = 0; i < sizeof(pmm_bitmap) / sizeof(pmm_bitmap[0]); i++)
    pmm_bitmap[i] = 0;

  viprint("PMM (stack + bitmap): total_frames=");
  hexprint(total_frames);
  viprint("\n");
}

void *pmm_alloc_page(void) {
  if (stack_top == 0)
    return NULL;

  uint32_t page = free_stack[--stack_top];
  uint32_t index = page_to_index((void *)page);

  bitmap_clear(index);

  if (free_page_count > 0)
    free_page_count--;

  return (void *)page;
}

void pmm_free_page(void *page) {
  uint32_t index = page_to_index(page);

  if (bitmap_test(index)) {
    viprint("PMM: double free at ");
    hexprint((uint32_t)page);
    viprint("\n");
    return;
  }

  if (stack_top >= MAX_FRAMES) {
    viprint("PMM free stack overflow!\n");
    return;
  }

  free_stack[stack_top++] = (uint32_t)page;

  bitmap_set(index);

  free_page_count++;
}

size_t pmm_get_free_page_count(void) { return free_page_count; }

bool pmm_is_page_free(void *page) {
  uint32_t index = page_to_index(page);
  return bitmap_test(index);
}
