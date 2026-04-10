#include "ring_buff.h"
#include <stdint.h>

void Ring_buff_init (struct Ring_buff_t *rb){
    rb-> size = 0;
    rb-> rear = 0;
    rb-> front = 0xffff;
}

// the below functions should only be called by isr

int8_t Ring_buff_write (Ring_buff_t *rb, uint8_t *buff, uint16_t size){
    
    if (1ll * RING_BUFF_SIZE < size + rb->size)
        return -1;
    if (rb->front == 0xffff && size)
      rb->front = 0;

    for (uint16_t i=0; i<size; i++){
        rb->buffer [rb->rear] = buff[i];
        rb->rear ++;
        if (rb->rear == RING_BUFF_SIZE)
            rb->rear = 0;
    }
    rb->size += size;
    return 0;
}

uint16_t Ring_buff_read (Ring_buff_t *rb, uint8_t *buff, uint16_t size){
    if (rb->size == 0) 
        return 0;

    uint16_t size_read = size > rb->size ? rb->size : size;
    
    for (uint16_t i=0; i<size_read; i++){
        buff[i] = rb->buffer[rb->front];
        rb-> front ++;
        if (rb->front == RING_BUFF_SIZE)
            rb->front = 0;
    }
    if (rb->front == rb->rear){
        rb->front = -1;
        rb->rear = 0;
    }
    rb->size -= size_read;
    return size_read;
}
