#include "DEBUG.h"
#include "commons.h"
#include "flash.h"
#include "ring_buff.h"
#include "usart.h"
#include <stdatomic.h>

extern volatile Ring_buff_t usart1_ring_buffer;
extern volatile Ring_buff_t usart2_ring_buffer;
extern volatile Ring_buff_t usart6_ring_buffer;
extern volatile uint8_t flash_state;
extern volatile uint32_t *isr_buffer;
uint32_t buffer_ind = 1;
extern volatile uint32_t *isr_flash_write_address;
extern volatile uint32_t isr_flash_write_size;

void USART1_IRQHandler_c(void) {
  if (__int_usart1_is_data_recieved()) {
    uint8_t data = __usart1_get_data();
    Ring_buff_write(&usart1_ring_buffer, &data, 1);
  }
}

void USART2_IRQHandler_c(void) {
  if (__int_usart2_is_data_recieved()) {
    uint8_t data = __usart2_get_data();
    Ring_buff_write(&usart2_ring_buffer, &data, 1);
  }
}

void USART6_IRQHandler_c(void) {
  if (__int_usart6_is_data_recieved()) {
    uint8_t data = __usart6_get_data();
    Ring_buff_write(&usart6_ring_buffer, &data, 1);
  }
}

void FLASH_IRQHandler(void) {
  // handle error

  if (flash_state == FLASH_STATE_WRITE) {
    // flash is written
    *isr_flash_write_address++ = isr_buffer[buffer_ind++];
    if (buffer_ind == isr_flash_write_size) {
      // falsh write operation is done

      flash_clear_sr();
      flash_state = FLASH_STATE_IDLE;
      flash_lock();
    }
  } else if (flash_state == FLASH_STATE_ERASE) {
    // flash is erased

    flash_clear_sr();
    flash_state = FLASH_STATE_IDLE;
    flash_lock();
  }
}
