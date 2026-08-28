#ifndef KLOG_H
#define KLOG_H

#include <stdint.h>

/*
 * Kernel Logging — structured output over COM1 serial port.
 *
 * All output goes to serial only, keeping the VGA display clean for the
 * shell and user-facing output.  Connect via QEMU's -serial stdio or
 * -serial file:serial.log.
 *
 * Format:
 *   [LEVEL] module: message
 *   [LEVEL] module: message (0xVALUE)
 *
 * Log levels (lowest to highest severity):
 *   KLOG_DEBUG  — verbose developer info, can be compiled out
 *   KLOG_INFO   — normal operational messages
 *   KLOG_WARN   — unexpected but recoverable situations
 *   KLOG_ERROR  — serious errors; system may continue
 *   KLOG_PANIC  — fatal; klog_panic() never returns
 */

typedef enum {
  KLOG_DEBUG = 0,
  KLOG_INFO,
  KLOG_WARN,
  KLOG_ERROR,
  KLOG_PANIC
} klog_level_t;

/* Minimum level to emit.  Messages below this are silently dropped.
 * Change at compile time with -DKLOG_MIN_LEVEL=KLOG_INFO etc.     */
#ifndef KLOG_MIN_LEVEL
#define KLOG_MIN_LEVEL KLOG_DEBUG
#endif

/* Initialise the serial port used for logging (call once, early in boot). */
void klog_init(void);

/* Core log functions */
void klog(klog_level_t level, const char *module, const char *msg);
void klog_hex(klog_level_t level, const char *module, const char *msg,
              uint32_t val);
void klog_dec(klog_level_t level, const char *module, const char *msg,
              uint32_t val);

/*
 * klog_panic — print msg + optional register state to serial and VGA,
 * then halt the CPU permanently.  Never returns.
 */
void klog_panic(const char *module, const char *msg);

/* Convenience macros so call sites don't repeat the level */
#define KDBG(mod, msg) klog(KLOG_DEBUG, mod, msg)
#define KDBG_HEX(mod, msg, v) klog_hex(KLOG_DEBUG, mod, msg, v)
#define KINFO(mod, msg) klog(KLOG_INFO, mod, msg)
#define KINFO_HEX(mod, msg, v) klog_hex(KLOG_INFO, mod, msg, v)
#define KWARN(mod, msg) klog(KLOG_WARN, mod, msg)
#define KERR(mod, msg) klog(KLOG_ERROR, mod, msg)
#define KERR_HEX(mod, msg, v) klog_hex(KLOG_ERROR, mod, msg, v)
#define KPANIC(mod, msg) klog_panic(mod, msg)

#endif /* KLOG_H */
