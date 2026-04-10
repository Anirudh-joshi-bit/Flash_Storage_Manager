#pragma once
#include "ring_buff.h"
#include <stdint.h>


void __usart1_init(void);
void __usart1_print(const char *msg, uint32_t size);
void __usart1_scan (Ring_buff_t *rb);
