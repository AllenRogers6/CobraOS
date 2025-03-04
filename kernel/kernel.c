#include <stdio.h>

#if defined(__x86_64__) || defined(_M_X64)
printf("x86_64");
#elif defined(__i386__) || defined(_M_IX86)
#define ARCH "x86 (32-bit)"
#elif defined(__aarch64__) || defined(_M_ARM64)
#define ARCH "ARM64"
#elif defined(__arm__) || defined(_M_ARM)
#define ARCH "ARM (32-bit)"
#elif defined(__powerpc64__) || defined(__ppc64__)
#define ARCH "PowerPC 64-bit"
#elif defined(__powerpc__) || defined(__ppc__)
#define ARCH "PowerPC 32-bit"
#elif defined(__riscv) || defined(__riscv__)
#define ARCH "RISC-V"
#else
#define ARCH "Unknown"
#endif
