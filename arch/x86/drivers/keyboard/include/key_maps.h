#ifndef KEY_MAPS_H
#define KEY_MAPS_H

#define MAX_SCANCODE 0x58

const char *qwerty_lowercase_key_map[MAX_SCANCODE] = {
    "ERROR",    "ESC",      "1",         "2",        "3",          "4",
    "5",        "6",        "7",         "8",        "9",          "0",
    "-",        "=",        "Backspace", "Tab",      "q",          "w",
    "e",        "r",        "t",         "y",        "u",          "i",
    "o",        "p",        "[",         "]",        "ENTER",      "LCtrl",
    "a",        "s",        "d",         "f",        "g",          "h",
    "j",        "k",        "l",         ";",        "'",          "`",
    "LShift",   "\\",       "z",         "x",        "c",          "v",
    "b",        "n",        "m",         ",",        ".",          "/",
    "RShift",   "Keypad *", "LAlt",      "Space",    "CapsLock",   "F1",
    "F2",       "F3",       "F4",        "F5",       "F6",         "F7",
    "F8",       "F9",       "F10",       "NumLock",  "ScrollLock", "Keypad 7",
    "Keypad 8", "Keypad 9", "Keypad -",  "Keypad 4", "Keypad 5",   "Keypad 6",
    "Keypad +", "Keypad 1", "Keypad 2",  "Keypad 3", "Keypad 0",   "Keypad .",
    "F11",      "F12"};

const char *qwerty_uppercase_key_map[MAX_SCANCODE] = {
    "ERROR",    "ESC",      "!",         "@",        "#",          "$",
    "%",        "^",        "&",         "*",        "(",          ")",
    "_",        "+",        "Backspace", "Tab",      "Q",          "W",
    "E",        "R",        "T",         "Y",        "U",          "I",
    "O",        "P",        "{",         "}",        "ENTER",      "LCtrl",
    "A",        "S",        "D",         "F",        "G",          "H",
    "J",        "K",        "L",         ":",        "\"",         "~",
    "LShift",   "|",        "Z",         "X",        "C",          "V",
    "B",        "N",        "M",         "<",        ">",          "?",
    "RShift",   "Keypad *", "LAlt",      "Space",    "CapsLock",   "F1",
    "F2",       "F3",       "F4",        "F5",       "F6",         "F7",
    "F8",       "F9",       "F10",       "NumLock",  "ScrollLock", "Keypad 7",
    "Keypad 8", "Keypad 9", "Keypad -",  "Keypad 4", "Keypad 5",   "Keypad 6",
    "Keypad +", "Keypad 1", "Keypad 2",  "Keypad 3", "Keypad 0",   "Keypad .",
    "F11",      "F12"};

const char *azerty_lowercase_key_map[MAX_SCANCODE] = {
    "ERROR",    "ESC",      "&",         "é",        "\"",         "'",
    "(",        "-",        "è",         "_",        "ç",          "à",
    ")",        "=",        "Backspace", "Tab",      "a",          "z",
    "e",        "r",        "t",         "y",        "u",          "i",
    "o",        "p",        "^",         "$",        "ENTER",      "LCtrl",
    "q",        "s",        "d",         "f",        "g",          "h",
    "j",        "k",        "l",         "m",        "ù",          "`",
    "LShift",   "*",        "w",         "x",        "c",          "v",
    "b",        "n",        ",",         ";",        ":",          "!",
    "RShift",   "Keypad *", "LAlt",      "Space",    "CapsLock",   "F1",
    "F2",       "F3",       "F4",        "F5",       "F6",         "F7",
    "F8",       "F9",       "F10",       "NumLock",  "ScrollLock", "Keypad 7",
    "Keypad 8", "Keypad 9", "Keypad -",  "Keypad 4", "Keypad 5",   "Keypad 6",
    "Keypad +", "Keypad 1", "Keypad 2",  "Keypad 3", "Keypad 0",   "Keypad .",
    "F11",      "F12"};

