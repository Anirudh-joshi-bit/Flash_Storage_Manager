#include <stdbool.h>
#include <stdint.h>
#include "ring_buff.h"

void Ring_buff_init(volatile Ring_buff_t *rb) {
  rb->rear = 0;
  rb->front = 0;
}
bool Ring_buff_empty(volatile Ring_buff_t *rb) { return rb->front == rb->rear; }
uint16_t Ring_buff_size(volatile Ring_buff_t *rb) {
  uint16_t local_front = rb->front;
  uint16_t local_rear = rb->rear;

  if (local_front <= local_rear) {
    return local_rear - local_front;
  }
  return RING_BUFF_SIZE - local_front + local_rear;
}

// the below functions should only be called by isr
// Use only REAR for write . donot read / write FRONT
// if ring buffer of overwhelmed ... then increase the size of Ringbuffer

void Ring_buff_write(volatile Ring_buff_t *rb, volatile uint8_t *buff,
                     uint16_t size) {
  // data can be overwritten ... if this happens -> increase the size of the
  // ring buffer

  uint16_t local_rear = rb->rear;

  for (uint16_t ind = 0; ind < size; ind++) {
    rb->buffer[local_rear] = buff[ind];
    local_rear++;
    if (local_rear == RING_BUFF_SIZE)
      local_rear = 0;
  }

  rb->rear = local_rear;
}

// read the whole Ring_buffer
uint16_t Ring_buff_read(volatile Ring_buff_t *rb, volatile uint8_t *buff,
                        uint16_t buff_size) {

  uint16_t local_front = rb->front;
  uint16_t local_rear = rb->rear;

  uint16_t ind = 0;

  while (ind < buff_size && local_front != local_rear) {
    buff[ind] = rb->buffer[local_front];
    local_front++;
    if (local_front == RING_BUFF_SIZE)
      local_front = 0;
    ind++;
  }

  rb->front = local_front;

  return ind;
}
