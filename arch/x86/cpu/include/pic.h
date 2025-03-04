#ifndef PIC_H
#define PIC_H

#include <stdint.h>

void remap();
void disable_pic(void);
void set_mask(uint8_t IRQline);
void clear_mask(uint8_t IRQline);
uint16_t pic_get_irr(void);
uint16_t pic_get_isr(void);
void unmask_kb();

#endif // !PIC_H
