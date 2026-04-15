#include "screen.h"
#include "io.h"
#include "string.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SCREEN_CTRL 0x3d4
#define SCREEN_DATA 0x3d5
#define VIDEO_MEMORY 0xB8000

static inline uint8_t vga_color_entry(enum vga_color fg, enum vga_color bg) {
  return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
  return (uint16_t)uc | (uint16_t)color << 8;
}

uint8_t colors;
static int cursor_row = 0;
static int cursor_col = 0;

void set_text_color(enum vga_color fg, enum vga_color bg);
void init_screen(void);
void set_cursor_offset(int offset);
int get_offset(int col, int row);
void cprint(char c);
void vprint(const char *data, size_t size);
void viprint(const char *data);
void scroll(void);
void clear_screen(void);
void update_cursor(void);
void hexprint(uint32_t value);
void has_loaded(void);

void set_text_color(enum vga_color fg, enum vga_color bg) {
  colors = vga_color_entry(fg, bg);
}

void init_screen(void) {
  uint16_t *video_memory = (uint16_t *)VIDEO_MEMORY;
  set_text_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);

  for (size_t y = 0; y < SCREEN_HEIGHT; y++) {
    for (size_t x = 0; x < SCREEN_WIDTH; x++) {
      video_memory[y * SCREEN_WIDTH + x] = vga_entry(' ', colors);
    }
  }

  cursor_row = 0;
  cursor_col = 0;
  set_cursor_offset(get_offset(0, 0));
}

void set_cursor_offset(int offset) {
  offset /= 2;
  cursor_row = offset / SCREEN_WIDTH;
  cursor_col = offset % SCREEN_WIDTH;

  outb(SCREEN_CTRL, 14);
  outb(SCREEN_DATA, (uint8_t)(offset >> 8));
  outb(SCREEN_CTRL, 15);
  outb(SCREEN_DATA, (uint8_t)(offset & 0xff));
}

int get_offset(int col, int row) { return 2 * (row * SCREEN_WIDTH + col); }

void cprint(char c) {
  uint16_t *video_memory = (uint16_t *)VIDEO_MEMORY;

  if (c == '\n') {
    cursor_row++;
    cursor_col = 0;
  } else if (c == '\b') {
    if (cursor_col > 0) {
      cursor_col--;
    } else if (cursor_row > 0) {
      cursor_row--;
      cursor_col = SCREEN_WIDTH - 1;
      while (cursor_col > 0 &&
             video_memory[cursor_row * SCREEN_WIDTH + cursor_col] ==
                 vga_entry(' ', colors)) {
        cursor_col--;
      }
    }
    video_memory[cursor_row * SCREEN_WIDTH + cursor_col] =
        vga_entry(' ', colors);
  } else {
    video_memory[cursor_row * SCREEN_WIDTH + cursor_col] = vga_entry(c, colors);
    cursor_col++;

    if (cursor_col >= SCREEN_WIDTH) {
      cursor_col = 0;
      cursor_row++;
    }
  }

  if (cursor_row >= SCREEN_HEIGHT) {
    scroll();
    cursor_row = SCREEN_HEIGHT - 1;
  }
  if (cursor_col >= SCREEN_WIDTH) {
    cursor_col = SCREEN_WIDTH - 1;
  }

  set_cursor_offset(get_offset(cursor_col, cursor_row));
}

void vprint(const char *data, size_t size) {
  for (size_t i = 0; i < size; i++) {
    cprint(data[i]);
  }
}

void viprint(const char *data) { vprint(data, strlen(data)); }

void scroll() {
  uint16_t *video_memory = (uint16_t *)VIDEO_MEMORY;
  memmove(video_memory, video_memory + SCREEN_WIDTH,
          (SCREEN_HEIGHT - 1) * SCREEN_WIDTH * 2);

  for (int col = 0; col < SCREEN_WIDTH; ++col) {
    video_memory[(SCREEN_HEIGHT - 1) * SCREEN_WIDTH + col] =
        vga_entry(' ', colors);
  }
  cursor_row = SCREEN_HEIGHT - 1;
  cursor_col = 0;
}

void clear_screen() {
  uint16_t *video_memory = (uint16_t *)VIDEO_MEMORY;

  for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
    video_memory[i] = vga_entry(' ', colors);
  }

  cursor_row = 0;
  cursor_col = 0;
  set_cursor_offset(get_offset(0, 0));
}

void update_cursor() {
  uint16_t pos = cursor_row * SCREEN_WIDTH + cursor_col;
  outb(0x3D4, 0x0F);
  outb(0x3D5, pos & 0xFF);
  outb(0x3D4, 0x0E);
  outb(0x3D5, (pos >> 8) & 0xFF);
}

void cprint_color(char c, enum vga_color fg, enum vga_color bg) {
  uint8_t old_colors = colors;
  set_text_color(fg, bg);
  cprint(c);
  set_text_color(old_colors >> 4, old_colors & 0x0F); // restore
}

void set_cursor_position(int x, int y) {
  cursor_col = x;
  cursor_row = y;
  set_cursor_offset(get_offset(x, y));
}

void hexprint(uint32_t value) {
  char hexString[11] = "0x00000000";
  for (int i = 9; i >= 2; i--) {
    hexString[i] = "0123456789ABCDEF"[value & 0xF];
    value >>= 4;
  }
  viprint(hexString);
  viprint("\n");
}

void has_loaded() {
  viprint("[ ");
  set_text_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK);
  viprint("yes");
  set_text_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
  viprint(" ] ");
}
