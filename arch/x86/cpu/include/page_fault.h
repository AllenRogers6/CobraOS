#ifndef PAGE_FAULT_H
#define PAGE_FAULT_H

#include "stdint.h"

void page_fault_exception(uint64_t errorCode);

#endif // !DEBUG