const char *azerty_uppercase_key_map[MAX_SCANCODE] = {
    "ERROR",    "ESC",      "1",         "2",        "3",          "4",
    "5",        "6",        "7",         "8",        "9",          "0",
    "°",        "+",        "Backspace", "Tab",      "A",          "Z",
    "E",        "R",        "T",         "Y",        "U",          "I",
    "O",        "P",        "¨",         "£",        "ENTER",      "LCtrl",
    "Q",        "S",        "D",         "F",        "G",          "H",
    "J",        "K",        "L",         "M",        "%",          "~",
    "LShift",   "µ",        "W",         "X",        "C",          "V",
    "B",        "N",        "?",         ".",        "/",          "§",
    "RShift",   "Keypad *", "LAlt",      "Space",    "CapsLock",   "F1",
    "F2",       "F3",       "F4",        "F5",       "F6",         "F7",
    "F8",       "F9",       "F10",       "NumLock",  "ScrollLock", "Keypad 7",
    "Keypad 8", "Keypad 9", "Keypad -",  "Keypad 4", "Keypad 5",   "Keypad 6",
    "Keypad +", "Keypad 1", "Keypad 2",  "Keypad 3", "Keypad 0",   "Keypad .",
    "F11",      "F12"};

const char *dvorak_lowercase_key_map[MAX_SCANCODE] = {
    "ERROR",    "ESC",      "1",         "2",        "3",          "4",
    "5",        "6",        "7",         "8",        "9",          "0",
    "[",        "]",        "Backspace", "Tab",      "'",          ",",
    ".",        "p",        "y",         "f",        "g",          "c",
    "r",        "l",        "/",         "=",        "ENTER",      "LCtrl",
    "a",        "o",        "e",         "u",        "i",          "d",
    "h",        "t",        "n",         "s",        "-",          "`",
    "LShift",   "\\",       ";",         "q",        "j",          "k",
    "x",        "b",        "m",         "w",        "v",          "z",
    "RShift",   "Keypad *", "LAlt",      "Space",    "CapsLock",   "F1",
    "F2",       "F3",       "F4",        "F5",       "F6",         "F7",
    "F8",       "F9",       "F10",       "NumLock",  "ScrollLock", "Keypad 7",
    "Keypad 8", "Keypad 9", "Keypad -",  "Keypad 4", "Keypad 5",   "Keypad 6",
    "Keypad +", "Keypad 1", "Keypad 2",  "Keypad 3", "Keypad 0",   "Keypad .",
    "F11",      "F12"};

const char *dvorak_uppercase_key_map[MAX_SCANCODE] = {
    "ERROR",    "ESC",      "!",         "@",        "#",          "$",
    "%",        "^",        "&",         "*",        "(",          ")",
    "{",        "}",        "Backspace", "Tab",      "\"",         "<",
    ">",        "P",        "Y",         "F",        "G",          "C",
    "R",        "L",        "?",         "+",        "ENTER",      "LCtrl",
    "A",        "O",        "E",         "U",        "I",          "D",
    "H",        "T",        "N",         "S",        "_",          "~",
    "LShift",   "|",        ":",         "Q",        "J",          "K",
    "X",        "B",        "M",         "W",        "V",          "Z",
    "RShift",   "Keypad *", "LAlt",      "Space",    "CapsLock",   "F1",
    "F2",       "F3",       "F4",        "F5",       "F6",         "F7",
    "F8",       "F9",       "F10",       "NumLock",  "ScrollLock", "Keypad 7",
    "Keypad 8", "Keypad 9", "Keypad -",  "Keypad 4", "Keypad 5",   "Keypad 6",
    "Keypad +", "Keypad 1", "Keypad 2",  "Keypad 3", "Keypad 0",   "Keypad .",
    "F11",      "F12"};

const char *fallback_map[MAX_SCANCODE] = {
    "ERROR",    "ESC",      "1",         "2",          "3",        "4",
    "5",        "6",        "7",         "8",          "9",        "0",
    "-",        "=",        "Backspace", "Tab",        "Q",        "W",
    "E",        "R",        "T",         "Y",          "I",        "O",
    "P",        "[",        "]",         "ENTER",      "LCtrl",    "A",
    "S",        "D",        "F",         "G",          "H",        "J",
    "K",        "L",        ";",         "'",          "`",        "LShift",
    "\\",       "Z",        "X",         "C",          "V",        "B",
    "N",        "M",        ",",         ".",          "/",        "RShift",
    "Keypad *", "LAlt",     "Space",     "CapsLock",   "F1",       "F2",
    "F3",       "F4",       "F5",        "F6",         "F7",       "F8",
    "F9",       "F10",      "NumLock",   "ScrollLock", "Keypad 7", "Keypad 8",
    "Keypad 9", "Keypad -", "Keypad 4",  "Keypad 5",   "Keypad 6", "Keypad +",
    "Keypad 1", "Keypad 2", "Keypad 3",  "Keypad 0",   "Keypad .", "Unknown",
    "Unknown",  "Unknown",  "F11",       "F12"};

#endif
