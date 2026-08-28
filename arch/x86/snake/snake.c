#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <heap.h>
#include <keyboard.h>
#include <ramfs.h>
#include <screen.h>
#include <snake.h>

#ifndef SCREEN_WIDTH
#define SCREEN_WIDTH 80
#endif
#ifndef SCREEN_HEIGHT
#define SCREEN_HEIGHT 25
#endif

#define TEXT_HEIGHT (SCREEN_HEIGHT - 1)
#define VGA_MEMORY 0xB8000
#define MAX_UNDO_LEVELS 100
#define MAX_PATH_LEN 256

typedef enum { KEY_CHAR, KEY_SPECIAL } KeyEventType;

typedef struct {
  KeyEventType type;
  char c;
  SpecialKey key;
} KeyEvent;

static inline uint16_t vga_entry(unsigned char c, uint8_t attr) {
  return (uint16_t)c | (uint16_t)attr << 8;
}

void screen_clear(void) {
  uint16_t *video = (uint16_t *)VGA_MEMORY;
  for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++)
    video[i] = vga_entry(' ', 0x07);
  set_cursor_position(0, 0);
}

void screen_putc(int x, int y, char c, uint8_t attr) {
  if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT)
    return;
  uint16_t *video = (uint16_t *)VGA_MEMORY;
  video[y * SCREEN_WIDTH + x] = vga_entry((unsigned char)c, attr);
}

void screen_set_cursor(int x, int y) { set_cursor_position(x, y); }

void screen_print_string(int x, int y, const char *str, uint8_t attr) {
  while (*str) {
    screen_putc(x++, y, *str++, attr);
    if (x >= SCREEN_WIDTH)
      break;
  }
}

static vfs_node_t *resolve_path(const char *path) {
  return ramfs_resolve(path);
}

int fs_read_file(const char *path, char **buf, size_t *len) {
  if (!path || !buf || !len)
    return -1;

  viprint("fs_read_file: path=");
  viprint(path);
  viprint("\n");

  vfs_node_t *node = resolve_path(path);

  if (!node) {
    viprint("Error: fs_read_file: node not found\n");
    return -1;
  }

  if (!(node->type & FS_FILE)) {
    viprint("Error: fs_read_file: node is not a file\n");
    return -1;
  }

  size_t file_size = node->size;

  *buf = kmalloc(file_size + 1);
  if (!*buf) {
    *len = 0;
    return -1;
  }

  if (file_size > 0 && node->data) {
    memcpy(*buf, node->data, file_size);
  }

  (*buf)[file_size] = '\0';
  *len = file_size;

  return 0;
}

int fs_write_file(const char *path, const char *buf, size_t len) {
  vfs_node_t *node = resolve_path(path);
  if (!node) {
    char dir_path[MAX_PATH_LEN];
    char name[VFS_NAME_MAX + 1];
    const char *slash = strrchr(path, '/');
    if (slash) {
      size_t dir_len = slash - path;
      if (dir_len == 0) {
        strcpy(dir_path, "/");
      } else {
        if (strlen(path) >= MAX_PATH_LEN)
          return -1;
        if (slash - path >= MAX_PATH_LEN)
          return -1;
        memcpy(dir_path, path, dir_len);
        dir_path[dir_len] = '\0';
      }
      strncpy(name, slash + 1, VFS_NAME_MAX);
      name[VFS_NAME_MAX] = '\0';
    } else {
      strcpy(dir_path, "/");
      strncpy(name, path, VFS_NAME_MAX);
      name[VFS_NAME_MAX] = '\0';
    }

    vfs_node_t *parent = resolve_path(dir_path);
    if (!parent || !(parent->type & FS_DIRECTORY))
      return -1;

    if (ramfs_create(parent, name, FS_FILE) != 0)
      return -1;
    node = ramfs_find(parent, name);
    if (!node)
      return -1;
  }

  if (!(node->type & FS_FILE))
    return -1;

  if (node->data) {
    kfree(node->data);
    node->data = NULL;
    node->size = 0;
  }

  if (len > 0) {
    uint8_t *new_data = kmalloc(len);
    if (!new_data)
      return -1;
    memcpy(new_data, buf, len);
    node->data = new_data;
    node->size = len;
  }

  return 0;
}

