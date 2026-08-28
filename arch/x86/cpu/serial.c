/* #include "serial.h" */
/* #include "io.h" */
/* #include <stdarg.h> */
/* #include <stddef.h> */
/**/
/* void serial_init(uint16_t port) { */
/*   outb(port + 1, 0x00); // Disable interrupts */
/*   outb(port + 3, 0x80); // Enable DLAB */
/*   outb(port + 0, 0x01); // Divisor low byte (115200) */
/*   outb(port + 1, 0x00); // Divisor high byte */
/*   outb(port + 3, 0x03); // 8N1, clear DLAB */
/*   outb(port + 2, 0xC7); // Enable FIFO, clear queues */
/*   outb(port + 4, 0x0B); // DTR, RTS, OUT2 */
/* } */
/**/
/* static int serial_tx_ready(uint16_t port) { return inb(port + 5) & 0x20; } */
/**/
/* void serial_write_char(uint16_t port, char c) { */
/*   while (!serial_tx_ready(port)) { */
/*     // busy wait */
/*   } */
/*   outb(port, (uint8_t)c); */
/* } */
/**/
/* void serial_write_string(uint16_t port, const char *str) { */
/*   for (size_t i = 0; str[i] != '\0'; i++) { */
/*     serial_write_char(port, str[i]); */
/*   } */
/* } // <-- Make sure this closing brace is present! */
/**/
/* /* ------- Formatted output support ------- */
/**/
/* static char *uint_to_str(unsigned int value, char *buffer, int base, */
/*                          int uppercase) { */
/*   const char *digits_lower = "0123456789abcdef"; */
/*   const char *digits_upper = "0123456789ABCDEF"; */
/*   const char *digits = uppercase ? digits_upper : digits_lower; */
/*   char temp[33]; */
/*   int i = 0; */
/**/
/*   if (value == 0) { */
/*     temp[i++] = '0'; */
/*   } else { */
/*     while (value > 0) { */
/*       temp[i++] = digits[value % base]; */
/*       value /= base; */
/*     } */
/*   } */
/**/
/*   int j = 0; */
/*   while (i > 0) { */
/*     buffer[j++] = temp[--i]; */
/*   } */
/*   buffer[j] = '\0'; */
/*   return buffer; */
/* } */
/**/
/* static int vsnprintf_impl(char *buf, size_t size, const char *fmt, */
/*                           va_list args) { */
/*   size_t count = 0; */
/*   if (size == 0) */
/*     return 0; */
/**/
/*   while (*fmt && count < size - 1) { */
/*     if (*fmt != '%') { */
/*       buf[count++] = *fmt++; */
/*       continue; */
/*     } */
/**/
/*     if (*(fmt + 1) == '%') { */
/*       buf[count++] = '%'; */
/*       fmt += 2; */
/*       continue; */
/*     } */
/**/
/*     fmt++; // skip '%' */
/**/
/*     char tmp[64]; */
/*     switch (*fmt) { */
/*     case 'c': { */
/*       char c = (char)va_arg(args, int); */
/*       buf[count++] = c; */
/*       fmt++; */
/*       break; */
/*     } */
/*     case 's': { */
/*       const char *s = va_arg(args, const char *); */
/*       while (*s && count < size - 1) { */
/*         buf[count++] = *s++; */
/*       } */
/*       fmt++; */
/*       break; */
/*     } */
/*     case 'd': */
/*     case 'i': { */
/*       int num = va_arg(args, int); */
/*       if (num < 0) { */
/*         buf[count++] = '-'; */
/*         num = -num; */
/*       } */
/*       uint_to_str((unsigned int)num, tmp, 10, 0); */
/*       char *p = tmp; */
/*       while (*p && count < size - 1) */
/*         buf[count++] = *p++; */
/*       fmt++; */
/*       break; */
/*     } */
/*     case 'u': { */
/*       unsigned int num = va_arg(args, unsigned int); */
/*       uint_to_str(num, tmp, 10, 0); */
/*       char *p = tmp; */
/*       while (*p && count < size - 1) */
/*         buf[count++] = *p++; */
/*       fmt++; */
/*       break; */
/*     } */
/*     case 'x': { */
/*       unsigned int num = va_arg(args, unsigned int); */
/*       uint_to_str(num, tmp, 16, 0); */
/*       char *p = tmp; */
/*       while (*p && count < size - 1) */
/*         buf[count++] = *p++; */
/*       fmt++; */
/*       break; */
/*     } */
/*     case 'X': { */
/*       unsigned int num = va_arg(args, unsigned int); */
/*       uint_to_str(num, tmp, 16, 1); */
/*       char *p = tmp; */
/*       while (*p && count < size - 1) */
/*         buf[count++] = *p++; */
/*       fmt++; */
/*       break; */
/*     } */
/*     case 'p': { */
/*       void *ptr = va_arg(args, void *); */
/*       buf[count++] = '0'; */
/*       if (count < size - 1) */
/*         buf[count++] = 'x'; */
/*       uint_to_str((unsigned int)(uintptr_t)ptr, tmp, 16, 0); */
/*       char *p = tmp; */
/*       while (*p && count < size - 1) */
/*         buf[count++] = *p++; */
/*       fmt++; */
/*       break; */
/*     } */
/*     default: */
/*       buf[count++] = '%'; */
/*       buf[count++] = *fmt; */
/*       fmt++; */
/*       break; */
/*     } */
/*   } */
/**/
/*   buf[count] = '\0'; */
/*   return count; */
/* } */
/**/
/* void serial_printf(uint16_t port, const char *fmt, ...) { */
/*   char buffer[256]; */
/*   va_list args; */
/*   va_start(args, fmt); */
/*   vsnprintf_impl(buffer, sizeof(buffer), fmt, args); */
/*   va_end(args); */
/*   serial_write_string(port, buffer); */
/* } */
