#ifndef SCREEN_H
#define SCREEN_H

#include "stdint.h"

void initScreen(void);
void putChar(char c);
void writeStrToScreen(const char *data);
void writeSpace();
void clearScreen();
void writeChar(char c);
void writeHex(uint32_t value);
void setCursorOffset(int offset);
int getOffset(int col, int row);
void scroll();

#endif // !SCREEN_H
