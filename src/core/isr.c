#include "commons.h"
#include "ring_buff.h"
#include "usart.h"

extern Ring_buff_t usart1_ring_buffer;

void USART1_IRQHandler_c (void){
    if (USART1->SR & USART_SR_RXNE_Msk){
        uint8_t data = USART1-> DR;
        Ring_buff_write (&usart1_ring_buffer, &data, 1);
    }
}
