#include <heap.h>
#include <screen.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef HEAP_SIZE
#ifdef HEAP_END
#define HEAP_SIZE (HEAP_END - HEAP_START)
#else
#define HEAP_SIZE (64 * 1024 * 1024) /* 64 MiB default */
#warning "HEAP_SIZE not defined; using default 64 MiB"
#endif
#endif

#define MIN_BLOCK_SHIFT 5 /* 2^5 = 32 bytes */
#define MIN_BLOCK_SIZE (1UL << MIN_BLOCK_SHIFT)

#define MAX_BLOCK_SHIFT 22 /* 2^22 = 4 MiB */
#define MAX_BLOCK_SIZE (1UL << MAX_BLOCK_SHIFT)

#if MAX_BLOCK_SIZE > HEAP_SIZE
#error "MAX_BLOCK_SIZE exceeds HEAP_SIZE"
#endif

typedef struct block {
  struct block *next; /* 4 bytes */
  uint32_t order;     /* 4 bytes */
  uint32_t free;      /* 4 bytes (0 or 1) */
  uint32_t self;      /* 4 bytes: address of this block, used by kfree() */
} block_t;

#define HEADER_SIZE sizeof(block_t) /* = 16 */

_Static_assert((HEADER_SIZE & 7) == 0, "Header size must be a multiple of 8");

static block_t *free_lists[MAX_BLOCK_SHIFT + 1];
// static uint8_t *heap_end = (uint8_t *)(HEAP_START + HEAP_SIZE);
static uint8_t *heap_top = (uint8_t *)HEAP_START;

static inline block_t *buddy_of(block_t *block);
static void split_block(uint32_t order);
static block_t *coalesce(block_t *block);
static void insert_free_block(block_t *block);
static void remove_free_block(block_t *block);
static void *buddy_alloc(size_t size);
static void buddy_free(void *ptr);

static void *heap_memset(void *dst, int c, size_t n);

void kmalloc_init(void) {
  for (int i = 0; i <= MAX_BLOCK_SHIFT; i++)
    free_lists[i] = NULL;

  uintptr_t aligned_start =
      (HEAP_START + MAX_BLOCK_SIZE - 1) & ~(MAX_BLOCK_SIZE - 1);

  if (aligned_start + MAX_BLOCK_SIZE > HEAP_START + HEAP_SIZE) {
    uint32_t max_order = MAX_BLOCK_SHIFT;
    while (max_order > MIN_BLOCK_SHIFT &&
           (aligned_start + (1UL << max_order) > HEAP_START + HEAP_SIZE))
      max_order--;

    if (max_order < MIN_BLOCK_SHIFT) {
      /* Heap too small even for a minimum block. Panic. */
      for (;;) {
        __asm__ volatile("cli; hlt");
      }
    }

    block_t *init_block = (block_t *)aligned_start;
    init_block->order = max_order;
    init_block->free = 1;
    init_block->next = NULL;
    init_block->self = 0;
    free_lists[max_order] = init_block;
    heap_top = (uint8_t *)(aligned_start + (1UL << max_order));
  } else {
    block_t *init_block = (block_t *)aligned_start;
    init_block->order = MAX_BLOCK_SHIFT;
    init_block->free = 1;
    init_block->next = NULL;
    init_block->self = 0;
    free_lists[MAX_BLOCK_SHIFT] = init_block;
    heap_top = (uint8_t *)(aligned_start + MAX_BLOCK_SIZE);
  }
}

static inline block_t *buddy_of(block_t *block) {
  uintptr_t addr = (uintptr_t)block;
  uintptr_t buddy_addr = addr ^ (1UL << block->order);
  return (block_t *)buddy_addr;
}

static void split_block(uint32_t order) {
  block_t *block = free_lists[order];
  remove_free_block(block);

  order--;
  block->order = order;
  block->free = 1;

  block_t *buddy = buddy_of(block);
  buddy->order = order;
  buddy->free = 1;
  buddy->self = 0;

  insert_free_block(block);
  insert_free_block(buddy);
}

static block_t *coalesce(block_t *block) {
  uint32_t order = block->order;

  while (order < MAX_BLOCK_SHIFT) {
    block_t *buddy = buddy_of(block);

    if (!buddy->free || buddy->order != order)
      break;

    remove_free_block(buddy);

    if ((uintptr_t)block > (uintptr_t)buddy)
      block = buddy;

    order++;
    block->order = order;
    block->free = 1;
  }

  return block;
}

