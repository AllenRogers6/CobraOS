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

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define VGA_COLOR_WHITE 15
#define VGA_COLOR_BLACK 0

#define BUFFER_SIZE 256
#define KEYBOARD_STATUS_PORT 0x64
#define KEYBOARD_DATA_PORT 0x60

extern uint32_t __stack_chk_guard;
#define STACK_CANARY __stack_chk_guard

// Modifier state structure (already defined in keyboard.h but we repeat for
// clarity)
typedef struct {
  bool shift;
  bool caps;
  bool ctrl;
  bool alt;
  bool gui;
  bool extended;
} modifier_state;

// Global state
static volatile modifier_state mods = {0};
static keyboard_layout current_layout = QWERTY;
static char keyboard_buffer[BUFFER_SIZE];
static volatile int buffer_index = 0;
static point cursor_pos = {0, 0};
static volatile bool buffer_overflow = false;

// FPU context (definition from keyboard.h)
static fpu_context_t fpu_ctx __attribute__((aligned(16)));

// External declarations (already in key_maps.h, but if not, declare them)
extern const char *qwerty_lowercase_key_map[];
extern const char *qwerty_uppercase_key_map[];
extern const char *azerty_lowercase_key_map[];
extern const char *azerty_uppercase_key_map[];
extern const char *dvorak_lowercase_key_map[];
extern const char *dvorak_uppercase_key_map[];

static keymap layouts[LAYOUT_COUNT] = {
    [QWERTY] = {.name = "QWERTY",
                .lowercase = qwerty_lowercase_key_map,
                .uppercase = qwerty_uppercase_key_map},
    [AZERTY] = {.name = "AZERTY",
                .lowercase = azerty_lowercase_key_map,
                .uppercase = azerty_uppercase_key_map},
    [DVORAK] = {.name = "DVORAK",
                .lowercase = dvorak_lowercase_key_map,
                .uppercase = dvorak_uppercase_key_map}};

/* ---------- Forward declarations ---------- */
static void handle_special_keys(uint8_t scancode);
static void handle_cursor_movement(uint8_t scancode);
static void process_character(uint8_t scancode);
static void buffer_add_char(char c);
bool check_stack_integrity(void);
bool fpu_in_use(void);
void fpu_save(fpu_context_t *ctx);
void fpu_restore(fpu_context_t *ctx);

/* ---------- Initialisation ---------- */
void init_keyboard(void) {
  ps2_init();
  set_keyboard_layout(QWERTY);
  outb(0x21, inb(0x21) & 0xFD);
  printf("Keyboard initialized with %s layout\n", layouts[current_layout].name);
}

/* ---------- Layout management ---------- */
void set_keyboard_layout(keyboard_layout layout) {
  if (layout >= LAYOUT_COUNT) {
    viprint("Invalid keyboard layout specified");
    return;
  }
  current_layout = layout;
  printf("Keyboard layout set to %s", layouts[layout].name);
}

/* ---------- Scancode translation ---------- */
const char *get_char_from_scancode(uint8_t scancode) {
  if (scancode >= MAX_SCANCODE)
    return NULL;

  // Capture modifier state atomically (simplified)
  bool shift = mods.shift;
  bool caps = mods.caps;

  const keymap *map = &layouts[current_layout];
  return (shift ^ caps) ? map->uppercase[scancode] : map->lowercase[scancode];
}

/* ---------- Stack checking (placeholder) ---------- */
void check_stack(void) {
  uint32_t canary = 0xDEADBEEF;
  uint32_t *stack_canary = &canary;
  if (*stack_canary != 0xDEADBEEF) {
    viprint("Stack corruption detected\n");
  } else {
    viprint("Stack healthy\n");
  }
}

