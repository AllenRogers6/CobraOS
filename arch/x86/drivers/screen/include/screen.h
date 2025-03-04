#ifndef SCREEN_H
#define SCREEN_H

#include "stdint.h"

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

enum vga_color {
  VGA_COLOR_BLACK = 0,
  VGA_COLOR_BLUE = 1,
  VGA_COLOR_GREEN = 2,
  VGA_COLOR_CYAN = 3,
  VGA_COLOR_RED = 4,
  VGA_COLOR_MAGENTA = 5,
  VGA_COLOR_BROWN = 6,
  VGA_COLOR_LIGHT_GREY = 7,
  VGA_COLOR_DARK_GREY = 8,
  VGA_COLOR_LIGHT_BLUE = 9,
  VGA_COLOR_LIGHT_GREEN = 10,
  VGA_COLOR_LIGHT_CYAN = 11,
  VGA_COLOR_LIGHT_RED = 12,
  VGA_COLOR_LIGHT_MAGENTA = 13,
  VGA_COLOR_LIGHT_BROWN = 14,
  VGA_COLOR_WHITE = 15,
};

void set_text_color(enum vga_color fg, enum vga_color bg);
void init_screen(void);
void set_cursor_offset(int offset);
int get_offset(int col, int row);
void cprint(char c);
void viprint(const char *data);
void scroll();
void clear_screen();
void hexprint(uint32_t value);
void has_loaded();

#endif // !SCREEN_H
