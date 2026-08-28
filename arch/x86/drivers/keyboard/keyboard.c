#include "keyboard.h"
#include "io.h"
#include "key_maps.h"
#include "ps2.h"
#include "registers.h"
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

#define KB_BUFFER_SIZE 256
#define KEYBOARD_STATUS_PORT 0x64
#define KEYBOARD_DATA_PORT 0x60

extern uint32_t __stack_chk_guard;
#define STACK_CANARY __stack_chk_guard

typedef struct {
  bool shift_left, shift_right;
  bool ctrl_left, ctrl_right;
  bool alt_left, alt_right;
  bool gui_left, gui_right;
  bool caps_lock, num_lock, scroll_lock;
  bool extended;
} keyboard_mods_t;

static volatile keyboard_mods_t mods;
static keyboard_layout current_layout = QWERTY;

static volatile char key_buffer[KB_BUFFER_SIZE];
static volatile unsigned int key_head = 0;
static volatile unsigned int key_tail = 0;
static volatile bool buffer_overflow = false;
static volatile bool reboot_requested = false;
static volatile bool repeat_event = false;
static volatile bool key_repeat_enabled = true;
static volatile uint8_t key_states[16];

static fpu_context_t fpu_ctx __attribute__((aligned(16)));

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

static void handle_special_keys(uint8_t scancode, bool extended);
static void handle_cursor_movement(uint8_t scancode);
static void process_character(uint8_t scancode);
static bool buffer_put(char c);
static char buffer_get(void);
static bool buffer_is_full(void);
bool check_stack_integrity(void);
bool fpu_in_use(void);
void fpu_save(fpu_context_t *ctx);
void fpu_restore(fpu_context_t *ctx);

static inline bool shift_active(void) {
  return mods.shift_left || mods.shift_right;
}

static inline bool ctrl_active(void) {
  return mods.ctrl_left || mods.ctrl_right;
}

static inline bool alt_active(void) { return mods.alt_left || mods.alt_right; }

static inline bool gui_active(void) { return mods.gui_left || mods.gui_right; }

static bool key_is_down(uint8_t scancode) {
  uint8_t idx = scancode >> 3;
  uint8_t mask = (uint8_t)(1u << (scancode & 7));
  return (key_states[idx] & mask) != 0;
}

static void key_set_down(uint8_t scancode) {
  key_states[scancode >> 3] |= (uint8_t)(1u << (scancode & 7));
}

static void key_clear_down(uint8_t scancode) {
  key_states[scancode >> 3] &= (uint8_t)~(1u << (scancode & 7));
}

static bool buffer_is_full(void) {
  return ((key_head + 1) % KB_BUFFER_SIZE) == key_tail;
}

static bool buffer_put(char c) {
  unsigned int next = (key_head + 1) % KB_BUFFER_SIZE;
  if (next == key_tail) {
    buffer_overflow = true;
    return false;
  }

  key_buffer[key_head] = c;
  key_head = next;
  return true;
}

static char buffer_get(void) {
  char c = key_buffer[key_tail];
  key_tail = (key_tail + 1) % KB_BUFFER_SIZE;
  return c;
}

bool keyboard_has_char(void) { return key_head != key_tail; }

char keyboard_get_char(void) {
  if (key_head == key_tail)
    return '\0';

  return buffer_get();
}

bool keyboard_consume_reboot_request(void) {
  bool ret = reboot_requested;
  reboot_requested = false;
  return ret;
}

void keyboard_set_key_repeat(bool enabled) { key_repeat_enabled = enabled; }

bool keyboard_get_key_repeat(void) { return key_repeat_enabled; }

void init_keyboard(void) {
  ps2_init();
  set_keyboard_layout(QWERTY);
  outb(0x21, inb(0x21) & 0xFD);

  key_head = key_tail = 0;
  buffer_overflow = false;
  reboot_requested = false;
  repeat_event = false;
  key_repeat_enabled = true;

  mods.shift_left = mods.shift_right = false;
  mods.ctrl_left = mods.ctrl_right = false;
  mods.alt_left = mods.alt_right = false;
  mods.gui_left = mods.gui_right = false;
  mods.caps_lock = mods.num_lock = mods.scroll_lock = false;
  mods.extended = false;

  for (int i = 0; i < (int)(sizeof(key_states) / sizeof(key_states[0])); i++) {
    key_states[i] = 0;
  }

  printf("Keyboard initialized with %s layout\n", layouts[current_layout].name);
}

void set_keyboard_layout(keyboard_layout layout) {
  if (layout >= LAYOUT_COUNT) {
    viprint("Invalid keyboard layout specified");
    return;
  }
  current_layout = layout;
  printf("Keyboard layout set to %s", layouts[layout].name);
}

const char *get_char_from_scancode(uint8_t scancode) {
  if (scancode >= MAX_SCANCODE)
    return NULL;

  bool shift = shift_active();
  bool caps = mods.caps_lock;

  const keymap *map = &layouts[current_layout];
  return (shift ^ caps) ? map->uppercase[scancode] : map->lowercase[scancode];
}

void check_stack(void) {
  uint32_t canary = 0xDEADBEEF;
  uint32_t *stack_canary = &canary;
  if (*stack_canary != 0xDEADBEEF) {
    viprint("Stack corruption detected\n");
  } else {
    viprint("Stack healthy\n");
  }
}

