#ifndef PIC_H
#define PIC_H

#include "stdint.h"

void remap();
void disablePIC(void);
void setIqrMask(uint8_t IRQline);
void clearIqrMask(uint8_t IRQline);

#endif // !PIC_H
