#include "../include/commons.h"
#include "../include/Ring_buff.h"
#include "../include/USART.h"

extern Ring_buff_t usart1_ring_buffer;

void USART1_IRQHandler_c (void){
    if (USART1->SR & USART_SR_RXNE){
        Ring_buff_write (&usart1_ring_buffer, (uint8_t *)(USART1->DR), 1);
    }
}