int kbd_read_event(KeyEvent *ev) {
  while (1) {
    while (!keyboard_has_char()) {
      asm volatile("sti; hlt");
    }

    char c = keyboard_get_char();

    if (c == '\x1B') {
      if (!keyboard_has_char()) {
        ev->type = KEY_SPECIAL;
        ev->key = KEY_ESC;
        return 0;
      }
      char c1 = keyboard_get_char();

      if (c1 == '[') {
        int timeout = 100000;
        while (!keyboard_has_char() && timeout-- > 0)
          __asm__ volatile("pause");
        if (!keyboard_has_char())
          continue;

        char c2 = keyboard_get_char();

        if (c2 == 'A') {
          ev->type = KEY_SPECIAL;
          ev->key = KEY_UP;
          return 0;
        }
        if (c2 == 'B') {
          ev->type = KEY_SPECIAL;
          ev->key = KEY_DOWN;
          return 0;
        }
        if (c2 == 'C') {
          ev->type = KEY_SPECIAL;
          ev->key = KEY_RIGHT;
          return 0;
        }
        if (c2 == 'D') {
          ev->type = KEY_SPECIAL;
          ev->key = KEY_LEFT;
          return 0;
        }
        if (c2 == 'H') {
          ev->type = KEY_SPECIAL;
          ev->key = KEY_HOME;
          return 0;
        }
        if (c2 == 'F') {
          ev->type = KEY_SPECIAL;
          ev->key = KEY_END;
          return 0;
        }
        if (c2 >= '1' && c2 <= '9') {
          timeout = 100000;
          while (!keyboard_has_char() && timeout-- > 0)
            __asm__ volatile("pause");
          if (!keyboard_has_char())
            continue;
          char c3 = keyboard_get_char();
          if (c3 == '~') {
            switch (c2) {
            case '3':
              ev->type = KEY_SPECIAL;
              ev->key = KEY_DELETE;
              return 0;
            case '5':
              ev->type = KEY_SPECIAL;
              ev->key = KEY_PAGEUP;
              return 0;
            case '6':
              ev->type = KEY_SPECIAL;
              ev->key = KEY_PAGEDOWN;
              return 0;
            case '1':
            case '7':
              ev->type = KEY_SPECIAL;
              ev->key = KEY_HOME;
              return 0;
            case '4':
            case '8':
              ev->type = KEY_SPECIAL;
              ev->key = KEY_END;
              return 0;
            default:
              continue;
            }
          }
        }
      }
      continue;
    }

    if (c == '\b' || c == 0x7F) {
      ev->type = KEY_SPECIAL;
      ev->key = KEY_BACKSPACE;
      return 0;
    }
    if (c == '\n' || c == '\r') {
      ev->type = KEY_SPECIAL;
      ev->key = KEY_ENTER;
      return 0;
    }
    if (c == '\t') {
      ev->type = KEY_CHAR;
      ev->c = '\t';
      return 0;
    }

    ev->type = KEY_CHAR;
    ev->c = c;
    return 0;
  }
}

static int int_to_str(int value, char *buf) {
  if (value == 0) {
    buf[0] = '0';
    return 1;
  }
  int len = 0;
  char tmp[16];
  while (value > 0) {
    tmp[len++] = '0' + (value % 10);
    value /= 10;
  }
  for (int i = 0; i < len; i++)
    buf[i] = tmp[len - 1 - i];
  return len;
}

char *my_strdup(const char *s) {
  size_t len = strlen(s);
  char *copy = kmalloc(len + 1);
  if (copy)
    memcpy(copy, s, len + 1);
  return copy;
}

