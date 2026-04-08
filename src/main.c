#include "../include/commons.h"
#include "../include/USART.h"
#include "../include/Ring_buff.h"

uint8_t buff[RING_BUFF_SIZE];

Ring_buff_t usart1_ring_buffer;

int main (void){
    __usart1_init();
    Ring_buff_init (&usart1_ring_buffer);

    printf ("give me some data", 0x0);
 
    for (int i=0; i<100; i++){
        __usart1_scan (&usart1_ring_buffer);
    }

    Ring_buff_read (&usart1_ring_buffer, buff, RING_BUFF_SIZE);

    while (1);
    return 0;
}
