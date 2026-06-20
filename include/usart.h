#pragma once
#include "ring_buff.h"
#include <stdint.h>


void __usart1_init(void);
void __usart1_print(const char *msg, uint32_t size);
void __usart1_scan (volatile Ring_buff_t *rb);
bool __int_usart1_is_data_recieved (void);
uint8_t __usart1_get_data (void);

void __usart2_init(void);
void __usart2_print(const char *msg, uint32_t size);
void __usart2_scan (volatile Ring_buff_t *rb);
bool __int_usart2_is_data_recieved (void);
uint8_t __usart2_get_data (void);

void __usart6_init(void);
void __usart6_print(const char *msg, uint32_t size);
void __usart6_scan (volatile Ring_buff_t *rb);
bool __int_usart6_is_data_recieved (void);
uint8_t __usart6_get_data (void);
