#include "cooperative.h"

#include <setjmp.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define START 0
#define RUNNING 1
#define DONE 2

#define STACK_SIZE (4 * 4096) // 16KB

static void* nc_realloc(void* old, size_t nmemb) {
  void* ptr = realloc(old, nmemb);
  if (!ptr) {
    abort();
  }

  return ptr;
}

static void* nc_malloc(size_t nmemb) {
  void* ptr = malloc(nmemb);
  if (!ptr) {
    abort();
  }

  return ptr;
}

typedef struct {
  void (*task)(void*);
  void* args;
  bool done;
  void* stack;
  jmp_buf context;
  bool started;
} task_t;

typedef struct {
  task_t* tasks;
  size_t size;
  size_t cap;
} tasks_t;

static bool initialized = false;
static tasks_t accumulated;
static jmp_buf main_context;
static task_t* current_task = NULL;
static task_t* prev_task = NULL;

static void coopmult_task_done() {
  longjmp(main_context, DONE);
}

static void coopmult_task_startup(void (*entry)(void*), void* args, void* SP) {
  uintptr_t stack_ptr = (uintptr_t) SP;
  stack_ptr -= stack_ptr % 16;
  uintptr_t* sp = (uintptr_t*) stack_ptr;
#ifndef __x86_64__
  sp--;
  *sp = (uintptr_t) args;
#endif

  sp--;
  *sp = (uintptr_t) coopmult_task_done;
#ifdef __x86_64__

  __asm__ volatile(
      "mov %0, %%rsp\n"
      "mov %1, %%rdi\n"
      "jmp *%2\n"
      :
      : "r"(sp), "r"(args), "r"(entry));
#else
  __asm__ volatile(
      "mov %0, %%esp\n"
      "jmp *%1\n"
      :
      : "r"(sp), "r"(entry));
#endif

  abort();
}

static size_t get_next_index(size_t current, size_t total) {
  return (current + 1) % total;
}

void coopmult_add_task(void (*entry)(void*), void* args) {
  if (!initialized) {
    accumulated.size = 0;
    accumulated.cap = 1;
    accumulated.tasks = nc_malloc(sizeof(*accumulated.tasks));
    initialized = true;
  }
  if (accumulated.size == accumulated.cap) {
    accumulated.cap <<= 1;
    accumulated.tasks = nc_realloc(accumulated.tasks, accumulated.cap * sizeof(*accumulated.tasks));
  }

  accumulated.tasks[accumulated.size].task = entry;
  accumulated.tasks[accumulated.size].args = args;
  accumulated.tasks[accumulated.size].done = false;
  accumulated.tasks[accumulated.size].started = false;
  accumulated.tasks[accumulated.size].stack = nc_malloc(STACK_SIZE);
  accumulated.size++;
}

void coopmult_sleep() {
  if (setjmp(current_task->context) == 0) {
    longjmp(main_context, RUNNING);
  }
}

static void find_next_task() {
  size_t next_index = get_next_index((size_t) (current_task - accumulated.tasks), accumulated.size);
  current_task = &accumulated.tasks[next_index];
  while (current_task != prev_task && current_task->done) {
    next_index = get_next_index((size_t) (current_task - accumulated.tasks), accumulated.size);
    current_task = &accumulated.tasks[next_index];
  }
}

void coopmult_run() {
  task_t* initial = &accumulated.tasks[0];
  if (!initial)
    return;
  switch (setjmp(main_context)) {
    case START:
      current_task = initial;
      if (!current_task->done) {
        current_task->started = true;
        coopmult_task_startup(current_task->task, current_task->args,
                              ((char*) current_task->stack) + STACK_SIZE);
      }
      break;
    case RUNNING:
      prev_task = current_task;
      find_next_task();
      if (current_task->started) {
        longjmp(current_task->context, RUNNING);
      } else {
        current_task->started = true;
        coopmult_task_startup(current_task->task, current_task->args,
                              ((char*) current_task->stack) + STACK_SIZE);
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
        coopmult_task_startup(current_task->task, current_task->args,
                              ((char*) current_task->stack) + STACK_SIZE);
      }
      break;
  }
EXIT:
  for (size_t i = 0; i < accumulated.size; ++i) {
    free(accumulated.tasks[i].stack);
  }
  free(accumulated.tasks);
  accumulated.tasks = NULL;
  accumulated.size = 0;
  accumulated.cap = 0;
  initialized = false;
  current_task = NULL;
}
