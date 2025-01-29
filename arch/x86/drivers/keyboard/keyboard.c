#include "keyboard.h"
#include "apic.h"
#include "io.h"
#include "screen.h"
#include "stddef.h"
#include "stdint.h"
#include "stdio.h"
#include "string.h"

static int shift = 0;
static int caps = 0;
static KeyboardLayout currentLayout = QWERTY;

static const char *qwerty_lowercase_key_map[] = {
    "ERROR",     "ESC",   "1",        "2",  "3",  "4",      "5",
    "6",         "7",     "8",        "9",  "0",  "-",      "=",
    "Backspace", "Tab",   "q",        "w",  "e",  "r",      "t",
    "y",         "u",     "i",        "o",  "p",  "[",      "]",
    "ENTER",     "LCtrl", "a",        "s",  "d",  "f",      "g",
    "h",         "j",     "k",        "l",  ";",  "'",      "`",
    "LShift",    "\\",    "z",        "x",  "c",  "v",      "b",
    "n",         "m",     ",",        ".",  "/",  "RShift", "Keypad *",
    "LAlt",      "Space", "CapsLock", "F1", "F2", "F3",     "F4",
    "F5",        "F6",    "F7",       "F8", "F9", "F10",    "NumLock",
    "ScrollLock"};
static const char *qwerty_uppercase_key_map[] = {
    "ERROR",     "ESC",   "!",        "@",  "#",  "$",      "%",
    "^",         "&",     "*",        "(",  ")",  "_",      "+",
    "Backspace", "Tab",   "Q",        "W",  "E",  "R",      "T",
    "Y",         "U",     "I",        "O",  "P",  "{",      "}",
    "ENTER",     "LCtrl", "A",        "S",  "D",  "F",      "G",
    "H",         "J",     "K",        "L",  ":",  "\"",     "~",
    "LShift",    "|",     "Z",        "X",  "C",  "V",      "B",
    "N",         "M",     "<",        ">",  "?",  "RShift", "Keypad *",
    "LAlt",      "Space", "CapsLock", "F1", "F2", "F3",     "F4",
    "F5",        "F6",    "F7",       "F8", "F9", "F10",    "NumLock",
    "ScrollLock"};

static const char *azerty_lowercase_key_map[] = {
    "ERROR",     "ESC",   "&",        "é",  "\"", "'",      "(",
    "-",         "è",     "_",        "ç",  "à",  ")",      "=",
    "Backspace", "Tab",   "a",        "z",  "e",  "r",      "t",
    "y",         "u",     "i",        "o",  "p",  "^",      "$",
    "ENTER",     "LCtrl", "q",        "s",  "d",  "f",      "g",
    "h",         "j",     "k",        "l",  "m",  "ù",      "",
    "LShift",    "\\",    "w",        "x",  "c",  "v",      "b",
    "n",         ",",     ";",        ":",  "!",  "RShift", "Keypad *",
    "LAlt",      "Space", "CapsLock", "F1", "F2", "F3",     "F4",
    "F5",        "F6",    "F7",       "F8", "F9", "F10",    "NumLock",
    "ScrollLock"};

static const char *azerty_uppercase_key_map[] = {
    "ERROR",     "ESC",   "1",        "2",  "3",  "4",      "5",
    "6",         "7",     "8",        "9",  "0",  "°",      "+",
    "Backspace", "Tab",   "A",        "Z",  "E",  "R",      "T",
    "Y",         "U",     "I",        "O",  "P",  "¨",      "*",
    "ENTER",     "LCtrl", "Q",        "S",  "D",  "F",      "G",
    "H",         "J",     "K",        "L",  "M",  "%",      "~",
    "LShift",    "|",     "W",        "X",  "C",  "V",      "B",
    "N",         "?",     ".",        "/",  "§",  "RShift", "Keypad *",
    "LAlt",      "Space", "CapsLock", "F1", "F2", "F3",     "F4",
    "F5",        "F6",    "F7",       "F8", "F9", "F10",    "NumLock",
    "ScrollLock"};

