#ifndef APIC_H
#define APIC_H

#include "stdint.h"

void lapic_handler();
void init_lapic_timer();
void enable_lapic();
void lapic_eoi();
void init_ioapic();
void ioapic_write(uint8_t reg, uint32_t value);
void enable_apic_msr();
int cpu_supports_apic();
void detect_and_enable_apic();
void check_apic_vectors();

#endif // !DEBUG
