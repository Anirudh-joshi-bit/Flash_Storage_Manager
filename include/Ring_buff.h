#pragma once
#include <stdint.h>

#define RING_BUFF_SIZE              10*1024    // 10kB


typedef struct Ring_buff_t {
    uint16_t size;
    uint8_t buffer [RING_BUFF_SIZE];
    uint16_t rear;                              // next free address
    uint16_t front;                             // front of the rb
}Ring_buff_t;


void Ring_buff_init (struct Ring_buff_t *rb);
int8_t Ring_buff_write (Ring_buff_t *rb, uint8_t *buff, uint16_t size);
uint16_t Ring_buff_read (Ring_buff_t *rb, uint8_t *buff, uint16_t size);
