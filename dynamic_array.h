#ifndef DYNAMIC_ARRAY
#define DYNAMIC_ARRAY

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void* nc_malloc(size_t _nmemb) {
  void* ptr = malloc(_nmemb);
  if (!ptr) {
    perror("Memory allocation failed");
    abort();
  }

  return ptr;
}

static void* nc_calloc(size_t n, size_t memb) {
  void* ptr = calloc(n, memb);
  if (!ptr) {
    perror("Memory allocation failed");
    abort();
  }

  return ptr;
}

static void* nc_realloc(void* old, size_t _nmemb) {
  void* ptr = realloc(old, _nmemb);
  if (!ptr) {
    perror("Memory reallocation failed");
    abort();
  }

  return ptr;
}

#define EXPONENT_FROM_DOUBLE_CONST 0x7FF00000

#define DEFINE_DYNAMIC_ARRAY(NAME, T, DEFAULT_CAP)                    \
  typedef struct {                                                    \
    T* data;                                                          \
    size_t size;                                                      \
    size_t cap;                                                       \
  } NAME;                                                             \
                                                                      \
                                                                      \
  void NAME##_init(NAME* arr) {                                       \
    if (!arr)                                                         \
      return;                                                         \
                                                                      \
    arr->data = nc_malloc(sizeof(*arr->data) * DEFAULT_CAP);          \
    arr->size = 0;                                                    \
    arr->cap = DEFAULT_CAP;                                           \
  }                                                                   \
                                                                      \
  NAME* NAME##_new() {                                                \
    NAME* result = nc_calloc(1, sizeof(NAME));                        \
    NAME##_init(result);                                              \
    return result;                                                    \
  }                                                                   \
                                                                      \
  void NAME##_reserve(NAME* arr, size_t reserve_cap) {                \
    if (arr->cap >= reserve_cap)                                      \
      return;                                                         \
    double k = (double) reserve_cap;                                  \
    uint32_t* p = (uint32_t*) &k;                                     \
    p++;                                                              \
    *p = 1 << ((((*p & EXPONENT_FROM_DOUBLE_CONST) >> 20) - 1022));   \
                                                                      \
    arr->cap = (size_t) * p;                                          \
    arr->data = nc_realloc(arr->data, arr->cap * sizeof(*arr->data)); \
  }                                                                   \
                                                                      \
  void NAME##_deinit(NAME* arr) {                                     \
    if (!arr || !arr->data)                                           \
      return;                                                         \
                                                                      \
    free(arr->data);                                                  \
    arr->size = 0;                                                    \
    arr->cap = 0;                                                     \
  }                                                                   \
                                                                      \
  void NAME##_free(NAME* arr) {                                       \
    free(arr->data);                                                  \
    free(arr);                                                        \
  }                                                                   \
                                                                      \
  void NAME##_set(NAME* arr, const size_t index, const T element) {   \
    arr->data[index] = element;                                       \
  }                                                                   \
                                                                      \
  T NAME##_get(NAME* arr, const size_t index) {                       \
    return arr->data[index];                                          \
  }                                                                   \
                                                                      \
  void NAME##_add_tail(NAME* arr, T element) {                        \
    NAME##_reserve(arr, arr->size + 1);                               \
    arr->data[arr->size++] = element;                                 \
  }                                                                   \
                                                                      \
  void NAME##_fill(NAME* arr, size_t count, T filter) {               \
    NAME##_reserve(arr, arr->size + count);                           \
    for (size_t i = 0; i < count; i++) {                              \
      arr->data[arr->size++] = filter;                                \
    }                                                                 \
  }                                                                   \
                                                                      \
  T NAME##_del_tail(NAME* arr) {                                      \
    return arr->data[--arr->size];                                    \
  }                                                                   \
                                                                      \
  T NAME##_del_element(NAME* arr, size_t index) {                     \
    T deleted = arr->data[index];                                     \
    memmove(arr->data + index, arr->data + index + 1,                 \
            (arr->size - index - 1) * sizeof(T));                     \
    arr->size--;                                                      \
                                                                      \
    return deleted;                                                   \
  }                                                                   \
                                                                      \
  void NAME##_delete(NAME* arr) {                                     \
    free(arr->data);                                                  \
    free(arr);                                                        \
  }

#endif
