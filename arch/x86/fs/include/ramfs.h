#ifndef RAMFS_H
#define RAMFS_H

#include <stddef.h>
#include <stdint.h>

#define FS_FILE 1
#define FS_DIRECTORY 2

#define O_READ 0x1
#define O_WRITE 0x2
#define O_APPEND 0x4

#define VFS_NAME_MAX 255

typedef struct vfs_node {
  char name[VFS_NAME_MAX + 1];
  uint32_t type;
  uint32_t size;
  uint8_t *data;
  struct vfs_node *parent;
  struct vfs_node *children;
  struct vfs_node *next;

  int (*read)(struct vfs_node *node, uint32_t offset, uint32_t size,
              uint8_t *buffer);
  int (*write)(struct vfs_node *node, uint32_t offset, uint32_t size,
               const uint8_t *buffer);
  struct vfs_node *(*finddir)(struct vfs_node *node, const char *name);
} vfs_node_t;

typedef struct vfs_file {
  vfs_node_t *node;
  uint32_t flags;
  uint32_t position;
} vfs_file_t;

void ramfs_init(void);
vfs_node_t *ramfs_get_root(void);
int ramfs_create(vfs_node_t *parent, const char *name, uint32_t type);
int ramfs_delete(vfs_node_t *node);
vfs_node_t *ramfs_find(vfs_node_t *dir, const char *name);
vfs_file_t *ramfs_open(vfs_node_t *node, uint32_t flags);
void ramfs_close(vfs_file_t *file);
int ramfs_read(vfs_file_t *file, uint32_t size, uint8_t *buffer);
int ramfs_write(vfs_file_t *file, uint32_t size, const uint8_t *buffer);
void ramfs_set_cwd(vfs_node_t *dir);
vfs_node_t *ramfs_resolve(const char *path);
vfs_node_t *vfs_resolve_path(vfs_node_t *root, const char *path,
                             vfs_node_t *cwd);

#endif // RAMFS_H
