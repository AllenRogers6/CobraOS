#ifndef PIC_H
#define PIC_H

#include "stdint.h"

void remap_pic();
void pic_disable(void);
void IRQ_set_mask(uint8_t IRQline);
void IRQ_clear_mask(uint8_t IRQline);

#endif // !PIC_H
