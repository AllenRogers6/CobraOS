#include "screen.h"
#include "io.h"
#include "string.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SCREEN_CTRL 0x3d4
#define SCREEN_DATA 0x3d5
#define VIDEO_MEMORY 0xB8000
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

static inline uint8_t vgaColorEntry(enum vga_color fg, enum vga_color bg) {
  return fg | bg << 4;
}

static inline uint16_t vgaEntry(unsigned char uc, uint8_t color) {
  return (uint16_t)uc | (uint16_t)color << 8;
}

uint8_t colors;
static int cursor_row = 0;
static int cursor_col = 0;

void initScreen(void) {
  uint16_t *video_memory = (uint16_t *)VIDEO_MEMORY;
  colors = vgaColorEntry(VGA_COLOR_WHITE, VGA_COLOR_BLACK);

  for (size_t y = 0; y < SCREEN_HEIGHT; y++) {
    for (size_t x = 0; x < SCREEN_WIDTH; x++) {
      video_memory[y * SCREEN_WIDTH + x] = vgaEntry(' ', colors);
    }
  }

  cursor_row = 0;
  cursor_col = 0;
  setCursorOffset(getOffset(0, 0));
}

void setCursorOffset(int offset) {
  offset /= 2;
  cursor_row = offset / SCREEN_WIDTH;
  cursor_col = offset % SCREEN_WIDTH;

  outb(SCREEN_CTRL, 14);
  outb(SCREEN_DATA, (uint8_t)(offset >> 8));
  outb(SCREEN_CTRL, 15);
  outb(SCREEN_DATA, (uint8_t)(offset & 0xff));
}

int getOffset(int col, int row) { return 2 * (row * SCREEN_WIDTH + col); }

void writeChar(char c) {
  if (c == '\n') {
    cursor_row++;
    cursor_col = 0;
  } else {
    uint16_t *video_memory = (uint16_t *)VIDEO_MEMORY;
    video_memory[cursor_row * SCREEN_WIDTH + cursor_col] = vgaEntry(c, colors);
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

  setCursorOffset(getOffset(cursor_col, cursor_row));
}

void writeToScreen(const char *data, size_t size) {
  for (size_t i = 0; i < size; i++) {
    writeChar(data[i]);
  }
}

void writeStrToScreen(const char *data) { writeToScreen(data, strlen(data)); }

void writeSpace() { writeStrToScreen(""); }

void scroll() {
  uint16_t *video_memory = (uint16_t *)VIDEO_MEMORY;
  memmove(video_memory, video_memory + SCREEN_WIDTH,
          (SCREEN_HEIGHT - 1) * SCREEN_WIDTH * 2);

  for (int col = 0; col < SCREEN_WIDTH; ++col) {
    video_memory[(SCREEN_HEIGHT - 1) * SCREEN_WIDTH + col] =
        vgaEntry(' ', colors);
  }
}

void clearScreen() {
  uint16_t *video_memory = (uint16_t *)VIDEO_MEMORY;

  for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
    video_memory[i] = vgaEntry(' ', colors);
  }

  cursor_row = 0;
  cursor_col = 0;
  setCursorOffset(getOffset(0, 0));
}

void updateCursor() {
  uint16_t pos = cursor_row * SCREEN_WIDTH + cursor_col;
  outb(0x0F, 0x3D4);
  outb(pos & 0xFF, 0x3D5);
  outb(0x0E, 0x3D4);
  outb((pos >> 8) & 0xFF, 0x3D5);
}

void writeHex(uint32_t value) {
  char hexDigits[] = "0123456789ABCDEF";
  char hexString[9];
  int i;

  for (i = 7; i >= 0; --i) {
    hexString[i] = hexDigits[value & 0xF];
    value >>= 4;
  }

  hexString[8] = '\0';

  writeStrToScreen("0x");
  writeStrToScreen(hexString);
  writeStrToScreen("\n");
}
