
#define shift_press
#define capsOn
#define capsOff
#define numOn
#define numOff

/* TODO
 * add an io_wait() for wait between cmds
 *
 *
 * */

unsigned char keymap[128] = {0,    27,  '1', '2',  '3', '4',  '5',  '6',
                             '7',  '8', '9', '0',  '-', '=',  '\b', // Backspace
                             '\t', 'q', 'w', 'e',  'r', 't',  'y',  'u',
                             'i',  'o', 'p', '[',  ']', '\n', // Enter key
                             0,    'a', 's', 'd',  'f', 'g',  'h',  'j',
                             'k',  'l', ';', '\'', '`', 0, // CapsLock
                             '\\', 'z', 'x', 'c',  'v', 'b',  'n',  'm',
                             ',',  '.', '/', 0,    '*', 0, // Shift
                             ' ',  0,   0,   0,    0,   0,    0,    0,
                             0,    0,   0,   0,    0,   0, // Spacebar
                             0,    0,   0,   0,    0,   0,    0,    0,
                             0,    0,   0,   0,    0}; // F1-F12 keys
