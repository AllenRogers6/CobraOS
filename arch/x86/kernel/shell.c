#include "shell.h"
#include "heap.h"
#include "keyboard.h"
#include "multiboot.h"
#include "pit.h"
#include "pmm.h"
#include "ramfs.h"
#include "rtc.h"
#include "screen.h"
#include "snake.h"
#include "stdio.h"
#include "string.h"
#include "task.h"
#include <log.h>
#include <stdbool.h>
#include <stdint.h>

#define MAX_CMD_LEN 256
#define HISTORY_SIZE 20
#define ESC '\x1B'
#define PROMPT "> "
#define PROMPT_LEN 2

#define VERSION_MAJOR 0
#define VERSION_MINOR 4

static char cmd_buffer[MAX_CMD_LEN];
static int cmd_pos = 0;
static int cmd_len = 0;

static char history[HISTORY_SIZE][MAX_CMD_LEN];
static int history_count = 0;
static int history_index = -1;

typedef enum { ESC_NONE, ESC_START, ESC_BRACKET } esc_state_t;
static esc_state_t esc_state = ESC_NONE;
static char esc_buf[8];
static int esc_pos = 0;

static void shell_process_line(const char *line);
static void shell_redraw_line(void);
static void shell_add_history(const char *cmd);
static void shell_insert_char(char c);
static void shell_delete_char_before(void);
static void shell_delete_char_at(void);
static void shell_move_left(void);
static void shell_move_right(void);
static void shell_move_home(void);
static void shell_move_end(void);
static void shell_handle_escape(char final);

static void cmd_help(void);
static void cmd_clear(void);
static void cmd_echo(const char *args);
static void cmd_ticks(void);
static void cmd_uptime(void);
static void cmd_version(void);
static void cmd_shutdown(void);
static void cmd_reboot(void);
static void cmd_memmap(void);
static void cmd_alloc(void);
static void cmd_pfree(char *args);
static void cmd_freecount(void);
static void cmd_history(void);
static void cmd_paging(void);
static void cmd_kalloc(char *args);
static void cmd_kfree(char *args);
static void cmd_kheapinfo(void);
static void cmd_tasks(void);
static void cmd_ls(const char *args);
static void cmd_cat(const char *args);
static void cmd_mkdir(const char *args);
static void cmd_rm(const char *args);
static void cmd_cd(const char *args);
static void cmd_pwd(void);
static void cmd_sn(const char *args);
static void cmd_touch(const char *args);
static void cmd_date(void);
static void cmd_time(void);

static vfs_node_t *shell_cwd = NULL;

static void print_uint(uint32_t n) {
  char buf[12];
  int i = sizeof(buf) - 1;
  buf[i] = '\0';
  if (n == 0) {
    buf[--i] = '0';
  } else {
    while (n > 0) {
      buf[--i] = '0' + (n % 10);
      n /= 10;
    }
  }
  viprint(&buf[i]);
}

static bool parse_hex(const char *s, uint32_t *out) {
  if (!s || !*s)
    return false;
  if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
    s += 2;
  if (!*s)
    return false;
  uint32_t val = 0;
  for (; *s; s++) {
    val <<= 4;
    if (*s >= '0' && *s <= '9')
      val |= *s - '0';
    else if (*s >= 'a' && *s <= 'f')
      val |= *s - 'a' + 10;
    else if (*s >= 'A' && *s <= 'F')
      val |= *s - 'A' + 10;
    else {
      viprint("Error: invalid hex character.\n");
      return false;
    }
  }
  *out = val;
  return true;
}

static inline uint32_t read_eflags(void) {
  uint32_t eflags;
  asm volatile("pushfl; pop %0" : "=r"(eflags));
  return eflags;
}

#define IF_FLAG (1 << 9)

void debug_int_state(const char *msg) {
  if (read_eflags() & IF_FLAG) {
    viprint("%s: interrupts ENABLED\n");
    viprint(msg);
  } else {
    viprint("%s: interrupts DISABLED\n");
    viprint(msg);
  }
}

static bool parse_dec(const char *s, size_t *out) {
  if (!s || !*s)
    return false;
  size_t val = 0;
  for (; *s; s++) {
    if (*s < '0' || *s > '9') {
      viprint("Error: expected decimal number.\n");
      return false;
    }
    val = val * 10 + (*s - '0');
  }
  *out = val;
  return true;
}