static void insert_free_block(block_t *block) {
  uint32_t order = block->order;
  block->next = free_lists[order];
  free_lists[order] = block;
}

static void remove_free_block(block_t *block) {
  uint32_t order = block->order;
  block_t **indirect = &free_lists[order];

  while (*indirect) {
    if (*indirect == block) {
      *indirect = block->next;
      block->next = NULL;
      return;
    }
    indirect = &(*indirect)->next;
  }

  /* Corrupted free list. Panic. */
  for (;;) {
    __asm__ volatile("cli; hlt");
  }
}

static void *buddy_alloc(size_t size) {
  if (size == 0)
    return NULL;

  if (size > (size_t)-1 - HEADER_SIZE)
    return NULL;

  size_t total = size + HEADER_SIZE;

  uint32_t order = MIN_BLOCK_SHIFT;
  while (order < MAX_BLOCK_SHIFT && (1UL << order) < total)
    order++;

  if ((1UL << order) < total)
    return NULL; /* Larger than maximum buddy block (4 MiB) */

  uint32_t o = order;
  while (o <= MAX_BLOCK_SHIFT && free_lists[o] == NULL)
    o++;

  if (o > MAX_BLOCK_SHIFT)
    return NULL;

  while (o > order) {
    split_block(o);
    o--;
  }

  block_t *block = free_lists[order];
  remove_free_block(block);

  block->free = 0;
  block->self = (uint32_t)(uintptr_t)block;

  return (uint8_t *)block + HEADER_SIZE;
}

static void buddy_free(void *ptr) {
  if (!ptr)
    return;

  uintptr_t block_addr = *(uintptr_t *)((uint8_t *)ptr - sizeof(uintptr_t));
  block_t *block = (block_t *)block_addr;

  if (!block || block->free) {
    for (;;) {
      __asm__ volatile("cli; hlt");
    }
  }

  block->free = 1;
  block = coalesce(block);
  insert_free_block(block);
}

void *kmalloc(size_t size) {
  if (size == 0)
    return NULL;

  if (size > (size_t)-1 - 7)
    return NULL;

  size = (size + 7) & ~7UL;

  void *ptr = buddy_alloc(size);

  return ptr;
}

void kfree(void *ptr) { buddy_free(ptr); }

void *kcalloc(size_t nmemb, size_t size) {
  if (nmemb == 0 || size == 0)
    return NULL;

  if (size > (size_t)-1 / nmemb)
    return NULL;

  size_t total = nmemb * size;
  void *ptr = kmalloc(total);

  if (ptr)
    heap_memset(ptr, 0, total);

  return ptr;
}

void *kmalloc_aligned(size_t size, size_t alignment) {
  if (size == 0)
    return NULL;

  if (alignment < sizeof(void *))
    alignment = sizeof(void *);

  if ((alignment & (alignment - 1)) != 0) {
    size_t a = 1;
    while (a < alignment) {
      if (a > (size_t)-1 / 2) {
        return NULL; /* alignment too large */
      }
      a <<= 1;
    }
    alignment = a;
  }

  if (size > (size_t)-1 - alignment - sizeof(uintptr_t))
    return NULL;

  size_t total = size + alignment + sizeof(uintptr_t);

  void *raw = kmalloc(total);
  if (!raw)
    return NULL;

  uintptr_t addr = (uintptr_t)raw;
  uintptr_t aligned =
      (addr + sizeof(uintptr_t) + alignment - 1) & ~(alignment - 1);

  block_t *block = (block_t *)((uint8_t *)raw - HEADER_SIZE);
  *(uintptr_t *)(aligned - sizeof(uintptr_t)) = (uintptr_t)block;

  return (void *)aligned;
}

static void *heap_memset(void *dst, int c, size_t n) {
  uint8_t *p = (uint8_t *)dst;
  while (n--)
    *p++ = (uint8_t)c;
  return dst;
}

void heap_init(void) { kmalloc_init(); }

uint32_t heap_get_start(void) { return (uint32_t)HEAP_START; }

uint32_t heap_get_current(void) { return (uint32_t)heap_top; }
