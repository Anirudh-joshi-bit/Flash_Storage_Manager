#include "commons.h"
#include "ring_buff.h"
#include "usart.h"
#include "flash.h"
#include <stdatomic.h>

extern volatile Ring_buff_t usart1_ring_buffer;
extern volatile Ring_buff_t usart2_ring_buffer;
extern volatile Ring_buff_t usart6_ring_buffer;
extern volatile uint8_t flash_state;
extern volatile uint32_t *isr_buffer;
uint32_t buffer_ind = 1;
extern volatile uint32_t *isr_flash_write_address;
extern volatile uint32_t isr_flash_write_size;


void USART1_IRQHandler_c (void){
    if (USART1->SR & USART_SR_RXNE_Msk){
        uint8_t data = USART1-> DR;
        Ring_buff_write (&usart1_ring_buffer, &data, 1);
    }
}


void USART2_IRQHandler_c (void){
    if (USART2->SR & USART_SR_RXNE_Msk){
        uint8_t data = USART2-> DR;
        Ring_buff_write (&usart2_ring_buffer, &data, 1);
    }
}


void USART6_IRQHandler_c (void){
    if (USART6->SR & USART_SR_RXNE_Msk){
        uint8_t data = USART6-> DR;
        Ring_buff_write (&usart6_ring_buffer, &data, 1);
    }
}


void FLASH_IRQHandler (void){
  // handle error
  
  if(flash_state == FLASH_STATE_WRITE) {
    // flash is written
    *isr_flash_write_address++ = isr_buffer[buffer_ind++];
    if (buffer_ind == isr_flash_write_size){
      // falsh write operation is done
      FLASH->SR = FLASH_SR_EOP
                  |FLASH_SR_OPERR
                  |FLASH_SR_WRPERR
                  |FLASH_SR_PGAERR
                  |FLASH_SR_PGPERR
                  |FLASH_SR_PGSERR;

      flash_state = FLASH_STATE_IDLE;
      FLASH->CR |= FLASH_CR_LOCK;
    }
  }
  else if (flash_state == FLASH_STATE_ERASE){
    // flash is erased 
    FLASH->SR |= FLASH_SR_EOP|
                 FLASH_SR_OPERR |  
                 FLASH_SR_WRPERR | 
                 FLASH_SR_PGAERR | 
                 FLASH_SR_PGPERR | 
                 FLASH_SR_PGSERR;  

    flash_state = FLASH_STATE_IDLE;
    FLASH-> CR |= FLASH_CR_LOCK;
  }
}
