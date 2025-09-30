#include "keyboard.h"
#include "fpu.h"
#include "io.h"
#include "key_maps.h"
#include "ps2.h"
#include "screen.h"
#include "stdio.h"
#include "string.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define VGA_COLOR_WHITE 15
#define VGA_COLOR_BLACK 0

#define BUFFER_SIZE 256
#define KEYBOARD_STATUS_PORT 0x64
#define KEYBOARD_DATA_PORT 0x60

extern uint32_t __stack_chk_guard;
#define STACK_CANARY __stack_chk_guard

typedef struct {
  bool shift;
  bool caps;
  bool ctrl;
  bool alt;
  bool gui;
  bool extended;
} modifier_state;

static volatile modifier_state mods = {0};
static keyboard_layout current_layout = QWERTY;
static char keyboard_buffer[BUFFER_SIZE];
static volatile int buffer_index = 0;
static point cursor_pos = {0, 0};
static volatile bool buffer_overflow = false;

static fpu_context_t fpu_ctx __attribute__((aligned(16)));

static keymap layouts[LAYOUT_COUNT] = {
    [QWERTY] = {.name = "QWERTY",
                .lowercase = {/* Your QWERTY lowercase map */},
                .uppercase = {/* Your QWERTY uppercase map */}},

};

void init_keyboard(void) {

  ps2_init();

  set_keyboard_layout(QWERTY);

  outb(0x21, inb(0x21) & 0xFD);

  printf("Keyboard initialized with %s layout\n", layouts[current_layout].name);
}

const char *get_char_from_scancode(uint8_t scancode) {
  if (scancode >= MAX_SCANCODE)
    return NULL;

  modifier_state local_mods;
  local_mods.shift = mods.shift;
  local_mods.caps = mods.caps;

  const keymap *map = &layouts[current_layout];

  return (local_mods.shift ^ local_mods.caps) ? map->uppercase[scancode]
                                              : map->lowercase[scancode];
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

void keyboard_handler(void) {

  asm volatile("pusha");
  asm volatile("pushf");

  int timeout = 1000;
  while ((inb(KEYBOARD_STATUS_PORT) & 0x02) && --timeout)
    ;
  if (timeout == 0) {
    viprint("Keyboard controller timeout\n");
    goto eoi;
  }

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
    void buffer_add_char(char c) {
      if (buffer_index >= BUFFER_SIZE - 1) {
        buffer_overflow = true;
        return;
      }
      letter_to_screen(scancode);
      keyboard_buffer[buffer_index++] = scancode;
    }

  eoi:
    outb(0x20, 0x20);

    asm volatile("fxrstor %0" : : "m"(fpu_state));
    asm volatile("popf");
    asm volatile("popa");

    keyboard_buffer[buffer_index++] = c;
    keyboard_buffer[buffer_index] = '\0';
  }

  void handle_special_keys(uint8_t scancode) {

    modifier_state local_mods;
    local_mods.shift = mods.shift;
    local_mods.caps = mods.caps;
    local_mods.ctrl = mods.ctrl;
    local_mods.alt = mods.alt;
    local_mods.gui = mods.gui;
    local_mods.extended = mods.extended;

    switch (scancode) {
    case 0x2A:
    case 0x36:
      local_mods.shift = true;
      break;
    case 0xAA:
    case 0xB6:
      local_mods.shift = false;
      break;

    case 0x3A:
      local_mods.caps = !local_mods.caps;
      break;

    case 0x1D:
      local_mods.ctrl = true;
      break;
    case 0x9D:
      local_mods.ctrl = false;
      break;

    case 0x38:
      local_mods.alt = true;
      break;
    case 0xB8:
      local_mods.alt = false;
      break;

    case 0x5B:
    case 0x5C:
      local_mods.gui = true;
      break;
    case 0xDB:
    case 0xDC:
      local_mods.gui = false;
      break;

    case 0xE0:
    case 0xE1:
      local_mods.extended = true;
      break;

    default:
      break;
    }

    mods = local_mods;
  }

  void handle_cursor_movement(uint8_t scancode) {
    if (!mods.extended)
      return;

    point local_cursor = cursor_pos;

    switch (scancode) {
    case 0x48:
      local_cursor.y = (local_cursor.y > 0) ? local_cursor.y - 1 : 0;
      break;
    case 0x50:
      local_cursor.y = (local_cursor.y < SCREEN_HEIGHT - 1) ? local_cursor.y + 1
                                                            : SCREEN_HEIGHT - 1;
      break;
    case 0x4B:
      local_cursor.x = (local_cursor.x > 0) ? local_cursor.x - 1 : 0;
      break;
    case 0x4D:
      local_cursor.x = (local_cursor.x < SCREEN_WIDTH - 1) ? local_cursor.x + 1
                                                           : SCREEN_WIDTH - 1;
      break;
    case 0x47:
      local_cursor.x = 0;
      break;
    case 0x4F:
      local_cursor.x = SCREEN_WIDTH - 1;
      break;
    case 0x49:
      local_cursor.y = (local_cursor.y > SCREEN_HEIGHT / 2)
                           ? local_cursor.y - SCREEN_HEIGHT / 2
                           : 0;
      break;
    case 0x51:
      local_cursor.y = (local_cursor.y < SCREEN_HEIGHT - SCREEN_HEIGHT / 2)
                           ? local_cursor.y + SCREEN_HEIGHT / 2
                           : SCREEN_HEIGHT - 1;
      break;
    default:
      break;
    }

    cursor_pos = local_cursor;
    set_cursor_position(cursor_pos.x, cursor_pos.y);
  }

  void process_character(uint8_t scancode) {
    if (scancode & 0x80)
      return;

    const char *ch = get_char_from_scancode(scancode);
    if (!ch || !*ch)
      return;

    if (mods.ctrl && ch[0] >= '@' && ch[0] <= '_') {
      char ctrl_char = ch[0] - '@';
      buffer_add_char(ctrl_char);
      cprint(ctrl_char, VGA_COLOR_WHITE, VGA_COLOR_BLACK);
      return;
    }

    buffer_add_char(ch[0]);
    cprint(ch[0], VGA_COLOR_WHITE, VGA_COLOR_BLACK);
  }

  void keyboard_handler(void) {

    if (!check_stack_integrity()) {

      viprint("Stack corruption detected in keyboard handler!");

      outb(0x20, 0x20);
      return;
    }

    asm volatile("pusha");
    asm volatile("pushf");

    if (fpu_in_use()) {
      fpu_save(&fpu_ctx);
    }

    uint8_t scancode = inb(KEYBOARD_DATA_PORT);

    handle_special_keys(scancode);

    if (mods.extended) {
      handle_cursor_movement(scancode);
      mods.extended = false;
      goto cleanup;
    }

    if (scancode == 0x0E) {
      if (buffer_index > 0) {
        buffer_index--;
        cprint('\b', VGA_COLOR_WHITE, VGA_COLOR_BLACK);
      }
      goto cleanup;
    }

    process_character(scancode);

  cleanup:

    if (buffer_index < BUFFER_SIZE - 1) {
      buffer_overflow = false;
    }

    outb(0x20, 0x20);

    if (fpu_in_use()) {
      fpu_restore(&fpu_ctx);
    }

    asm volatile("popf");
    asm volatile("popa");
  }

  void set_keyboard_layout(keyboard_layout layout) {
    if (layout >= LAYOUT_COUNT) {
      viprint("Invalid keyboard layout specified");
      return;
    }

    current_layout = layout;
    printf("Keyboard layout set to %s", layouts[layout].name);
  }

  bool check_stack_integrity() { return true; }
