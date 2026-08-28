#include "task.h"
#include "heap.h"
#include "screen.h"
#include "string.h"

static task_t *task_queue = NULL;
task_t *current_task = NULL;
static uint32_t next_pid = 1;

extern void task_switch(task_t *next);
task_t *next_task = NULL;

static task_t *task_queue_head = NULL;
static task_t *task_queue_tail = NULL;

void tasking_init(void) {
  // Create a task structure for the current kernel execution context.
  current_task = (task_t *)kmalloc(sizeof(task_t));

  if (!current_task) {
    viprint("Failed to allocate initial task!\n");
    while (1)
      ;
  }
  memset(current_task, 0, sizeof(task_t));
  current_task->state = TASK_RUNNING;
  current_task->pid = next_pid++;
  current_task->stack = NULL; // Using the boot stack

  // Get current page directory from CR3
  asm volatile("mov %%cr3, %0" : "=r"(current_task->page_dir));

  task_queue = current_task;
  viprint("Tasking initialized.\n");
}

void enqueue(task_t *t) {
    t->next = NULL;
    t->state = TASK_READY;

    if (!task_queue_head) {
        task_queue_head = task_queue_tail = t;
        return;
    }

    task_queue_tail->next = t;
    task_queue_tail = t;
}

task_t *dequeue(void) {
    if (!task_queue_head)
        return NULL;

    task_t *t = task_queue_head;
    task_queue_head = t->next;

    if (!task_queue_head)
        task_queue_tail = NULL;

    t->next = NULL;
    return t;
}

task_t *task_create(void (*entry)(void), uint32_t page_dir) {
  task_t *task = (task_t *)kmalloc(sizeof(task_t));
  if (!task)
    return NULL;
  memset(task, 0, sizeof(task_t));

  // Allocate a kernel stack for the new task
  uint8_t *stack = (uint8_t *)kmalloc(4096);
  if (!stack) {
    kfree(task);
    return NULL;
  }
  task->stack = stack;
  task->state = TASK_READY;
  task->pid = next_pid++;
  task->page_dir = page_dir;

  // Set up initial stack frame for the task.
  // When the task is switched to, it will pop these registers
  // and then execute `ret` to jump to entry().
  uint32_t *esp = (uint32_t *)(stack + 4096); // Top of stack
  *--esp = (uint32_t)entry;                   // EIP (return address)
  *--esp = 0x202;                             // EFLAGS (interrupt enabled)
  *--esp = 0x08;                              // CS (kernel code segment)
  *--esp = 0;                                 // EAX
  *--esp = 0;                                 // ECX
  *--esp = 0;                                 // EDX
  *--esp = 0;                                 // EBX
  *--esp = 0; // ESP (unused, will be overwritten)
  *--esp = 0; // EBP
  *--esp = 0; // ESI
  *--esp = 0; // EDI
  task->esp = (uint32_t)esp;

  // Insert into ready queue
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

  // Simple round-robin: pick the next ready task in the queue
  task_t *next = current_task->next;
  if (!next)
    next = task_queue;

  while (next && next->state != TASK_READY) {
    next = next->next;
    if (!next)
      next = task_queue;
  }

  if (next && next != current_task) {
    next_task = next;
    task_switch(next);
  }
}
