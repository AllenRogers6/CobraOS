#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "stddef.h"
#include "stdint.h"

typedef enum { QWERTY, AZERTY, DVORAK } KeyboardLayout;

void asciiConverter(uint8_t scancode, char str[], size_t size);
void keyboardHandler();
void letterToScreen(uint8_t scancode);
void setKeyboardLayout(KeyboardLayout layout);

#endif // !KEYBOARD_H