static void print_path(vfs_node_t *node) {
  if (!node || !node->parent) {
    viprint("/");
    return;
  }

  print_path(node->parent);

  if (node->parent->parent != NULL)
    viprint("/");
  viprint(node->name);
}

void shell_init(void) {
  cmd_len = 0;
  cmd_pos = 0;
  history_index = -1;
  esc_state = ESC_NONE;
  shell_cwd = ramfs_get_root();
  if (shell_cwd)
    ramfs_set_cwd(shell_cwd);

  clear_screen();
  viprint("\nCobraOS Shell v");
  print_uint(VERSION_MAJOR);
  cprint('.');
  print_uint(VERSION_MINOR);
  viprint("\nType 'help' for available commands.\n");
  viprint(PROMPT);
}

void shell_feed_char(char c) {

  if (esc_state != ESC_NONE) {
    esc_buf[esc_pos++] = c;

    if (esc_state == ESC_START) {
      if (c == '[') {
        esc_state = ESC_BRACKET;
        return;
      }
      esc_state = ESC_NONE;
      esc_pos = 0;
      return;
    }

    if (esc_state == ESC_BRACKET) {
      if ((c >= 'A' && c <= 'D') || c == 'H' || c == 'F') {
        shell_handle_escape(c);
        esc_state = ESC_NONE;
        esc_pos = 0;
        return;
      }
      if (c == '~') {
        shell_handle_escape('~');
        esc_state = ESC_NONE;
        esc_pos = 0;
        return;
      }
      if (esc_pos < (int)(sizeof(esc_buf) - 1))
        return;
      esc_state = ESC_NONE;
      esc_pos = 0;
      return;
    }
  }

  if (c == ESC) {
    esc_state = ESC_START;
    esc_pos = 0;
    esc_buf[esc_pos++] = ESC;
    return;
  }

  if (c == '\n') {
    viprint("\n");
    if (cmd_len > 0) {
      cmd_buffer[cmd_len] = '\0';
      shell_add_history(cmd_buffer);
      shell_process_line(cmd_buffer);
    }
    cmd_len = 0;
    cmd_pos = 0;
    history_index = -1;
    viprint(PROMPT);
    return;
  }

  if (c == '\b' || c == 0x7F) {
    shell_delete_char_before();
    return;
  }

  if (c >= 32 && c <= 126) {
    shell_insert_char(c);
  }
}

static void shell_redraw_line(void) {
  int row = get_cursor_row();
  clear_screen_row(row);
  set_cursor_position(0, row);
  viprint(PROMPT);
  for (int i = 0; i < cmd_len; i++)
    cprint(cmd_buffer[i]);
  set_cursor_position(PROMPT_LEN + cmd_pos, row);
}

static void shell_insert_char(char c) {
  if (cmd_len >= MAX_CMD_LEN - 1)
    return;
  for (int i = cmd_len; i > cmd_pos; i--)
    cmd_buffer[i] = cmd_buffer[i - 1];
  cmd_buffer[cmd_pos] = c;
  cmd_len++;
  cmd_pos++;
  shell_redraw_line();
}

static void shell_delete_char_before(void) {
  if (cmd_pos == 0)
    return;
  for (int i = cmd_pos - 1; i < cmd_len - 1; i++)
    cmd_buffer[i] = cmd_buffer[i + 1];
  cmd_len--;
  cmd_pos--;
  shell_redraw_line();
}

static void shell_delete_char_at(void) {
  if (cmd_pos >= cmd_len)
    return;
  for (int i = cmd_pos; i < cmd_len - 1; i++)
    cmd_buffer[i] = cmd_buffer[i + 1];
  cmd_len--;
  shell_redraw_line();
}

static void shell_move_left(void) {
  if (cmd_pos > 0) {
    cmd_pos--;
    shell_redraw_line();
  }
}

static void shell_move_right(void) {
  if (cmd_pos < cmd_len) {
    cmd_pos++;
    shell_redraw_line();
  }
}

static void shell_move_home(void) {
  cmd_pos = 0;
  shell_redraw_line();
}

static void shell_move_end(void) {
  cmd_pos = cmd_len;
  shell_redraw_line();
}

