#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "stddef.h"
#include "stdint.h"

typedef enum { QWERTY, AZERTY, DVORAK } keyboard_layout;

void ascii_converter(uint8_t scancode, char str[], size_t size);
void letter_to_screen(uint8_t scancode);
void keyboard_handler(void);
void set_keyboard_layout(keyboard_layout layout);
void testing_kb_output(void);

#endif // !KEYBOARD_H
