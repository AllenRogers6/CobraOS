#ifndef HEAP_H
#define HEAP_H

#include <stddef.h>
#include <stdint.h>

#define HEAP_START 0x02000000 // 32 MB
#define HEAP_SIZE 0x02000000

void heap_init(void);
void *kmalloc(size_t size);
void kfree(void *ptr);
void *krealloc(void *ptr, size_t new_size);
uint32_t heap_get_start(void);
uint32_t heap_get_current(void);

#endif