static void shell_handle_escape(char final) {
  switch (final) {
  case 'A':
    if (history_count == 0)
      break;
    if (history_index == -1)
      history_index = history_count - 1;
    else if (history_index > 0)
      history_index--;
    strcpy(cmd_buffer, history[history_index % HISTORY_SIZE]);
    cmd_len = strlen(cmd_buffer);
    cmd_pos = cmd_len;
    shell_redraw_line();
    break;

  case 'B':
    if (history_index == -1)
      break;
    if (history_index < history_count - 1) {
      history_index++;
      strcpy(cmd_buffer, history[history_index % HISTORY_SIZE]);
    } else {
      history_index = -1;
      cmd_buffer[0] = '\0';
    }
    cmd_len = strlen(cmd_buffer);
    cmd_pos = cmd_len;
    shell_redraw_line();
    break;

  case 'C':
    shell_move_right();
    break;
  case 'D':
    shell_move_left();
    break;
  case 'H':
    shell_move_home();
    break;
  case 'F':
    shell_move_end();
    break;

  case '~':
    if (esc_pos >= 3 && esc_buf[2] == '3')
      shell_delete_char_at();
    break;

  default:
    break;
  }
}

static void shell_add_history(const char *cmd) {
  if (history_count > 0 &&
      strcmp(history[(history_count - 1) % HISTORY_SIZE], cmd) == 0)
    return; /* duplicate — skip */
  strcpy(history[history_count % HISTORY_SIZE], cmd);
  history_count++;
}

static void shell_process_line(const char *line) {
  if (!line || line[0] == '\0')
    return;

  char cmd[32] = {0};
  int i = 0;
  const char *args = line;

  while (*args && *args != ' ' && i < 31)
    cmd[i++] = *args++;
  cmd[i] = '\0';

  while (*args == ' ')
    args++;

  if (strcmp(cmd, "help") == 0)
    cmd_help();
  else if (strcmp(cmd, "clear") == 0)
    cmd_clear();
  else if (strcmp(cmd, "echo") == 0)
    cmd_echo(args);
  else if (strcmp(cmd, "ticks") == 0)
    cmd_ticks();
  else if (strcmp(cmd, "uptime") == 0)
    cmd_uptime();
  else if (strcmp(cmd, "version") == 0)
    cmd_version();
  else if (strcmp(cmd, "shutdown") == 0)
    cmd_shutdown();
  else if (strcmp(cmd, "reboot") == 0)
    cmd_reboot();
  else if (strcmp(cmd, "memmap") == 0)
    cmd_memmap();
  else if (strcmp(cmd, "alloc") == 0)
    cmd_alloc();
  else if (strcmp(cmd, "free") == 0)
    cmd_pfree((char *)args);
  else if (strcmp(cmd, "freecount") == 0)
    cmd_freecount();
  else if (strcmp(cmd, "history") == 0)
    cmd_history();
  else if (strcmp(cmd, "paging") == 0)
    cmd_paging();
  else if (strcmp(cmd, "kalloc") == 0)
    cmd_kalloc((char *)args);
  else if (strcmp(cmd, "kfree") == 0)
    cmd_kfree((char *)args);
  else if (strcmp(cmd, "kheapinfo") == 0)
    cmd_kheapinfo();
  else if (strcmp(cmd, "tasks") == 0)
    cmd_tasks();
  else if (strcmp(cmd, "ls") == 0)
    cmd_ls(args);
  else if (strcmp(cmd, "cat") == 0)
    cmd_cat(args);
  else if (strcmp(cmd, "mkdir") == 0)
    cmd_mkdir(args);
  else if (strcmp(cmd, "rm") == 0)
    cmd_rm(args);
  else if (strcmp(cmd, "cd") == 0)
    cmd_cd(args);
  else if (strcmp(cmd, "pwd") == 0)
    cmd_pwd();
  else if (strcmp(cmd, "sn") == 0)
    cmd_sn(args);
  else if (strcmp(cmd, "touch") == 0)
    cmd_touch(args);
  else if (strcmp(cmd, "date") == 0)
    cmd_date();
  else if (strcmp(cmd, "time") == 0)
    cmd_time();

  else {
    viprint("Unknown command: '");
    viprint(cmd);
    viprint("'. Type 'help' for a list.\n");
  }
}