static EditorState editor_state_clone(Editor *ed) {
  EditorState st;
  st.num_lines = ed->num_lines;
  st.lines = kmalloc(st.num_lines * sizeof(Line));
  if (!st.lines) {
    st.num_lines = 0;
    return st;
  }

  for (size_t i = 0; i < st.num_lines; i++) {
    Line *src = &ed->lines[i];
    Line *dst = &st.lines[i];
    dst->len = src->len;
    dst->cap = src->len + 1;
    dst->data = kmalloc(dst->cap);
    if (!dst->data) {
      for (size_t j = 0; j < i; j++)
        kfree(st.lines[j].data);
      kfree(st.lines);
      st.lines = NULL;
      st.num_lines = 0;
      return st;
    }
    memcpy(dst->data, src->data, src->len);
    dst->data[dst->len] = '\0';
  }

  st.cursor_x = ed->cursor_x;
  st.cursor_y = ed->cursor_y;
  st.scroll_x = ed->scroll_x;
  st.scroll_y = ed->scroll_y;
  return st;
}

static void editor_state_free(EditorState *st) {
  if (!st || !st->lines)
    return;
  for (size_t i = 0; i < st->num_lines; i++)
    kfree(st->lines[i].data);
  kfree(st->lines);
  st->lines = NULL;
  st->num_lines = 0;
}

static void editor_state_restore(Editor *ed, EditorState *st) {
  for (size_t i = 0; i < ed->num_lines; i++)
    kfree(ed->lines[i].data);
  kfree(ed->lines);

  ed->lines = st->lines;
  ed->num_lines = st->num_lines;
  ed->cursor_x = st->cursor_x;
  ed->cursor_y = st->cursor_y;
  ed->scroll_x = st->scroll_x;
  ed->scroll_y = st->scroll_y;
  ed->modified = true;

  st->lines = NULL;
  st->num_lines = 0;
}

static bool undo_stack_push(EditorState *stack, int *top, EditorState state) {
  if (*top >= MAX_UNDO_LEVELS - 1) {
    // Stack full: drop oldest
    editor_state_free(&stack[0]);
    for (int i = 1; i <= *top; i++) {
      stack[i - 1] = stack[i];
    }
    (*top)--;
  }
  stack[++(*top)] = state;
  return true;
}

static EditorState undo_stack_pop(EditorState *stack, int *top) {
  return stack[(*top)--];
}

static void undo_stack_clear(EditorState *stack, int *top) {
  for (int i = 0; i <= *top; i++) {
    editor_state_free(&stack[i]);
  }
  *top = -1;
}

static bool editor_push_undo(Editor *ed) {
  EditorState st = editor_state_clone(ed);
  if (!st.lines)
    return false;
  undo_stack_push(ed->undo_stack, &ed->undo_top, st);
  return true;
}

static void editor_discard_last_undo(Editor *ed) {
  if (ed->undo_top < 0)
    return;
  EditorState st = undo_stack_pop(ed->undo_stack, &ed->undo_top);
  editor_state_free(&st);
}

static void editor_clear_redo(Editor *ed) {
  undo_stack_clear(ed->redo_stack, &ed->redo_top);
}

static void editor_undo(Editor *ed) {
  if (ed->undo_top < 0)
    return;

  EditorState current = editor_state_clone(ed);
  if (current.lines) {
    undo_stack_push(ed->redo_stack, &ed->redo_top, current);
  }

  EditorState prev = undo_stack_pop(ed->undo_stack, &ed->undo_top);
  editor_state_restore(ed, &prev);
  editor_state_free(&prev);
}

static void editor_redo(Editor *ed) {
  if (ed->redo_top < 0)
    return;

  EditorState current = editor_state_clone(ed);
  if (current.lines) {
    undo_stack_push(ed->undo_stack, &ed->undo_top, current);
  }

  EditorState next = undo_stack_pop(ed->redo_stack, &ed->redo_top);
  editor_state_restore(ed, &next);
  editor_state_free(&next);
}

static bool line_reserve(Line *line, size_t needed) {
  if (line->cap >= needed)
    return true;
  size_t new_cap = line->cap ? line->cap * 2 : 16;
  while (new_cap < needed)
    new_cap *= 2;
  char *new_data = kmalloc(new_cap);
  if (!new_data)
    return false;
  if (line->data) {
    memcpy(new_data, line->data, line->len + 1);
    kfree(line->data);
  }
  line->data = new_data;
  line->cap = new_cap;
  return true;
}

