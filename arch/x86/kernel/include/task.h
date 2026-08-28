#ifndef TASK_H
#define TASK_H

#include <stdint.h>

typedef enum {
  TASK_RUNNING,
  TASK_READY,
  TASK_BLOCKED,
  TASK_SLEEPING,
  TASK_TERMINATED
} task_state_t;

typedef struct task {
  uint32_t esp;
  uint32_t page_dir;
  uint8_t *stack;
  task_state_t state;
  uint32_t pid;
  struct task *next;
} task_t;

extern task_t *current_task;

void tasking_init(void);
task_t *task_create(void (*entry)(void), uint32_t page_dir);
void schedule(void);
void task_exit(void);

#endif
