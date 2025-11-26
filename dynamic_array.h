#ifndef DYNAMIC_ARRAY
#define DYNAMIC_ARRAY

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void* nc_malloc(size_t _nmemb) {
  void* ptr = malloc(_nmemb);
  if (!ptr) {
    perror("Memory allocation failed");
    abort();
  }

  return ptr;
}

void* nc_calloc(size_t n, size_t memb) {
  void* ptr = calloc(n, memb);
  if (!ptr) {
    perror("Memory allocation failed");
    abort();
  }

  return ptr;
}

void* nc_realloc(void* old, size_t _nmemb) {
  void* ptr = realloc(old, _nmemb);
  if (!ptr) {
    perror("Memory reallocation failed");
    abort();
  }

  return ptr;
}

#define DEFAULT_CAP 2

#define STRUCT(NAME, T) \
  typedef struct {      \
    T* data;            \
    size_t size;        \
    size_t cap;         \
  } NAME;


/* Define after INIT_FUNC or use DEFINE_ALL */
#define CREATE_FUNC(NAME, DEFAULT_CAP)         \
  NAME* NAME##_new() {                         \
    NAME* result = nc_calloc(1, sizeof(NAME)); \
    NAME##_init(result);                       \
    return result;                             \
  }

#define RESERVE_FUNC(NAME)                                               \
  void NAME##_reserve(NAME* arr, size_t reserve_cap) {                   \
    if (arr->cap >= reserve_cap)                                         \
      return;                                                            \
    arr->cap = reserve_cap;                                              \
    arr->data = nc_realloc(arr->data, reserve_cap * sizeof(*arr->data)); \
  }

#define INIT_FUNC(NAME, DEFAULT_CAP)                         \
  void NAME##_init(NAME* arr) {                              \
    if (!arr)                                                \
      return;                                                \
                                                             \
    arr->data = nc_malloc(sizeof(*arr->data) * DEFAULT_CAP); \
    arr->size = 0;                                           \
    arr->cap = DEFAULT_CAP;                                  \
  }

#define DEINIT_FUNC(NAME)         \
  void NAME##_deinit(NAME* arr) { \
    if (!arr || !arr->data)       \
      return;                     \
                                  \
    free(arr->data);              \
    arr->size = 0;                \
    arr->cap = 0;                 \
  }

#define FREE_FUNC(NAME)         \
  void NAME##_free(NAME* arr) { \
    free(arr->data);            \
    free(arr);                  \
  }

#define SET_FUNC(NAME, T)                                           \
  void NAME##_set(NAME* arr, const size_t index, const T element) { \
    if (index >= arr->size)                                         \
      return;                                                       \
    arr->data[index] = element;                                     \
  }

#define GET_FUNC(NAME, T)                       \
  T NAME##_get(NAME* arr, const size_t index) { \
    return arr->data[index];                    \
  }

/* define after RESERVE_FUNC or use DEFINE_ALL*/
#define ADD_TAIL_FUNC(NAME, T)                 \
  void NAME##_add_tail(NAME* arr, T element) { \
    if (arr->size == arr->cap) {               \
      NAME##_reserve(arr, arr->cap * 2);       \
    }                                          \
                                               \
    arr->data[arr->size++] = element;          \
  }

/* define after RESERVE FUNCT or use DEFINE_ALL */
#define FILL_FUNC(NAME, T)                              \
  void NAME##_fill(NAME* arr, size_t count, T filter) { \
    if (arr->cap < count)                               \
      NAME##_reserve(arr, arr->cap * 2);                \
    for (size_t i = 0; i < count; i++) {                \
      arr->data[arr->size++] = filter;                  \
    }                                                   \
  }

#define DELETE_TAIL_FUNC(NAME, T)  \
  T NAME##_del_tail(NAME* arr) {   \
    return arr->data[--arr->size]; \
  }

#define DELETE_ELEMENT_FUNC(NAME, T)                 \
  T NAME##_del_element(NAME* arr, size_t index) {    \
    T deleted = arr->data[index];                    \
    for (size_t i = index + 1; i < arr->size; i++) { \
      arr->data[i - 1] = arr->data[i];               \
    }                                                \
    arr->size--;                                     \
                                                     \
    return deleted;                                  \
  }

#define DELETE_FUNC(NAME)         \
  void NAME##_delete(NAME* arr) { \
    free(arr->data);              \
    free(arr);                    \
  }

#define DEFINE_ALL(NAME, T, DEFAULT_CAP) \
  STRUCT(NAME, T)                        \
  RESERVE_FUNC(NAME)                     \
  INIT_FUNC(NAME, DEFAULT_CAP)           \
  CREATE_FUNC(NAME, DEFAULT_CAP)         \
  DEINIT_FUNC(NAME)                      \
  FREE_FUNC(NAME)                        \
  SET_FUNC(NAME, T)                      \
  GET_FUNC(NAME, T)                      \
  ADD_TAIL_FUNC(NAME, T)                 \
  DELETE_TAIL_FUNC(NAME, T)              \
  DELETE_ELEMENT_FUNC(NAME, T)           \
  DELETE_FUNC(NAME)

#endif
