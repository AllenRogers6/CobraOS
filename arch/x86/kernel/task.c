#include "task.h"
#include "heap.h"
#include "screen.h"
#include "string.h"

static task_t *task_queue = NULL;
task_t *current_task = NULL;
static uint32_t next_pid = 1;

extern void task_switch(task_t *next);

static void task_reap_terminated(void) {
  task_t **pp = &task_queue;

  while (*pp) {
    task_t *t = *pp;

    if (t->state == TASK_TERMINATED && t != current_task) {
      *pp = t->next;
      t->next = NULL;

      if (t->stack) {
        kfree(t->stack);
      }

      kfree(t);
    } else {
      pp = &t->next;
    }
  }
}

void tasking_init(void) {
  current_task = (task_t *)kmalloc(sizeof(task_t));

  if (!current_task) {
    viprint("Failed to allocate initial task!\n");
    while (1) {
      asm volatile("hlt");
    }
  }

  memset(current_task, 0, sizeof(task_t));
  current_task->state = TASK_RUNNING;
  current_task->pid = next_pid++;
  current_task->stack = NULL;

  asm volatile("mov %%cr3, %0" : "=r"(current_task->page_dir));

  task_queue = current_task;
  viprint("Tasking initialized.\n");
}

task_t *task_create(void (*entry)(void), uint32_t page_dir) {
  task_t *task = (task_t *)kmalloc(sizeof(task_t));
  if (!task)
    return NULL;

  memset(task, 0, sizeof(task_t));

  uint8_t *stack = (uint8_t *)kmalloc(4096);
  if (!stack) {
    kfree(task);
    return NULL;
  }

  task->stack = stack;
  task->state = TASK_READY;
  task->pid = next_pid++;
  task->page_dir = page_dir;

  uint32_t *esp = (uint32_t *)(stack + 4096);

  *--esp = (uint32_t)entry;
  *--esp = 0x202;
  *--esp = 0;
  *--esp = 0;
  *--esp = 0;
  *--esp = 0;
  *--esp = 0;
  *--esp = 0;
  *--esp = 0;
  *--esp = 0;

  task->esp = (uint32_t)esp;

  if (!task_queue) {
    task_queue = task;
  } else {
    task_t *t = task_queue;
    while (t->next)
      t = t->next;
    t->next = task;
  }

  return task;
}

void schedule(void) {
  if (!current_task)
    return;

  task_reap_terminated();

  task_t *next = current_task->next ? current_task->next : task_queue;

  while (next && next != current_task && next->state != TASK_READY) {
    next = next->next ? next->next : task_queue;
  }

  if (!next || next == current_task || next->state != TASK_READY)
    return;

  task_t *old = current_task;

  next->state = TASK_RUNNING;

  if (old->state == TASK_RUNNING) {
    old->state = TASK_READY;
  }

  task_switch(next);

  current_task->state = TASK_RUNNING;
}

void task_exit(void) {
  if (!current_task)
    return;

  current_task->state = TASK_TERMINATED;

  schedule();

  while (1) {
    asm volatile("hlt");
  }
}