static const char *dvorak_lowercase_key_map[] = {
    "ERROR",     "ESC",   "1",        "2",  "3",  "4",      "5",
    "6",         "7",     "8",        "9",  "0",  "[",      "]",
    "Backspace", "Tab",   "'",        ",",  ".",  "p",      "y",
    "f",         "g",     "c",        "r",  "l",  "/",      "=",
    "ENTER",     "LCtrl", "a",        "o",  "e",  "u",      "i",
    "d",         "h",     "t",        "n",  "s",  "-",      "",
    "LShift",    "\\",    ";",        "q",  "j",  "k",      "x",
    "b",         "m",     "w",        "v",  "z",  "RShift", "Keypad *",
    "LAlt",      "Space", "CapsLock", "F1", "F2", "F3",     "F4",
    "F5",        "F6",    "F7",       "F8", "F9", "F10",    "NumLock",
    "ScrollLock"};

static const char *dvorak_uppercase_key_map[] = {
    "ERROR",     "ESC",   "!",        "@",  "#",  "$",      "%",
    "^",         "&",     "*",        "(",  ")",  "{",      "}",
    "Backspace", "Tab",   "\"",       "<",  ">",  "P",      "Y",
    "F",         "G",     "C",        "R",  "L",  "?",      "+",
    "ENTER",     "LCtrl", "A",        "O",  "E",  "U",      "I",
    "D",         "H",     "T",        "N",  "S",  "_",      "~",
    "LShift",    "|",     ":",        "Q",  "J",  "K",      "X",
    "B",         "M",     "W",        "V",  "Z",  "RShift", "Keypad *",
    "LAlt",      "Space", "CapsLock", "F1", "F2", "F3",     "F4",
    "F5",        "F6",    "F7",       "F8", "F9", "F10",    "NumLock",
    "ScrollLock"};

const char **getKeyMap() {
  switch (currentLayout) {
  case QWERTY:
    return shift ? (caps ? qwerty_lowercase_key_map : qwerty_uppercase_key_map)
                 : (caps ? qwerty_uppercase_key_map : qwerty_lowercase_key_map);
  case AZERTY:
    return shift ? (caps ? azerty_lowercase_key_map : azerty_uppercase_key_map)
                 : (caps ? azerty_uppercase_key_map : azerty_lowercase_key_map);
    break;
  case DVORAK:
    return shift ? (caps ? dvorak_lowercase_key_map : dvorak_uppercase_key_map)
                 : (caps ? dvorak_uppercase_key_map : dvorak_lowercase_key_map);
    break;
  default:
    return shift ? (caps ? qwerty_lowercase_key_map : qwerty_uppercase_key_map)
                 : (caps ? qwerty_uppercase_key_map : qwerty_lowercase_key_map);
  }
}

void asciiConverter(uint8_t scancode, char str[], size_t size) {
  const char **map = getKeyMap();
  size_t mapSize =
      sizeof(qwerty_lowercase_key_map) / sizeof(qwerty_lowercase_key_map[0]);

  if (scancode >= 0xE0) {
    snprintf(str, size, "Extended Scancode: 0x%X", scancode);
  } else if (scancode < mapSize) {
    strncpy(str, map[scancode], size - 1);
    str[size - 1] = '\0';
  } else {
    snprintf(str, size, "Invalid Scancode: 0x%X", scancode);
  }
}

void letterToScreen(uint8_t scancode) {
  char scancodeAscii[32];
  if (scancode == 0x2A || scancode == 0x36) {
    shift = 1; // down
  } else if (scancode == 0xAA || scancode == 0xB6) {
    shift = 0; // up
  } else if (scancode == 0x3A) {
    caps = !caps; // caps on
    writeStrToScreen(caps ? "Caps Lock Enabled\n" : "Caps Lock Disabled\n");
  }

  if (scancode >= 0x80) {
    writeStrToScreen("Key up: ");
    scancode -= 0x80;
  } else {
    writeStrToScreen("Key down: ");
  }

  asciiConverter(scancode & 0x7F, scancodeAscii, sizeof(scancodeAscii));
  writeStrToScreen(scancodeAscii);
  writeChar('\n');
}

void keyboardHandler() {
  writeStrToScreen("Keyboard Interrupt\n");
  uint8_t scancode = inb(0x60);
  letterToScreen(scancode);
  lapic_eoi();
}

void setKeyboardLayout(KeyboardLayout layout) {
  currentLayout = layout;
  switch (layout) {
  case QWERTY:
    writeStrToScreen("Keyboard Layout: QWERTY\n");
    break;
  case AZERTY:
    writeStrToScreen("Keyboard Layout: AZERTY\n");
    break;
  case DVORAK:
    writeStrToScreen("Keyboard Layout: DVORAK\n");
    break;
  }
}
