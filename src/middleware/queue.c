#include "queue.h"


void queue_init (queue_t *q, void *array, uint32_t size){
  q-> array = array;
  q-> capacity = size;
  q-> front = 0;
  q-> rear = 0;

}
bool queue_empty (queue_t *q){
  return q-> front == q-> rear;
}
uint32_t queue_size (queue_t *q){
  return q-> rear - q-> front;
}
bool queue_push (queue_t *q, void *element, uint32_t ele_size){
  if (q->capacity < ele_size + q->rear - q->front) return false;
  uint8_t *ele = (uint8_t *) element;
  for (uint32_t i=0; i<ele_size; i++ ){
    *(uint8_t *)(q-> array) = ele[i];
  }
  q-> rear += ele_size;
  return true;
}
bool queue_pop (queue_t *q, uint32_t ele_size){
  if (q-> front == q-> rear){
    return false;
  }
  q->front += ele_size;

  if (q-> front >= q-> rear){
    q-> front = q-> rear = 0;
    return false;
  }
  return true;
}