static bool line_insert_char(Line *line, size_t pos, char c) {
  if (pos > line->len)
    return false;
  if (!line_reserve(line, line->len + 2))
    return false;
  memmove(&line->data[pos + 1], &line->data[pos], line->len - pos + 1);
  line->data[pos] = c;
  line->len++;
  return true;
}

static bool line_delete_char(Line *line, size_t pos) {
  if (pos >= line->len)
    return false;
  memmove(&line->data[pos], &line->data[pos + 1], line->len - pos);
  line->len--;
  return true;
}

static bool line_split(Line *line, size_t pos, Line *new_line) {
  if (pos > line->len)
    return false;
  size_t tail_len = line->len - pos;
  new_line->data = NULL;
  new_line->len = 0;
  new_line->cap = 0;
  if (!line_reserve(new_line, tail_len + 1))
    return false;
  memcpy(new_line->data, &line->data[pos], tail_len);
  new_line->data[tail_len] = '\0';
  new_line->len = tail_len;
  line->data[pos] = '\0';
  line->len = pos;
  return true;
}

static bool line_join(Line *dest, Line *src) {
  size_t new_len = dest->len + src->len;
  if (!line_reserve(dest, new_len + 1))
    return false;
  memcpy(&dest->data[dest->len], src->data, src->len);
  dest->len = new_len;
  dest->data[dest->len] = '\0';
  return true;
}

void editor_init(Editor *ed) {

  ed->num_lines = 1;

  ed->lines = kmalloc(sizeof(Line));

  if (!ed->lines) {
    ed->num_lines = 0;
    return;
  }

  ed->lines[0].data = kmalloc(1);

  if (!ed->lines[0].data) {
    kfree(ed->lines);
    ed->lines = NULL;
    ed->num_lines = 0;
    return;
  }

  ed->lines[0].data[0] = '\0';
  ed->lines[0].len = 0;
  ed->lines[0].cap = 1;

  ed->cursor_x = 0;
  ed->cursor_y = 0;
  ed->scroll_x = 0;
  ed->scroll_y = 0;

  ed->filename = NULL;
  ed->modified = false;
  ed->file_ends_with_newline = false;

  ed->undo_stack = kmalloc(MAX_UNDO_LEVELS * sizeof(EditorState));

  ed->redo_stack = kmalloc(MAX_UNDO_LEVELS * sizeof(EditorState));

  if (!ed->undo_stack || !ed->redo_stack) {

    if (ed->undo_stack)
      kfree(ed->undo_stack);

    if (ed->redo_stack)
      kfree(ed->redo_stack);

    ed->undo_stack = NULL;
    ed->redo_stack = NULL;
  }

  ed->undo_top = -1;
  ed->redo_top = -1;
}

void editor_free(Editor *ed) {
  if (ed->lines) {
    for (size_t i = 0; i < ed->num_lines; i++)
      kfree(ed->lines[i].data);
    kfree(ed->lines);
  }
  ed->lines = NULL;
  ed->num_lines = 0;

  if (ed->filename)
    kfree(ed->filename);
  ed->filename = NULL;

  if (ed->undo_stack) {
    undo_stack_clear(ed->undo_stack, &ed->undo_top);
    kfree(ed->undo_stack);
    ed->undo_stack = NULL;
  }
  if (ed->redo_stack) {
    undo_stack_clear(ed->redo_stack, &ed->redo_top);
    kfree(ed->redo_stack);
    ed->redo_stack = NULL;
  }

  ed->cursor_x = ed->cursor_y = 0;
  ed->scroll_x = ed->scroll_y = 0;
  ed->modified = false;
  ed->file_ends_with_newline = false;
}

