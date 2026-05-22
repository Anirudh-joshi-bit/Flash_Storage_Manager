#pragma once

#include <stdint.h>
#include <stdbool.h>


// not a circular queue
// capasity in bytes

typedef struct __queue_t{

  void* array;
  uint32_t capacity;
  uint32_t front;
  uint32_t rear;

}queue_t;

void queue_init (queue_t *q, void *array, uint32_t size);
bool queue_empty (queue_t *q);
uint32_t queue_size (queue_t *q);
bool queue_push (queue_t *q, void *element, uint32_t ele_size);
bool queue_pop (queue_t *q, uint32_t ele_size);
