#ifndef EDITOR_H
#define EDITOR_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct Line {
  char *data;
  size_t len;
  size_t cap;
} Line;

typedef struct EditorState {
  Line *lines;
  size_t num_lines;
  size_t cursor_x;
  size_t cursor_y;
  size_t scroll_x;
  size_t scroll_y;
} EditorState;

typedef struct Editor {
  Line *lines;
  size_t num_lines;

  size_t cursor_x;
  size_t cursor_y;

  size_t scroll_x;
  size_t scroll_y;

  char *filename;

  bool modified;
  bool file_ends_with_newline;

  EditorState *undo_stack;
  int undo_top;

  EditorState *redo_stack;
  int redo_top;
} Editor;

typedef enum {
  KEY_UP,
  KEY_DOWN,
  KEY_LEFT,
  KEY_RIGHT,
  KEY_HOME,
  KEY_END,
  KEY_PAGEUP,
  KEY_PAGEDOWN,
  KEY_DELETE,
  KEY_BACKSPACE,
  KEY_ENTER,
  KEY_TAB,
  KEY_SAVE,
  KEY_QUIT,
  KEY_ESC
} SpecialKey;

void editor_init(Editor *ed);
void editor_free(Editor *ed);
bool editor_load(Editor *ed, const char *path);
bool editor_save(Editor *ed);
void editor_insert_char(Editor *ed, char c);
void editor_insert_newline(Editor *ed);
void editor_delete_backward(Editor *ed);
void editor_delete_forward(Editor *ed);
void editor_move_cursor(Editor *ed, SpecialKey key);
void editor_scroll_to_cursor(Editor *ed);
void editor_render(Editor *ed);
void editor_run(Editor *ed);
char *my_strdup(const char *s);

#endif
