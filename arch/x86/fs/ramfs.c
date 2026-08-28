#include "ramfs.h"
#include "heap.h"
#include "pmm.h"
#include "screen.h"
#include <stddef.h>
#include <string.h>

#ifdef SMP
typedef struct {
  volatile int locked;
} spinlock_t;
#define SPINLOCK_INIT {0}
static inline void spinlock_acquire(spinlock_t *lk) {
  while (__sync_lock_test_and_set(&lk->locked, 1)) {
  }
}
static inline void spinlock_release(spinlock_t *lk) {
  __sync_lock_release(&lk->locked);
}
static spinlock_t ramfs_lock = SPINLOCK_INIT;
#define RAMFS_LOCK() spinlock_acquire(&ramfs_lock)
#define RAMFS_UNLOCK() spinlock_release(&ramfs_lock)
#else
#define RAMFS_LOCK()
#define RAMFS_UNLOCK()
#endif

static int default_read(vfs_node_t *node, uint32_t offset, uint32_t size,
                        uint8_t *buffer);
static int default_write(vfs_node_t *node, uint32_t offset, uint32_t size,
                         const uint8_t *buffer);
static vfs_node_t *default_finddir(vfs_node_t *node, const char *name);

static vfs_node_t *ramfs_root = NULL;
static vfs_node_t *ramfs_cwd = NULL;

