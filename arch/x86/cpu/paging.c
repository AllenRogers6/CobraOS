#include "paging.h"
#include "string.h"
#include <stdint.h>

static uint32_t page_directory[1024] __attribute__((aligned(4096)));

// Page tables for the first 64 MB (16 tables × 4 MB each)
static uint32_t pt0[1024] __attribute__((aligned(4096)));  // 0–4 MB
static uint32_t pt1[1024] __attribute__((aligned(4096)));  // 4–8 MB
static uint32_t pt2[1024] __attribute__((aligned(4096)));  // 8–12 MB
static uint32_t pt3[1024] __attribute__((aligned(4096)));  // 12–16 MB
static uint32_t pt4[1024] __attribute__((aligned(4096)));  // 16–20 MB
static uint32_t pt5[1024] __attribute__((aligned(4096)));  // 20–24 MB
static uint32_t pt6[1024] __attribute__((aligned(4096)));  // 24–28 MB
static uint32_t pt7[1024] __attribute__((aligned(4096)));  // 28–32 MB
static uint32_t pt8[1024] __attribute__((aligned(4096)));  // 32–36 MB
static uint32_t pt9[1024] __attribute__((aligned(4096)));  // 36–40 MB
static uint32_t pt10[1024] __attribute__((aligned(4096))); // 40–44 MB
static uint32_t pt11[1024] __attribute__((aligned(4096))); // 44–48 MB
static uint32_t pt12[1024] __attribute__((aligned(4096))); // 48–52 MB
static uint32_t pt13[1024] __attribute__((aligned(4096))); // 52–56 MB
static uint32_t pt14[1024] __attribute__((aligned(4096))); // 56–60 MB
static uint32_t pt15[1024] __attribute__((aligned(4096))); // 60–64 MB

#define FILL_PT(pt, base_mb)                                                   \
  for (int i = 0; i < 1024; i++)                                               \
    pt[i] = ((base_mb * 0x100000) + i * 4096) | 3;

void paging_init(void) {
  FILL_PT(pt0, 0);
  FILL_PT(pt1, 4);
  FILL_PT(pt2, 8);
  FILL_PT(pt3, 12);
  FILL_PT(pt4, 16);
  FILL_PT(pt5, 20);
  FILL_PT(pt6, 24);
  FILL_PT(pt7, 28);
  FILL_PT(pt8, 32);
  FILL_PT(pt9, 36);
  FILL_PT(pt10, 40);
  FILL_PT(pt11, 44);
  FILL_PT(pt12, 48);
  FILL_PT(pt13, 52);
  FILL_PT(pt14, 56);
  FILL_PT(pt15, 60);

  memset(page_directory, 0, sizeof(page_directory));

  page_directory[0] = (uint32_t)pt0 | 3;
  page_directory[1] = (uint32_t)pt1 | 3;
  page_directory[2] = (uint32_t)pt2 | 3;
  page_directory[3] = (uint32_t)pt3 | 3;
  page_directory[4] = (uint32_t)pt4 | 3;
  page_directory[5] = (uint32_t)pt5 | 3;
  page_directory[6] = (uint32_t)pt6 | 3;
  page_directory[7] = (uint32_t)pt7 | 3;
  page_directory[8] = (uint32_t)pt8 | 3;
  page_directory[9] = (uint32_t)pt9 | 3;
  page_directory[10] = (uint32_t)pt10 | 3;
  page_directory[11] = (uint32_t)pt11 | 3;
  page_directory[12] = (uint32_t)pt12 | 3;
  page_directory[13] = (uint32_t)pt13 | 3;
  page_directory[14] = (uint32_t)pt14 | 3;
  page_directory[15] = (uint32_t)pt15 | 3;

  asm volatile("mov %0, %%cr3" : : "r"(page_directory));

  uint32_t cr0;
  asm volatile("mov %%cr0, %0" : "=r"(cr0));
  cr0 |= 0x80000000;
  asm volatile("mov %0, %%cr0" : : "r"(cr0));
}
