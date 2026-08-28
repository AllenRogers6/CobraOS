#ifndef PAGE_FAULT_H
#define PAGE_FAULT_H

#include "stdint.h"

void page_fault_handler(uint32_t *esp);

#endif // !DEBUG