static vfs_node_t *create_vfs_node(const char *name, uint32_t type) {
  vfs_node_t *node = (vfs_node_t *)kmalloc(sizeof(vfs_node_t));
  if (!node)
    return NULL;

  memset(node, 0, sizeof(vfs_node_t));
  strncpy(node->name, name, VFS_NAME_MAX);
  node->name[VFS_NAME_MAX] = '\0';
  node->type = type;
  node->size = 0;
  node->data = NULL;
  node->parent = NULL;
  node->children = NULL;
  node->next = NULL;

  node->read = default_read;
  node->write = default_write;
  node->finddir = default_finddir;

  return node;
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

static int default_read(vfs_node_t *node, uint32_t offset, uint32_t size,
                        uint8_t *buffer) {
  if (!node || !(node->type & FS_FILE))
    return -1;
  if (offset >= node->size)
    return 0;
  if (offset + size > node->size)
    size = node->size - offset;
  if (size == 0)
    return 0;

  memcpy(buffer, node->data + offset, size);
  return (int)size;
}

static int default_write(vfs_node_t *node, uint32_t offset, uint32_t size,
                         const uint8_t *buffer) {
  if (!node || !(node->type & FS_FILE))
    return -1;

  uint32_t new_size = offset + size;
  if (new_size > node->size) {
    // Expand data buffer using the heap allocator
    uint8_t *new_data = (uint8_t *)kmalloc(new_size);
    if (!new_data)
      return -1;

    if (node->data) {
      memcpy(new_data, node->data, node->size);
      kfree(node->data);
    }
    // Zero out the newly allocated tail
    memset(new_data + node->size, 0, new_size - node->size);

    node->data = new_data;
    node->size = new_size;
  }

  memcpy(node->data + offset, buffer, size);
  return (int)size;
}

static vfs_node_t *default_finddir(vfs_node_t *node, const char *name) {
  if (!node || !(node->type & FS_DIRECTORY))
    return NULL;

  vfs_node_t *child = node->children;
  while (child) {
    if (strcmp(child->name, name) == 0) {
      return child;
    }
    child = child->next;
  }
  return NULL;
}

static void add_child(vfs_node_t *parent, vfs_node_t *child) {
  child->parent = parent;
  child->next = parent->children;
  parent->children = child;
}

static void remove_child(vfs_node_t *child) {
  vfs_node_t *parent = child->parent;
  if (!parent)
    return;

  vfs_node_t *cur = parent->children;
  vfs_node_t *prev = NULL;

  while (cur) {
    if (cur == child) {
      if (prev)
        prev->next = cur->next;
      else
        parent->children = cur->next;
      child->parent = NULL;
      child->next = NULL;
      return;
    }
    prev = cur;
    cur = cur->next;
  }
}

void ramfs_init(void) {
  if (ramfs_root)
    return;

  ramfs_root = create_vfs_node("/", FS_DIRECTORY);
  viprint("RAMFS: root node created\n");
}

vfs_node_t *ramfs_get_root(void) { return ramfs_root; }

int ramfs_create(vfs_node_t *parent, const char *name, uint32_t type) {
  RAMFS_LOCK();

  if (!parent || !(parent->type & FS_DIRECTORY)) {
    RAMFS_UNLOCK();
    return -1;
  }
  if (ramfs_find(parent, name)) {
    RAMFS_UNLOCK();
    return -2;
  }

  vfs_node_t *new_node = create_vfs_node(name, type);
  if (!new_node) {
    RAMFS_UNLOCK();
    return -3;
  }

  add_child(parent, new_node);
  RAMFS_UNLOCK();
  return 0;
}

int ramfs_delete(vfs_node_t *node) {
  RAMFS_LOCK();

  if (!node || node == ramfs_root) {
    RAMFS_UNLOCK();
    return -1;
  }

  if (node->type & FS_DIRECTORY) {
    if (node->children) {
      RAMFS_UNLOCK();
      return -2; // not empty
    }
  }

  if (node->data) {
    kfree(node->data);
    node->data = NULL;
  }

  remove_child(node);
  kfree(node);

  RAMFS_UNLOCK();
  return 0;
}

vfs_node_t *ramfs_find(vfs_node_t *dir, const char *name) {
  if (!dir)
    return NULL;
  return dir->finddir(dir, name);
}

vfs_file_t *ramfs_open(vfs_node_t *node, uint32_t flags) {
  if (!node || !(node->type & FS_FILE))
    return NULL;

  vfs_file_t *file = (vfs_file_t *)kmalloc(sizeof(vfs_file_t));
  if (!file)
    return NULL;

  file->node = node;
  file->flags = flags;
  file->position = 0;

  if (flags & O_APPEND) {
    file->position = node->size;
  }
  return file;
}

void ramfs_close(vfs_file_t *file) {
  if (file)
    kfree(file);
}

int ramfs_read(vfs_file_t *file, uint32_t size, uint8_t *buffer) {
  RAMFS_LOCK();

  if (!file || !(file->flags & O_READ)) {
    RAMFS_UNLOCK();
    return -1;
  }

  vfs_node_t *node = file->node;
  int bytes_read = node->read(node, file->position, size, buffer);
  if (bytes_read > 0) {
    file->position += bytes_read;
  }

  RAMFS_UNLOCK();
  return bytes_read;
}

int ramfs_write(vfs_file_t *file, uint32_t size, const uint8_t *buffer) {
  RAMFS_LOCK();

  if (!file || !(file->flags & O_WRITE)) {
    RAMFS_UNLOCK();
    return -1;
  }

  vfs_node_t *node = file->node;
  int bytes_written = node->write(node, file->position, size, buffer);
  if (bytes_written > 0) {
    file->position += bytes_written;
  }

  RAMFS_UNLOCK();
  return bytes_written;
}

vfs_node_t *vfs_resolve_path(vfs_node_t *root, const char *path,
                             vfs_node_t *cwd) {

  if (!root) {
    viprint("vfs_resolve_path: root is NULL\n");
    return NULL;
  }
  if (!path || *path == '\0') {
    viprint("vfs_resolve_path: path empty\n");
    return NULL;
  }

  vfs_node_t *current = (path[0] == '/') ? root : (cwd ? cwd : root);

  const char *p = path;
  while (*p == '/')
    p++;

  if (*p == '\0') {
    viprint("vfs_resolve_path: path is root only\n");
    return current;
  }

  const char *start = p;
  while (*p != '\0') {
    const char *end = p;
    while (*end != '\0' && *end != '/')
      end++;

    size_t len = end - start;

    if (len == 0) {
      start = end + 1;
      p = end + 1;
      continue;
    }

    char token[VFS_NAME_MAX + 1];
    if (len > VFS_NAME_MAX) {
      viprint("vfs_resolve_path: token too long (");
      char lenbuf[16];
      int_to_str((int)len, lenbuf);
      viprint(lenbuf);
      viprint(")\n");
      return NULL;
    }

    memcpy(token, start, len);
    token[len] = '\0';

    if (strcmp(token, ".") == 0) {
      // stay in current
    } else if (strcmp(token, "..") == 0) {
      if (current->parent) {
        current = current->parent;
        viprint("vfs_resolve_path: .. -> '");
        viprint(current->name);
        viprint("'\n");
      } else {
        viprint("vfs_resolve_path: .. but no parent, stay at root\n");
      }
    } else {
      if (current->finddir) {
        vfs_node_t *child = current->finddir(current, token);
        if (!child) {
          return NULL;
        }
        current = child;
      } else {
        viprint("vfs_resolve_path: current node has no finddir\n");
        return NULL;
      }
    }

    p = end;
    if (*p == '/') {
      p++;
      while (*p == '/')
        p++;
    }
    start = p;
  }

  return current;
}

void ramfs_set_cwd(vfs_node_t *dir) { ramfs_cwd = dir; }

vfs_node_t *ramfs_resolve(const char *path) {
  return vfs_resolve_path(ramfs_get_root(), path, ramfs_cwd);
}
