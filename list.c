#include "vector.h"
#include <assert.h>
#include <stdlib.h>

#include <stdio.h>

// defines free_func_t
typedef void (*free_func_t)(void *);

// defines list_t
typedef struct list {
  void **arr;
  size_t capacity;
  size_t size;
  free_func_t freer;
} list_t;

// initializes array and capacity
list_t *list_init(size_t initial_size, free_func_t freer) {
  list_t *new_list = malloc(sizeof(list_t));
  assert(new_list != NULL);
  new_list->capacity = initial_size;
  if (initial_size == 0) {
    initial_size = 1;
  }
  new_list->arr = malloc(sizeof(void *) * initial_size);
  assert(new_list->arr != NULL);
  new_list->capacity = initial_size;
  new_list->size = 0;
  new_list->freer = freer;

  return new_list;
}

size_t list_size(list_t *list) { return list->size; }

void list_free(list_t *list) {
  for (size_t i = 0; i < list->size; i++) {
    list->freer(list->arr[i]);
  }
  free(list->arr);
  free(list);
}

void list_resize(list_t *list) {
  if (list->capacity == 0) {
    list->arr = realloc(list->arr, 2 * sizeof(void *));
    list->capacity = 2;
  } else {
    list->arr = realloc(list->arr, 2 * list_size(list) * sizeof(void *));
    list->capacity = 2 * list_size(list);
  }
}

void *list_get(list_t *list, size_t index) {
  assert(index < list->size && index >= 0);
  return list->arr[index];
}

void list_add(list_t *list, void *value) {
  assert(value != NULL);
  if (list->size == list->capacity) {
    list_resize(list);
  }
  list->arr[list->size] = value;
  list->size++;
}

void *list_remove(list_t *list, size_t index) {
  assert(list->size > 0);
  void *old_value = list->arr[index];

  for (size_t i = index + 1; i < list_size(list); i++) {
    void *prev_value = list->arr[i];
    list->arr[i - 1] = prev_value;
  }

  list->size--;
  return old_value;
}
