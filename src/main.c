#include "../include/commons.h"
#include "../include/USART.h"
#include "../include/Ring_buff.h"

uint8_t buff[RING_BUFF_SIZE];

Ring_buff_t usart1_ring_buffer;

int main (void){
    
    NVIC_EnableIRQ(USART1_IRQn);

    __usart1_init();
    Ring_buff_init (&usart1_ring_buffer);

    printf ("give me some data", 0x0);
 
    while (usart1_ring_buffer.size < 30);
    
    Ring_buff_read (&usart1_ring_buffer, buff, RING_BUFF_SIZE);
    
    printf ("read from ring buffer", 0x0);


    while (1);
    return 0;
}
