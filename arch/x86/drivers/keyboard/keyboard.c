#include "keyboard.h"
#include "io.h"
#include "key_maps.h"
#include "ps2.h"
#include "screen.h"
#include "stdio.h"
#include "string.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*definitions*/
#define BUFFER_SIZE 256
#define KEYBOARD_STATUS_PORT 0x64
#define KEYBOARD_DATA_PORT 0x60

/*our static vars for this file*/
static int shift = 0;
static int caps = 0;
static int ctrl = 0;
static int alt = 0;
static keyboard_layout current_layout = QWERTY;
static bool extended_key = false;
static char keyboard_buffer[BUFFER_SIZE];
static int buffer_index = 0;
static int cursor_row = 0;
static int cursor_col = 0;
uint8_t fpu_state[512];

/*get the current keymap*/
const char **getKeyMap() {
  static const char *fallback_map[MAX_SCANCODE] = {0};
  switch (current_layout) {
  case QWERTY:
    return (shift || caps) ? qwerty_uppercase_key_map
                           : qwerty_lowercase_key_map;
  case AZERTY:
    return (shift || caps) ? azerty_uppercase_key_map
                           : azerty_lowercase_key_map;
  case DVORAK:
    return (shift || caps) ? dvorak_uppercase_key_map
                           : dvorak_lowercase_key_map;
  default:
    return fallback_map;
  }
}

/*convert to ascii from scancode*/
void ascii_converter(uint8_t scancode, char str[], size_t size) {
  const char **map = getKeyMap();
  if (scancode >= MAX_SCANCODE || !map[scancode]) {
    snprintf(str, size, "Unknown: 0x%X", scancode);
    return;
  }

  strncpy(str, map[scancode], size - 1);
  str[size - 1] = '\0';
}

/*check for stack corruption*/
void check_stack() {
  uint32_t canary = 0xDEADBEEF;
  uint32_t *stack_canary = &canary;
  if (*stack_canary != 0xDEADBEEF) {
    viprint("Stack corruption detected\n");
  } else {
    viprint("Stack healthy\n");
  }
}

/*print letter to screen after translation*/
void letter_to_screen(uint8_t scancode) {
  char scancode_ascii[85] = {0};

  switch (scancode) {
  case 0x2A:
  case 0x36:
    shift = 1;
    break;
  case 0xAA:
  case 0xB6:
    shift = 0;
    break;
  case 0x3A:
    caps = !caps;
    break;
  case 0x1D:
    ctrl = 1;
    break;
  case 0x9D:
    ctrl = 0;
    break;
  case 0x38:
    alt = 1;
    break;
  case 0xB8:
    alt = 0;
    break;
  default:
    ascii_converter(scancode, scancode_ascii, sizeof(scancode_ascii));
    viprint(scancode_ascii);
    cprint('\n');
    break;
  }
}

/*debug, esp and check stack health*/
void debug_kb_handler(void) {
  uint32_t esp;
  asm volatile("mov %%esp, %0" : "=r"(esp));
  viprint("Stack pointer: ");
  hexprint(esp);
  viprint("\n");

  check_stack();
}

/*kb handler*/
void keyboard_handler(void) {
  viprint("Keyboard Interrupt\n\n");

  asm volatile("pusha");
  asm volatile("pushf");

  int timeout = 1000;
  while ((inb(KEYBOARD_STATUS_PORT) & 0x02) && --timeout)
    ;
  if (timeout == 0) {
    viprint("Keyboard controller timeout\n");
    goto eoi;
  }

  asm volatile("fxsave %0" : : "m"(fpu_state));

  uint8_t scancode = inb(KEYBOARD_DATA_PORT);

  if (scancode == 0xE0) {
    extended_key = true;
    viprint("Arrow keys");
    goto eoi;
  }

  if (extended_key) {
    switch (scancode) {
    case 0x48:
      cursor_row = (cursor_row > 0) ? cursor_row - 1 : 0;
      break;
    case 0x50:
      cursor_row =
          (cursor_row < SCREEN_HEIGHT - 1) ? cursor_row + 1 : SCREEN_HEIGHT - 1;
      break;
    case 0x4B:
      cursor_col = (cursor_col > 0) ? cursor_col - 1 : 0;
      break;
    case 0x4D:
      cursor_col =
          (cursor_col < SCREEN_WIDTH - 1) ? cursor_col + 1 : SCREEN_WIDTH - 1;
      break;
    default:
      printf("Unknown extended scancode: %X\n", scancode);
      break;
    }
    set_cursor_offset(get_offset(cursor_col, cursor_row));
    extended_key = false;
  } else if (scancode == 0x0E && buffer_index > 0) {
    buffer_index--;
    cprint('\b');
  } else if (!(scancode & 0x80)) {
    if (buffer_index >= BUFFER_SIZE - 1) {
      viprint("Keyboard buffer overflow\n");
      goto eoi;
    }
    letter_to_screen(scancode);
    keyboard_buffer[buffer_index++] = scancode;
  }

eoi:
  outb(0x20, 0x20);

  // debug_kb_handler();

  asm volatile("fxrstor %0" : : "m"(fpu_state));
  asm volatile("popf");
  asm volatile("popa");
  asm volatile("iret");
}

/*set the layout for the kb*/
void set_keyboard_layout(keyboard_layout layout) {
  current_layout = layout;
  has_loaded();
  switch (layout) {
  case QWERTY:
    viprint("Keyboard Layout: QWERTY\n");
    break;
  case AZERTY:
    viprint("Keyboard Layout: AZERTY\n");
    break;
  case DVORAK:
    viprint("Keyboard Layout: DVORAK\n");
    break;
  }
}
