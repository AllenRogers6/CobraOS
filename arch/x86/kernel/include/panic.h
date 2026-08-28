#ifndef PANIC_H
#define PANIC_H

#include "registers.h"
#include <stdint.h>

/*
 * panic.h — Kernel Panic & Exception Register Dump
 *
 * Two entry points:
 *
 *  panic(msg)
 *      Call from anywhere in kernel C code when a fatal condition is hit.
 *      Captures live registers, dumps to serial + VGA, halts.
 *
 *  panic_regs(msg, regs)
 *      Call from an IDT exception handler that already has a registers_t
 *      from its assembly stub (e.g. page fault, GPF, double fault).
 *      Prints the saved register state — more accurate than live capture.
 */

void panic(const char *msg);
void panic_regs(const char *msg, registers_t *regs);

#endif /* PANIC_H */