static void cmd_help(void) {
  viprint("CobraOS Shell commands:\n");
  viprint("  help      - Show this message\n");
  viprint("  ls        - List files in current directory\n");
  viprint("  cat       - Print file contents\n");
  viprint("  mkdir     - Create a new directory\n");
  viprint("  rm        - Remove a file\n");
  viprint("  sn        - Open a file with the file editor\n");
  viprint("  cd        - Change directory\n");
  viprint("  version   - Show kernel version\n");
  viprint("  clear     - Clear the screen\n");
  viprint("  date      - Show current date\n");
  viprint("  time      - Show current time\n");
  viprint("  echo ...  - Print arguments to screen\n");
  viprint("  ticks     - Show raw PIT tick count\n");
  viprint("  uptime    - Show uptime in seconds\n");
  viprint("  tasks     - List all tasks and their states\n");
  viprint("  shutdown  - Halt the CPU\n");
  viprint("  reboot    - Reset via keyboard controller\n");
  viprint("  memmap    - Display memory map from bootloader\n");
  viprint("  alloc     - Allocate one physical page (PMM)\n");
  viprint("  free <x>  - Free a physical page at hex address\n");
  viprint("  freecount - Show number of free physical pages\n");
  viprint("  kalloc <n>- Allocate n bytes from kernel heap\n");
  viprint("  kfree <x> - Free a kernel heap pointer at hex address\n");
  viprint("  kheapinfo - Show kernel heap start and current top\n");
  viprint("  paging    - Show CR0 paging status\n");
  viprint("  history   - Show command history\n");
}

static void cmd_clear(void) { clear_screen(); }

static void cmd_echo(const char *args) {
  viprint(args);
  viprint("\n");
}

static void cmd_ticks(void) {
  viprint("Ticks: ");
  print_uint(tick_count);
  viprint("\n");
}

static void cmd_uptime(void) {
  uint32_t seconds = tick_count / 100;
  uint32_t minutes = seconds / 60;
  uint32_t hours = minutes / 60;
  seconds %= 60;
  minutes %= 60;
  viprint("Uptime: ");
  print_uint(hours);
  viprint("h ");
  print_uint(minutes);
  viprint("m ");
  print_uint(seconds);
  viprint("s\n");
}

static void cmd_version(void) {
  viprint("CobraOS kernel v");
  print_uint(VERSION_MAJOR);
  cprint('.');
  print_uint(VERSION_MINOR);
  viprint(" (x86 32-bit, freestanding)\n");
}

static void cmd_shutdown(void) {
  viprint("System halting. Goodbye.\n");
  asm volatile("cli");
  for (;;)
    asm volatile("hlt");
}

static void cmd_reboot(void) {
  viprint("Rebooting...\n");
  asm volatile("cli");
  uint8_t status;
  do {
    asm volatile("inb $0x64, %0" : "=a"(status));
  } while (status & 0x02);
  asm volatile("outb %0, $0x64" : : "a"((uint8_t)0xFE));
  for (;;) {
    struct {
      uint16_t limit;
      uint32_t base;
    } __attribute__((packed)) idt0 = {0, 0};
    asm volatile("lidt %0; int $3" : : "m"(idt0));
  }
}

static void cmd_memmap(void) { print_memory_map(); }

static void cmd_alloc(void) {
  void *page = pmm_alloc_page();
  if (page) {
    viprint("Allocated physical page at: ");
    hexprint((uint32_t)page);
  } else {
    viprint("PMM: no free pages available.\n");
  }
}

static void cmd_pfree(char *args) {
  uint32_t addr;
  if (!parse_hex(args, &addr)) {
    viprint("Usage: free <hex_addr>\n");
    return;
  }
  pmm_free_page((void *)addr);
  viprint("Freed physical page at: ");
  hexprint(addr);
}

static void cmd_freecount(void) {
  viprint("Free physical pages: ");
  print_uint((uint32_t)pmm_get_free_page_count());
  viprint("\n");
}

static void cmd_history(void) {
  if (history_count == 0) {
    viprint("No history yet.\n");
    return;
  }
  int start = (history_count > HISTORY_SIZE) ? history_count - HISTORY_SIZE : 0;
  viprint("Command history:\n");
  for (int i = start; i < history_count; i++) {
    viprint("  ");
    print_uint((uint32_t)(i + 1));
    viprint("  ");
    viprint(history[i % HISTORY_SIZE]);
    viprint("\n");
  }
}

static void cmd_paging(void) {
  uint32_t cr0, cr3;
  asm volatile("mov %%cr0, %0" : "=r"(cr0));
  asm volatile("mov %%cr3, %0" : "=r"(cr3));
  viprint("CR0: ");
  hexprint(cr0);
  viprint("CR3: ");
  hexprint(cr3);
  viprint(cr0 & 0x80000000 ? "Paging: enabled\n" : "Paging: disabled\n");
}

