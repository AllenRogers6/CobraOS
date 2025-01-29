#ifndef PAGEFAULT_H
#define PAGEFAULT_H

#include "stdint.h"

void pageFaultException(uint64_t errorCode);

#endif // !DEBUG
