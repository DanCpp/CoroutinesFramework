
#include "cooperative.h"
#include "dynamic_array.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

DEFINE_DYNAMIC_ARRAY(int_array, int, 2)
DEFINE_DYNAMIC_ARRAY(long_array, size_t, 2)

size_t factorial(size_t n) {
  if (n <= 1)
    return 1;
  coopmult_sleep();
  return n * factorial(n - 1);
}

int fibbonachi(int arg) {
  if (arg <= 1)
    return arg;
  coopmult_sleep();
  return fibbonachi(arg - 1) + fibbonachi(arg - 2);
}

void sleeper_sort_task(void* arg) {
  size_t value = *(size_t*) arg;
  size_t max_depth = 1 + value;
  factorial(max_depth);
  printf("%zu\n", value);
}

int main() {
  long_array arr;
  long_array_init(&arr);
  size_t a = 0;
  do {
    scanf("%zu", &a);
    long_array_add_tail(&arr, a);
  } while (a);

  for (size_t i = 0; i < arr.size; i++) {
    coopmult_add_task(sleeper_sort_task, &arr.data[i]);
  }
  coopmult_run();

  long_array_deinit(&arr);
  return 0;
}