static void cmd_kalloc(char *args) {
  size_t size;
  if (!parse_dec(args, &size)) {
    viprint("Usage: kalloc <decimal_size>\n");
    return;
  }
  void *ptr = kmalloc(size);
  if (ptr) {
    viprint("Allocated ");
    print_uint((uint32_t)size);
    viprint(" bytes at: ");
    hexprint((uint32_t)ptr);
  } else {
    viprint("kmalloc: allocation failed.\n");
  }
}

static void cmd_kfree(char *args) {
  uint32_t addr;
  if (!parse_hex(args, &addr)) {
    viprint("Usage: kfree <hex_addr>\n");
    return;
  }
  kfree((void *)addr);
  viprint("Freed kernel heap pointer: ");
  hexprint(addr);
}

static void cmd_kheapinfo(void) {
  viprint("Heap start:   ");
  hexprint(heap_get_start());
  viprint("Heap current: ");
  hexprint(heap_get_current());
}

static void cmd_tasks(void) {
  extern task_t *current_task;

  extern task_t *task_queue_head_extern __attribute__((weak));

  viprint("PID  STATE\n");
  viprint("---  -----\n");

  task_t *t = current_task;
  if (!t) {
    viprint("No tasks.\n");
    return;
  }

  int shown = 0;
  do {
    print_uint(t->pid);
    viprint("    ");
    switch (t->state) {
    case TASK_RUNNING:
      viprint("RUNNING");
      break;
    case TASK_READY:
      viprint("READY");
      break;
    case TASK_BLOCKED:
      viprint("BLOCKED");
      break;
    default:
      viprint("UNKNOWN");
      break;
    }
    if (t == current_task)
      viprint(" *");
    viprint("\n");
    t = t->next;
    shown++;
  } while (t && t != current_task && shown < 64);
}

static void cmd_ls(const char *args) {
  vfs_node_t *dir;

  if (args && *args) {
    dir = ramfs_resolve(args);
  } else {
    dir = shell_cwd;
  }

  if (!dir) {
    viprint("ls: directory not found.\n");
    return;
  }

  if (!(dir->type & FS_DIRECTORY)) {
    viprint("ls: not a directory.\n");
    return;
  }

  vfs_node_t *child = dir->children;
  if (!child) {
    viprint("(empty)\n");
    return;
  }

  while (child) {
    viprint(child->name);
    if (child->type & FS_DIRECTORY)
      viprint("/");
    viprint("\n");
    child = child->next;
  }
}
static void cmd_cat(const char *args) {
  if (!args || !*args) {
    viprint("Usage: cat <file>\n");
    return;
  }

  vfs_node_t *node = ramfs_resolve(args);
  if (!node) {
    viprint("cat: file not found.\n");
    return;
  }

  if (!(node->type & FS_FILE)) {
    viprint("cat: not a file.\n");
    return;
  }

  vfs_file_t *file = ramfs_open(node, O_READ);
  if (!file) {
    viprint("cat: failed to open file.\n");
    return;
  }

  uint8_t buf[256];
  int n;
  while ((n = ramfs_read(file, sizeof(buf), buf)) > 0) {
    for (int i = 0; i < n; i++)
      cprint(buf[i]);
  }
  viprint("\n");
  ramfs_close(file);
}

static void cmd_mkdir(const char *args) {
  if (!args || !*args) {
    viprint("Usage: mkdir <path>\n");
    return;
  }

  const char *slash = strrchr(args, '/');
  vfs_node_t *parent;
  const char *name;

  if (slash == NULL) {
    parent = shell_cwd;
    name = args;
  } else {
    if (slash == args) {
      parent = ramfs_get_root();
      name = slash + 1;
    } else {
      size_t parent_len = slash - args;
      char parent_path[256];
      if (parent_len >= sizeof(parent_path)) {
        viprint("mkdir: path too long.\n");
        return;
      }
      strncpy(parent_path, args, parent_len);
      parent_path[parent_len] = '\0';
      parent = ramfs_resolve(parent_path);
      name = slash + 1;
    }
  }

  if (!parent) {
    viprint("mkdir: parent directory not found.\n");
    return;
  }

  if (!(parent->type & FS_DIRECTORY)) {
    viprint("mkdir: parent is not a directory.\n");
    return;
  }

  if (*name == '\0') {
    viprint("mkdir: missing directory name.\n");
    return;
  }

  int ret = ramfs_create(parent, name, FS_DIRECTORY);
  if (ret == 0)
    viprint("Directory created.\n");
  else if (ret == -2)
    viprint("mkdir: already exists.\n");
  else
    viprint("mkdir: failed.\n");
}

