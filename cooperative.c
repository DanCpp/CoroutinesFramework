#include "cooperative.h"

#include "dynamic_array.h"

#include <setjmp.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define START 0   /* start ONLY initial task */
#define RUNNING 1 /* go to the next task if enabled, else run this */
#define DONE 2    /* task finished */

#define STACK_SIZE (4 * 4096) // 16KB

typedef struct {
  void (*task)(void*);
  void* args;
  bool done;
  void* stack;
  jmp_buf context;
  bool started;
} task_t;

DEFINE_DYNAMIC_ARRAY(tasks_t, task_t, 2)

static bool initialized = false;
static tasks_t accumulated;
static jmp_buf main_context;
static task_t* current_task = NULL;
static task_t* prev_task = NULL;

static void coopmult_task_handler() {
  current_task->task(current_task->args);
  longjmp(main_context, DONE);
}

static void coopmult_task_startup() {
  uintptr_t sp = (uintptr_t) ((char*) current_task->stack + STACK_SIZE);
  sp -= sp % 16;
#ifdef __x86_64__
  __asm__ volatile(
      "mov %0, %%rsp\n"
      "call *%1\n"
      :
      : "r"(sp), "r"(coopmult_task_handler));
#else
  __asm__ volatile(
      "mov %0, %%esp\n"
      "call *%1\n"
      :
      : "r"(sp), "r"(coopmult_task_handler));
#endif
  abort(); // Unluck, bro :(
}

static size_t get_next_index(size_t current, size_t total) {
  return (current + 1) % total;
}

void coopmult_add_task(void (*entry)(void*), void* args) {
  if (!initialized) {
    tasks_t_init(&accumulated);
    initialized = true;
  }
  task_t new_task = {
      .task = entry,
      .args = args,
      .done = false,
      .stack = nc_malloc(STACK_SIZE),
      .started = false};
  tasks_t_add_tail(&accumulated, new_task);
}

void coopmult_sleep() {
  if (setjmp(current_task->context) == 0) {
    longjmp(main_context, RUNNING);
  }
}

static void find_next_task() {
  size_t next_index = get_next_index((size_t) (current_task - accumulated.data), accumulated.size);
  current_task = &accumulated.data[next_index];
  while (current_task != prev_task && current_task->done) {
    next_index = get_next_index((size_t) (current_task - accumulated.data), accumulated.size);
    current_task = &accumulated.data[next_index];
  }
}

void coopmult_run() {
  task_t* initial = &accumulated.data[0];
  if (!initial)
    return;
  switch (setjmp(main_context)) {
    case START:
      current_task = initial;
      current_task->started = true;
      coopmult_task_startup();
      break;
    case RUNNING:
      prev_task = current_task;
      find_next_task();
      if (current_task->started) {
        longjmp(current_task->context, RUNNING);
      } else {
        current_task->started = true;
        coopmult_task_startup();
      }
      break;
    case DONE:
      current_task->done = true;
      prev_task = current_task;
      find_next_task();

      if (current_task == prev_task) {
        goto EXIT;
      }

      if (current_task->started) {
        longjmp(current_task->context, RUNNING);
      } else {
        current_task->started = true;
        coopmult_task_startup();
      }
      break;
  }
EXIT:
  for (size_t i = 0; i < accumulated.size; ++i) {
    free(accumulated.data[i].stack);
  }
  tasks_t_deinit(&accumulated);
  initialized = false;
}