void keyboard_handler(struct registers *r) {
  (void)r;
  if (!check_stack_integrity()) {
    viprint("Stack corruption detected in keyboard handler!");
    return;
  }

  if (fpu_in_use()) {
    fpu_save(&fpu_ctx);
  }

  int timeout = 1000;
  while ((inb(KEYBOARD_STATUS_PORT) & 0x01) == 0 && --timeout) {
  }

  if (timeout == 0) {
    viprint("Keyboard controller timeout\n");
    goto cleanup;
  }

  uint8_t scancode = inb(KEYBOARD_DATA_PORT);

  if (scancode == 0xE0) {
    mods.extended = true;
    goto cleanup;
  }

  if (scancode == 0xE1) {
    mods.extended = false;
    goto cleanup;
  }

  bool extended = mods.extended;
  bool is_release = (scancode & 0x80) != 0;
  uint8_t base = scancode & 0x7F;

  bool was_down = key_is_down(base);
  repeat_event = !is_release && was_down;

  if (!is_release) {
    key_set_down(base);
  } else {
    key_clear_down(base);
  }

  handle_special_keys(scancode, extended);

  if (extended) {
    mods.extended = false;

    if (is_release)
      goto cleanup;

    if (base == 0x1C) {
      if (!repeat_event || key_repeat_enabled)
        buffer_put('\n');
      goto cleanup;
    }

    if (base == 0x53 && ctrl_active() && alt_active() && !repeat_event) {
      reboot_requested = true;
      goto cleanup;
    }

    if (!repeat_event || key_repeat_enabled)
      handle_cursor_movement(base);

    goto cleanup;
  }

  if (base == 0x0E) {
    if (!is_release && (!repeat_event || key_repeat_enabled))
      buffer_put('\b');
    goto cleanup;
  }

  if (is_release)
    goto cleanup;

  process_character(scancode);

cleanup:
  if (!buffer_is_full())
    buffer_overflow = false;

  outb(0x20, 0x20);

  if (fpu_in_use()) {
    fpu_restore(&fpu_ctx);
  }
}

static void handle_special_keys(uint8_t scancode, bool extended) {
  uint8_t base = scancode & 0x7F;
  bool make = !(scancode & 0x80);

  switch (base) {
  case 0x2A:
    mods.shift_left = make;
    break;
  case 0x36:
    mods.shift_right = make;
    break;

  case 0x1D:
    if (extended)
      mods.ctrl_right = make;
    else
      mods.ctrl_left = make;
    break;

  case 0x38:
    if (extended)
      mods.alt_right = make;
    else
      mods.alt_left = make;
    break;

  case 0x5B:
    mods.gui_left = make;
    break;
  case 0x5C:
    mods.gui_right = make;
    break;

  case 0x3A:
    if (make && !repeat_event)
      mods.caps_lock = !mods.caps_lock;
    break;

  case 0x45:
    if (make && !repeat_event)
      mods.num_lock = !mods.num_lock;
    break;

  case 0x46:
    if (make && !repeat_event)
      mods.scroll_lock = !mods.scroll_lock;
    break;

  default:
    break;
  }
}

static void handle_cursor_movement(uint8_t scancode) {
  const char *seq = NULL;

  switch (scancode) {
  case 0x48:
    seq = "\x1B[A";
    break;
  case 0x50:
    seq = "\x1B[B";
    break;
  case 0x4B:
    seq = "\x1B[D";
    break;
  case 0x4D:
    seq = "\x1B[C";
    break;
  case 0x47:
    seq = "\x1B[H";
    break;
  case 0x4F:
    seq = "\x1B[F";
    break;
  case 0x53:
    seq = "\x1B[3~";
    break;
  default:
    return;
  }

  for (const char *p = seq; *p; p++) {
    if (!buffer_put(*p))
      break;
  }
}

static void process_character(uint8_t scancode) {
  const char *ch = get_char_from_scancode(scancode);
  if (!ch || !*ch)
    return;

  if (scancode == 0x1C) {
    if (!repeat_event || key_repeat_enabled)
      buffer_put('\n');
    return;
  }

  if (ch[1] != '\0')
    return;

  char c = ch[0];

  if (ctrl_active() && !alt_active()) {
    if (c >= 'a' && c <= 'z')
      c -= 0x20;

    if (c >= '@' && c <= '_') {
      if (!repeat_event || key_repeat_enabled)
        buffer_put(c - '@');
      return;
    }
  }

  if (!repeat_event || key_repeat_enabled)
    buffer_put(c);
}

bool check_stack_integrity(void) { return true; }

bool fpu_in_use(void) { return false; }

void fpu_save(fpu_context_t *ctx) { (void)ctx; }

void fpu_restore(fpu_context_t *ctx) { (void)ctx; }

void debug_kb_handler(void) {
  uint32_t esp;
  asm volatile("mov %%esp, %0" : "=r"(esp));
  viprint("Stack pointer: ");
  hexprint(esp);
  viprint("\n");
  check_stack();
}

// deprecated
void letter_to_screen(uint8_t scancode) { (void)scancode; }

void ascii_converter(uint8_t scancode, char str[], size_t size) {
  if (size == 0)
    return;

  const char *ch = get_char_from_scancode(scancode);
  if (ch && *ch) {
    str[0] = *ch;
    if (size > 1)
      str[1] = '\0';
  } else {
    str[0] = '\0';
  }
}
