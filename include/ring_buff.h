#pragma once
#include <stdint.h>

#define RING_BUFF_SIZE              10*1024    // 10kB


typedef struct Ring_buff_t {
    uint8_t buffer [RING_BUFF_SIZE];
    uint16_t rear;                              // next free address
    uint16_t front;                             // front of the rb
}Ring_buff_t;


void Ring_buff_init (volatile Ring_buff_t *rb);
void Ring_buff_write(volatile Ring_buff_t *rb, volatile uint8_t *buff, uint16_t size);
uint16_t Ring_buff_read(volatile Ring_buff_t *rb, volatile uint8_t *buff, uint16_t buff_size);
bool Ring_buff_empty(volatile Ring_buff_t *rb);
uint16_t Ring_buff_size(volatile Ring_buff_t *rb);