static bool editor_parse_buffer(Editor *ed, const char *buf, size_t len,
                                bool file_ends_with_newline) {
  ed->num_lines = 0;
  ed->lines = NULL;

  size_t start = 0;
  bool ends_with_newline = (len > 0 && buf[len - 1] == '\n');
  size_t end = len;
  if (ends_with_newline)
    end = len - 1;

  for (size_t i = 0; i <= end; i++) {
    if (i == end || buf[i] == '\n') {
      size_t line_len = i - start;
      if (line_len > 0 && buf[line_len - 1] == '\r')
        line_len--;

      Line *new_lines = kmalloc((ed->num_lines + 1) * sizeof(Line));
      if (!new_lines)
        return false;
      if (ed->lines) {
        memcpy(new_lines, ed->lines, ed->num_lines * sizeof(Line));
        kfree(ed->lines);
      }
      ed->lines = new_lines;

      Line *line = &ed->lines[ed->num_lines];
      line->data = kmalloc(line_len + 1);
      if (!line->data)
        return false;
      memcpy(line->data, &buf[start], line_len);
      line->data[line_len] = '\0';
      line->len = line_len;
      line->cap = line_len + 1;

      ed->num_lines++;
      start = i + 1;
    }
  }

  if (ed->num_lines == 0) {
    ed->lines = kmalloc(sizeof(Line));
    if (!ed->lines)
      return false;
    ed->lines[0].data = kmalloc(1);
    if (!ed->lines[0].data) {
      kfree(ed->lines);
      ed->lines = NULL;
      return false;
    }
    ed->lines[0].data[0] = '\0';
    ed->lines[0].len = 0;
    ed->lines[0].cap = 1;
    ed->num_lines = 1;
  }

  ed->file_ends_with_newline = file_ends_with_newline;
  return true;
}

bool editor_load(Editor *ed, const char *path) {
  char *buf;
  size_t len = 0;
  if (fs_read_file(path, &buf, &len) != 0) {
    viprint("Error: fs_read_file failed\n");
    return false;
  }

  Editor new_ed;
  editor_init(&new_ed);

  bool ok =
      editor_parse_buffer(&new_ed, buf, len, (len > 0 && buf[len - 1] == '\n'));
  kfree(buf);

  if (!ok) {
    viprint("Error: editor_parse_buffer failed\n");
    editor_free(&new_ed);
    return false;
  }

  editor_free(ed);
  *ed = new_ed;

  new_ed.lines = NULL;
  new_ed.num_lines = 0;
  new_ed.filename = NULL;
  new_ed.undo_stack = NULL;
  new_ed.redo_stack = NULL;

  ed->filename = my_strdup(path);
  if (!ed->filename)
    ed->filename = NULL;
  ed->modified = false;
  return true;
}

static bool editor_prompt_filename(char *buf, size_t buf_size) {
  screen_clear();
  screen_print_string(0, 0, "Open file: ", 0x0F);
  set_cursor_position(11, 0);

  size_t pos = 0;
  buf[0] = '\0';

  while (1) {
    KeyEvent ev;
    if (kbd_read_event(&ev) != 0)
      continue;

    if (ev.type == KEY_SPECIAL) {
      if (ev.key == KEY_ENTER) {
        buf[pos] = '\0';
        return pos > 0;
      }
      if (ev.key == KEY_ESC)
        return false;
      if (ev.key == KEY_BACKSPACE) {
        if (pos > 0) {
          pos--;
          screen_putc(11 + pos, 0, ' ', 0x0F);
          set_cursor_position(11 + pos, 0);
          buf[pos] = '\0';
        }
      }
      continue;
    }

    if (ev.type == KEY_CHAR) {
      if (ev.c >= 32 && ev.c < 127) {
        if (pos < buf_size - 1) {
          buf[pos++] = ev.c;
          buf[pos] = '\0';
          screen_putc(11 + pos - 1, 0, ev.c, 0x0F);
          set_cursor_position(11 + pos, 0);
        }
      }
    }
  }
}

static bool editor_confirm_save(Editor *ed) {
  screen_clear();
  const char *msg = "Unsaved changes. Save? (Y/N)";
  int x = 0, y = SCREEN_HEIGHT / 2;
  screen_print_string(x, y, msg, 0x0F);
  set_cursor_position(x + strlen(msg), y);

  while (1) {
    KeyEvent ev;
    if (kbd_read_event(&ev) != 0)
      continue;
    if (ev.type == KEY_CHAR) {
      if (ev.c == 'y' || ev.c == 'Y') {
        screen_clear();
        return true;
      }
      if (ev.c == 'n' || ev.c == 'N')
        return false;
    }
  }
}

