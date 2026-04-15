#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MAX_SCANCODE 128
#define LAYOUT_COUNT 3

typedef struct {
  int x;
  int y;
} point;

typedef struct {
  unsigned char data[512] __attribute__((aligned(16)));
} fpu_context_t;

typedef struct {
  const char *name;
  const char **lowercase;
  const char **uppercase;
} keymap;

typedef enum { QWERTY, AZERTY, DVORAK } keyboard_layout;

void ascii_converter(uint8_t scancode, char str[], size_t size);
void letter_to_screen(uint8_t scancode);
void keyboard_handler(void);
void set_keyboard_layout(keyboard_layout layout);
void testing_kb_output(void);

#endif