static void cmd_rm(const char *args) {
  if (!args || !*args) {
    viprint("Usage: rm <path>\n");
    return;
  }

  vfs_node_t *node = ramfs_resolve(args);
  if (!node) {
    viprint("rm: not found.\n");
    return;
  }

  int ret = ramfs_delete(node);
  if (ret == 0)
    viprint("Removed.\n");
  else if (ret == -2)
    viprint("rm: directory not empty.\n");
  else
    viprint("rm: failed.\n");
}

static void cmd_cd(const char *args) {
  vfs_node_t *new_cwd;

  if (!args || !*args || strcmp(args, "/") == 0) {
    new_cwd = ramfs_get_root();
  } else {
    new_cwd = ramfs_resolve(args);
  }

  if (!new_cwd) {
    viprint("cd: directory not found.\n");
    return;
  }

  if (!(new_cwd->type & FS_DIRECTORY)) {
    viprint("cd: not a directory.\n");
    return;
  }

  shell_cwd = new_cwd;
  ramfs_set_cwd(shell_cwd);
}

static void cmd_pwd(void) {
  if (!shell_cwd) {
    viprint("/\n");
    return;
  }
  print_path(shell_cwd);
  viprint("\n");
}

static void cmd_sn(const char *args) {
  if (!args || !*args) {
    viprint("Usage: sn <file-name>\n");
    return;
  }

  while (*args == ' ')
    args++;
  if (*args == '\0') {
    viprint("Usage: sn <file-name>\n");
    return;
  }

  viprint(args);

  char filename[128];
  strncpy(filename, args, sizeof(filename) - 1);
  filename[sizeof(filename) - 1] = '\0';

  Editor ed;

  editor_init(&ed);

  if (!editor_load(&ed, filename)) {
    viprint("Error: could not open file\n");
    editor_free(&ed);
    return;
  }

  editor_run(&ed);
  editor_free(&ed);
}

static void cmd_touch(const char *args) {
  if (!args || !*args) {
    viprint("Usage: touch <file>\n");
    return;
  }

  const char *slash = strrchr(args, '/');
  vfs_node_t *parent;
  const char *name;

  if (slash == NULL) {
    parent = shell_cwd;
    name = args;
  } else {
    if (slash == args) {
      parent = ramfs_get_root();
      name = slash + 1;
    } else {
      size_t parent_len = slash - args;
      char parent_path[256];
      if (parent_len >= sizeof(parent_path)) {
        viprint("touch: path too long.\n");
        return;
      }
      strncpy(parent_path, args, parent_len);
      parent_path[parent_len] = '\0';
      parent = ramfs_resolve(parent_path);
      name = slash + 1;
    }
  }

  if (!parent) {
    viprint("touch: parent directory not found.\n");
    return;
  }

  if (!(parent->type & FS_DIRECTORY)) {
    viprint("touch: parent is not a directory.\n");
    return;
  }

  if (*name == '\0') {
    viprint("touch: missing file name.\n");
    return;
  }

  int ret = ramfs_create(parent, name, FS_FILE);
  if (ret == 0) {
    // File created successfully
  } else if (ret == -2) {
    // File already exists – no error, because touch normally updates timestamps
  } else {
    viprint("touch: failed to create file.\n");
  }
}

static void cmd_date(void) {
  struct rtc_datetime dt;
  rtc_read_datetime(&dt);
  viprint("Date: ");
  print_uint(dt.month);
  viprint("/");
  print_uint(dt.day);
  viprint("/");
  print_uint(dt.year);
  viprint("\n");
}

static void cmd_time(void) {
  struct rtc_datetime dt;
  rtc_read_datetime(&dt);
  viprint("Time: ");
  print_uint(dt.hour);
  viprint(":");
  if (dt.minute < 10)
    viprint("0");
  print_uint(dt.minute);
  viprint(":");
  if (dt.second < 10)
    viprint("0");
  print_uint(dt.second);
  viprint("\n");
}