bool editor_save(Editor *ed) {
  if (!ed->filename)
    return false;

  size_t total = 0;
  for (size_t i = 0; i < ed->num_lines; i++)
    total += ed->lines[i].len + 1;
  if (!ed->file_ends_with_newline)
    total -= 1;

  char *buf = kmalloc(total + 1);
  if (!buf)
    return false;

  size_t pos = 0;
  for (size_t i = 0; i < ed->num_lines; i++) {
    memcpy(&buf[pos], ed->lines[i].data, ed->lines[i].len);
    pos += ed->lines[i].len;
    if (i < ed->num_lines - 1 || ed->file_ends_with_newline)
      buf[pos++] = '\n';
  }
  buf[pos] = '\0';

  int ret = fs_write_file(ed->filename, buf, pos);
  kfree(buf);
  if (ret != 0)
    return false;
  ed->modified = false;
  return true;
}

void editor_scroll_to_cursor(Editor *ed) {
  if (ed->cursor_x < ed->scroll_x)
    ed->scroll_x = ed->cursor_x;
  if (ed->cursor_x >= ed->scroll_x + SCREEN_WIDTH)
    ed->scroll_x = ed->cursor_x - SCREEN_WIDTH + 1;

  if (ed->cursor_y < ed->scroll_y)
    ed->scroll_y = ed->cursor_y;
  if (ed->cursor_y >= ed->scroll_y + TEXT_HEIGHT)
    ed->scroll_y = ed->cursor_y - TEXT_HEIGHT + 1;
}

void editor_move_cursor(Editor *ed, SpecialKey key) {
  switch (key) {
  case KEY_LEFT:
    if (ed->cursor_x > 0)
      ed->cursor_x--;
    else if (ed->cursor_y > 0) {
      ed->cursor_y--;
      ed->cursor_x = ed->lines[ed->cursor_y].len;
    }
    break;
  case KEY_RIGHT:
    if (ed->cursor_x < ed->lines[ed->cursor_y].len)
      ed->cursor_x++;
    else if (ed->cursor_y + 1 < ed->num_lines) {
      ed->cursor_y++;
      ed->cursor_x = 0;
    }
    break;
  case KEY_UP:
    if (ed->cursor_y > 0) {
      ed->cursor_y--;
      if (ed->cursor_x > ed->lines[ed->cursor_y].len)
        ed->cursor_x = ed->lines[ed->cursor_y].len;
    } else
      ed->cursor_x = 0;
    break;
  case KEY_DOWN:
    if (ed->cursor_y + 1 < ed->num_lines) {
      ed->cursor_y++;
      if (ed->cursor_x > ed->lines[ed->cursor_y].len)
        ed->cursor_x = ed->lines[ed->cursor_y].len;
    } else
      ed->cursor_x = ed->lines[ed->cursor_y].len;
    break;
  case KEY_HOME:
    ed->cursor_x = 0;
    break;
  case KEY_END:
    ed->cursor_x = ed->lines[ed->cursor_y].len;
    break;
  case KEY_PAGEUP:
    ed->cursor_y =
        (ed->cursor_y > TEXT_HEIGHT) ? ed->cursor_y - TEXT_HEIGHT : 0;
    if (ed->cursor_x > ed->lines[ed->cursor_y].len)
      ed->cursor_x = ed->lines[ed->cursor_y].len;
    break;
  case KEY_PAGEDOWN:
    ed->cursor_y = (ed->cursor_y + TEXT_HEIGHT < ed->num_lines)
                       ? ed->cursor_y + TEXT_HEIGHT
                       : ed->num_lines - 1;
    if (ed->cursor_x > ed->lines[ed->cursor_y].len)
      ed->cursor_x = ed->lines[ed->cursor_y].len;
    break;
  default:
    break;
  }
  editor_scroll_to_cursor(ed);
}

void editor_insert_char(Editor *ed, char c) {
  bool undo_pushed = editor_push_undo(ed);
  Line *line = &ed->lines[ed->cursor_y];
  if (!line_insert_char(line, ed->cursor_x, c)) {
    if (undo_pushed)
      editor_discard_last_undo(ed);
    return;
  }
  ed->cursor_x++;
  ed->modified = true;
  editor_clear_redo(ed);
  editor_scroll_to_cursor(ed);
}