/* ---------- Main keyboard interrupt handler ---------- */
void keyboard_handler(void) {
  // Verify stack integrity
  if (!check_stack_integrity()) {
    viprint("Stack corruption detected in keyboard handler!");
    outb(0x20, 0x20);
    return;
  }

  asm volatile("pusha");
  asm volatile("pushf");

  // Save FPU state if needed
  /*if (fpu_in_use()) {
    fpu_save(&fpu_ctx);
  }*/

  // Read scancode with timeout
  int timeout = 1000;
  while ((inb(KEYBOARD_STATUS_PORT) & 0x02) && --timeout) {
  }
  if (timeout == 0) {
    viprint("Keyboard controller timeout\n");
    goto cleanup;
  }

  uint8_t scancode = inb(KEYBOARD_DATA_PORT);

  // Handle extended prefix (E0)
  if (scancode == 0xE0) {
    mods.extended = true;
    goto cleanup;
  }

  // Process modifiers
  handle_special_keys(scancode);

  // Handle extended keys (cursor movement)
  if (mods.extended) {
    handle_cursor_movement(scancode);
    mods.extended = false;
    goto cleanup;
  }

  // Backspace handling
  if (scancode == 0x0E) {
    if (buffer_index > 0) {
      buffer_index--;
      cprint_color('\b', VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    }
    goto cleanup;
  }

  // Normal key press (ignore key releases)
  if (!(scancode & 0x80)) {
    process_character(scancode);
  }

cleanup:
  // Clear buffer overflow flag if space available
  if (buffer_index < BUFFER_SIZE - 1) {
    buffer_overflow = false;
  }

  // Send EOI
  outb(0x20, 0x20);

  // Restore FPU state
  if (fpu_in_use()) {
    fpu_restore(&fpu_ctx);
  }

  asm volatile("popf");
  asm volatile("popa");
}

/* ---------- Helper: modifier key handling ---------- */
static void handle_special_keys(uint8_t scancode) {
  switch (scancode) {
  // Left/Right Shift press
  case 0x2A:
  case 0x36:
    mods.shift = true;
    break;
  // Left/Right Shift release
  case 0xAA:
  case 0xB6:
    mods.shift = false;
    break;

  // Caps Lock toggles on press only
  case 0x3A:
    mods.caps = !mods.caps;
    break;

  // Left/Right Ctrl press
  case 0x1D:
    mods.ctrl = true;
    break;
  // Left/Right Ctrl release
  case 0x9D:
    mods.ctrl = false;
    break;

  // Left/Right Alt press
  case 0x38:
    mods.alt = true;
    break;
  // Left/Right Alt release
  case 0xB8:
    mods.alt = false;
    break;

  // Windows/GUI keys
  case 0x5B:
  case 0x5C:
    mods.gui = true;
    break;
  case 0xDB:
  case 0xDC:
    mods.gui = false;
    break;

  // Extended key prefix is handled separately
  default:
    break;
  }
}

/* ---------- Helper: cursor movement (arrow keys, home, end, pgup, pgdn)
 * ---------- */
static void handle_cursor_movement(uint8_t scancode) {
  point local_cursor = cursor_pos;

  switch (scancode) {
  case 0x48: // Up arrow
    local_cursor.y = (local_cursor.y > 0) ? local_cursor.y - 1 : 0;
    break;
  case 0x50: // Down arrow
    local_cursor.y = (local_cursor.y < SCREEN_HEIGHT - 1) ? local_cursor.y + 1
                                                          : SCREEN_HEIGHT - 1;
    break;
  case 0x4B: // Left arrow
    local_cursor.x = (local_cursor.x > 0) ? local_cursor.x - 1 : 0;
    break;
  case 0x4D: // Right arrow
    local_cursor.x = (local_cursor.x < SCREEN_WIDTH - 1) ? local_cursor.x + 1
                                                         : SCREEN_WIDTH - 1;
    break;
  case 0x47: // Home
    local_cursor.x = 0;
    break;
  case 0x4F: // End
    local_cursor.x = SCREEN_WIDTH - 1;
    break;
  case 0x49: // Page Up
    local_cursor.y = (local_cursor.y > SCREEN_HEIGHT / 2)
                         ? local_cursor.y - SCREEN_HEIGHT / 2
                         : 0;
    break;
  case 0x51: // Page Down
    local_cursor.y = (local_cursor.y < SCREEN_HEIGHT - SCREEN_HEIGHT / 2)
                         ? local_cursor.y + SCREEN_HEIGHT / 2
                         : SCREEN_HEIGHT - 1;
    break;
  default:
    // Unknown extended scancode – ignore
    return;
  }

  cursor_pos = local_cursor;
  set_cursor_position(cursor_pos.x, cursor_pos.y);
}

/* ---------- Helper: normal character processing ---------- */
static void process_character(uint8_t scancode) {
  const char *ch = get_char_from_scancode(scancode);
  if (!ch || !*ch)
    return;

  // Handle Enter key (scancode 0x1C) – output newline
  if (scancode == 0x1C) {
    buffer_add_char('\n');
    cprint_color('\n', VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    return;
  }

  // Only process if the string is exactly one character long
  // (This filters out "LShift", "ESC", "F1", etc.)
  if (ch[1] != '\0')
    return;

  // Handle Ctrl+letter combinations
  if (mods.ctrl && ch[0] >= '@' && ch[0] <= '_') {
    char ctrl_char = ch[0] - '@';
    buffer_add_char(ctrl_char);
    cprint_color(ctrl_char, VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    return;
  }

  // Normal printable character
  buffer_add_char(ch[0]);
  cprint_color(ch[0], VGA_COLOR_WHITE, VGA_COLOR_BLACK);
}
/* ---------- Helper: add character to keyboard buffer ---------- */
static void buffer_add_char(char c) {
  if (buffer_index >= BUFFER_SIZE - 1) {
    buffer_overflow = true;
    return;
  }
  keyboard_buffer[buffer_index++] = c;
  keyboard_buffer[buffer_index] = '\0';
}

/* ---------- Dummy implementations for missing functions ---------- */
// Replace these with actual implementations from your kernel.
bool check_stack_integrity(void) {
  // TODO: Implement real stack canary check
  return true;
}

bool fpu_in_use(void) {
  // TODO: Check CR0.TS or task state segment
  return false;
}

void fpu_save(fpu_context_t *ctx) {
  // TODO: Save FPU state (e.g., fxsave)
  (void)ctx;
}

void fpu_restore(fpu_context_t *ctx) {
  // TODO: Restore FPU state (e.g., fxrstor)
  (void)ctx;
}

/* ---------- Debug function ---------- */
void debug_kb_handler(void) {
  uint32_t esp;
  asm volatile("mov %%esp, %0" : "=r"(esp));
  viprint("Stack pointer: ");
  hexprint(esp);
  viprint("\n");
  check_stack();
}

/* ---------- Legacy function (kept for compatibility) ---------- */
void letter_to_screen(uint8_t scancode) {
  // This function is deprecated; use process_character instead.
  char scancode_ascii[85] = {0};
  switch (scancode) {
  case 0x2A:
  case 0x36:
    mods.shift = true;
    break;
  case 0xAA:
  case 0xB6:
    mods.shift = false;
    break;
  case 0x3A:
    mods.caps = !mods.caps;
    break;
  case 0x1D:
    mods.ctrl = true;
    break;
  case 0x9D:
    mods.ctrl = false;
    break;
  case 0x38:
    mods.alt = true;
    break;
  case 0xB8:
    mods.alt = false;
    break;
  default:
    ascii_converter(scancode, scancode_ascii, sizeof(scancode_ascii));
    viprint(scancode_ascii);
    cprint('\n');
    break;
  }
}

void ascii_converter(uint8_t scancode, char str[], size_t size) {
  const char *ch = get_char_from_scancode(scancode);
  if (ch && *ch) {
    str[0] = *ch;
    str[1] = '\0';
  } else {
    str[0] = '\0';
  }
}
