
#include "cooperative.h"
#include "dynamic_array.h"

#include <stdio.h>
#include <stdlib.h>

DEFINE_ALL(int_array, int, 2)

int fibbonachi(int arg) {
  if (arg <= 1)
    return arg;
  coopmult_sleep();
  return fibbonachi(arg - 1) + fibbonachi(arg - 2);
}

void sleeper_sort_task(void* arg) {
  int value = *(int*) arg;
  int max_depth = 1 + value;
  fibbonachi(max_depth);
  printf("%d\n", value);
}

int main() {
  int_array arr;
  int_array_init(&arr);
  int a = 0;
  do {
    scanf("%d", &a);
    int_array_add_tail(&arr, a);
  } while (a);

  for (size_t i = 0; i < arr.size; i++) {
    coopmult_add_task(sleeper_sort_task, &arr.data[i]);
  }
  coopmult_run();

  int_array_deinit(&arr);
  return 0;
}