void editor_insert_newline(Editor *ed) {
  bool undo_pushed = editor_push_undo(ed);
  size_t old_count = ed->num_lines;
  Line *new_lines = kmalloc((old_count + 1) * sizeof(Line));
  if (!new_lines) {
    if (undo_pushed)
      editor_discard_last_undo(ed);
    return;
  }

  Line *current = &ed->lines[ed->cursor_y];
  Line new_line;
  if (!line_split(current, ed->cursor_x, &new_line)) {
    kfree(new_lines);
    if (undo_pushed)
      editor_discard_last_undo(ed);
    return;
  }

  memcpy(new_lines, ed->lines, old_count * sizeof(Line));
  kfree(ed->lines);
  ed->lines = new_lines;

  memmove(&ed->lines[ed->cursor_y + 2], &ed->lines[ed->cursor_y + 1],
          (old_count - ed->cursor_y - 1) * sizeof(Line));

  ed->lines[ed->cursor_y + 1] = new_line;
  ed->num_lines++;

  ed->cursor_y++;
  ed->cursor_x = 0;
  ed->modified = true;
  editor_clear_redo(ed);
  editor_scroll_to_cursor(ed);
}

void editor_delete_backward(Editor *ed) {
  bool undo_pushed = editor_push_undo(ed);
  bool changed = false;

  if (ed->cursor_x > 0) {
    Line *line = &ed->lines[ed->cursor_y];
    if (line_delete_char(line, ed->cursor_x - 1)) {
      ed->cursor_x--;
      changed = true;
    }
  } else if (ed->cursor_y > 0) {
    Line *prev = &ed->lines[ed->cursor_y - 1];
    Line *cur = &ed->lines[ed->cursor_y];
    size_t prev_len = prev->len;
    if (line_join(prev, cur)) {
      kfree(cur->data);
      memmove(&ed->lines[ed->cursor_y], &ed->lines[ed->cursor_y + 1],
              (ed->num_lines - ed->cursor_y - 1) * sizeof(Line));
      ed->num_lines--;
      ed->cursor_y--;
      ed->cursor_x = prev_len;
      changed = true;
    }
  }

  if (changed) {
    ed->modified = true;
    editor_clear_redo(ed);
    editor_scroll_to_cursor(ed);
  } else {
    if (undo_pushed)
      editor_discard_last_undo(ed);
  }
}

void editor_delete_forward(Editor *ed) {
  bool undo_pushed = editor_push_undo(ed);
  bool changed = false;

  Line *line = &ed->lines[ed->cursor_y];
  if (ed->cursor_x < line->len) {
    if (line_delete_char(line, ed->cursor_x))
      changed = true;
  } else if (ed->cursor_y + 1 < ed->num_lines) {
    Line *next = &ed->lines[ed->cursor_y + 1];
    if (line_join(line, next)) {
      kfree(next->data);
      memmove(&ed->lines[ed->cursor_y + 1], &ed->lines[ed->cursor_y + 2],
              (ed->num_lines - ed->cursor_y - 2) * sizeof(Line));
      ed->num_lines--;
      changed = true;
    }
  }

  if (changed) {
    ed->modified = true;
    editor_clear_redo(ed);
    editor_scroll_to_cursor(ed);
  } else {
    if (undo_pushed)
      editor_discard_last_undo(ed);
  }
}

