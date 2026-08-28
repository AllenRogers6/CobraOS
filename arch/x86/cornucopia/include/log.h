// log.h
// #ifndef LOG_H
// #define LOG_H
//
// #include "cornucopia_serial.h"
// #include "serial.h"
//
// #define LOG_LEVEL_DEBUG 0
// #define LOG_LEVEL_INFO 1
// #define LOG_LEVEL_WARN 2
// #define LOG_LEVEL_ERROR 3
// #define LOG_LEVEL_NONE 4
//
// // Set current log level (adjust as needed)
// #ifndef LOG_LEVEL
// #define LOG_LEVEL LOG_LEVEL_DEBUG
// #endif
//
// #define LOG(level, fmt, ...) \
//   do { \
//     if ((level) >= LOG_LEVEL) { \
//       serial_printf(COM1_DATA_PORT, "[%s] %s:%d: " fmt "\r\n", #level, \
//                     __FILE__, __LINE__, ##__VA_ARGS__); \
//     } \
//   } while (0)
//
// #define LOG_DEBUG(fmt, ...) LOG(LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
// #define LOG_INFO(fmt, ...) LOG(LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
// #define LOG_WARN(fmt, ...) LOG(LOG_LEVEL_WARN, fmt, ##__VA_ARGS__)
// #define LOG_ERROR(fmt, ...) LOG(LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)
//
// #endif