void editor_render(Editor *ed) {
  screen_clear();

  size_t end_y = ed->scroll_y + TEXT_HEIGHT;
  if (end_y > ed->num_lines)
    end_y = ed->num_lines;

  for (size_t y = ed->scroll_y; y < end_y; y++) {
    Line *line = &ed->lines[y];
    size_t screen_y = y - ed->scroll_y;

    size_t start = ed->scroll_x;
    if (start > line->len)
      start = line->len;
    size_t count = line->len - start;
    if (count > SCREEN_WIDTH)
      count = SCREEN_WIDTH;

    for (size_t i = 0; i < count; i++)
      screen_putc(i, screen_y, line->data[start + i], 0x07);
  }

  char status[SCREEN_WIDTH + 1];
  int len = 0;

  const char *name = ed->filename ? ed->filename : "[No Name]";
  while (*name && len < SCREEN_WIDTH)
    status[len++] = *name++;
  if (ed->modified && len < SCREEN_WIDTH)
    status[len++] = '*';

  char numbuf[16];
  int n = int_to_str(ed->cursor_y + 1, numbuf);
  for (int i = 0; i < n && len < SCREEN_WIDTH; i++)
    status[len++] = numbuf[i];
  if (len < SCREEN_WIDTH)
    status[len++] = ':';
  n = int_to_str(ed->cursor_x + 1, numbuf);
  for (int i = 0; i < n && len < SCREEN_WIDTH; i++)
    status[len++] = numbuf[i];

  while (len < SCREEN_WIDTH)
    status[len++] = ' ';
  status[SCREEN_WIDTH] = '\0';

  for (int x = 0; x < SCREEN_WIDTH; x++)
    screen_putc(x, SCREEN_HEIGHT - 1, status[x], 0x70);

  screen_set_cursor(ed->cursor_x - ed->scroll_x, ed->cursor_y - ed->scroll_y);
}

void editor_run(Editor *ed) {
  bool running = true;
  while (running) {
    editor_render(ed);

    KeyEvent ev;
    if (kbd_read_event(&ev) != 0)
      continue;

    if (ev.type == KEY_CHAR) {
      // Ctrl+Z undo, Ctrl+Y redo
      if (ev.c == 26) {
        editor_undo(ed);
      } else if (ev.c == 25) {
        editor_redo(ed);
      }

      // Ctrl+O open file
      if (ev.c == 15) {
        char path[MAX_PATH_LEN];
        if (editor_prompt_filename(path, sizeof(path))) {
          if (!editor_load(ed, path)) {
            screen_clear();
            screen_print_string(0, 0, "Error opening file", 0x0F);
            KeyEvent dummy;
            kbd_read_event(&dummy);
          }
        }
      }

      // Ctrl+Q quit
      if (ev.c == 17) {
        if (ed->modified) {
          bool save_choice = editor_confirm_save(ed);
          if (save_choice) {
            if (!editor_save(ed)) {
              clear_screen();
              screen_print_string(0, 0, "Error saving file", 0x0F);
              KeyEvent dummy;
              kbd_read_event(&dummy);
            } else {
              clear_screen();
              running = false;
            }
          } else {
            clear_screen();
            running = false;
          }
        } else {
          clear_screen();
          running = false;
        }
      } else if (ev.c == 19) { // Ctrl+S save
        if (!editor_save(ed)) {
          screen_clear();
          screen_print_string(0, 0, "Error saving file", 0x0F);
          KeyEvent dummy;
          kbd_read_event(&dummy);
        }
      } else if (ev.c == '\r' || ev.c == '\n') {
        editor_insert_newline(ed);
      } else if (ev.c == '\t') {
        editor_insert_char(ed, '\t');
      } else if (ev.c >= 32 && ev.c < 127) {
        editor_insert_char(ed, ev.c);
      }
    } else if (ev.type == KEY_SPECIAL) {
      switch (ev.key) {
      case KEY_UP:
      case KEY_DOWN:
      case KEY_LEFT:
      case KEY_RIGHT:
      case KEY_HOME:
      case KEY_END:
      case KEY_PAGEUP:
      case KEY_PAGEDOWN:
        editor_move_cursor(ed, ev.key);
        break;
      case KEY_BACKSPACE:
        editor_delete_backward(ed);
        break;
      case KEY_DELETE:
        editor_delete_forward(ed);
        break;
      case KEY_ENTER:
        editor_insert_newline(ed);
        break;
      case KEY_SAVE:
        if (!editor_save(ed)) {
          screen_clear();
          screen_print_string(0, 0, "Error saving file", 0x0F);
          KeyEvent dummy;
          kbd_read_event(&dummy);
        }
        break;
      case KEY_QUIT:
        running = false;
        break;
      case KEY_ESC:
        break;
      default:
        break;
      }
    }
  }
}
