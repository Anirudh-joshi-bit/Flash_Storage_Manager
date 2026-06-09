#include "flash.h"
#include "commons.h"
#include "stm32f401xe.h"
#include "usart.h"



volatile uint8_t flash_state = FLASH_STATE_IDLE;
volatile uint32_t *isr_buffer;
volatile uint32_t *isr_flash_write_address;
volatile uint32_t isr_flash_write_size;

// assumes that hclk is 16MHz
void flash_init (void){

  // unlock the flash control register
  FLASH->KEYR = KEY1;
  FLASH->KEYR = KEY2;

  FLASH-> CR |= FLASH_CR_EOPIE;         // enable eop interrupt
  FLASH-> CR |= FLASH_CR_ERRIE;         // enable err interrupt

  NVIC_EnableIRQ (FLASH_IRQn);
}

uint32_t *flash_get_sector_address (uint32_t sector_number){
  switch (sector_number){
    case 0:
      return (uint32_t *)FLASH_SECTOR0_Addr;
    break;
    case 1:
      return (uint32_t *)FLASH_SECTOR1_Addr;
    break;
    case 2:
      return (uint32_t *)FLASH_SECTOR2_Addr;
    break;
    case 3:
      return (uint32_t *)FLASH_SECTOR3_Addr;
    break;
    case 4:
      return (uint32_t *)FLASH_SECTOR4_Addr;
    break;
    case 5:
      return (uint32_t *)FLASH_SECTOR5_Addr;
    break;
    case 6:
      return (uint32_t *)FLASH_SECTOR6_Addr;
    break;
    case 7:
      return (uint32_t *)FLASH_SECTOR7_Addr;
    break;
    default:
    break;
  }
  return NULL;
}

uint32_t flash_get_sector_size  (uint32_t sector_number){
  switch (sector_number){
    case 0:
      return 16;
    break;
    case 1:
      return 16;
    break;
    case 2:
      return 16;
    break;
    case 3:
      return 16;
    break;
    case 4:
      return 64;
    break;
    case 5:
      return 128;
    break;
    case 6:
      return 128;
    break;
    case 7:
      return 128;
    break;
    default:
    break;
  }
  return -1;
}

uint32_t flash_get_sector (uint32_t *address){

  if (address >= (uint32_t *) 0x08060000 && address < (uint32_t *)0x08080000)
    return 7;
  else if (address >= (uint32_t *) 0x08040000)
    return 6;
  else if (address >= (uint32_t *) 0x08020000)
    return 5;
  else if (address >= (uint32_t *) 0x08010000)
    return 4;
  else if (address >= (uint32_t *) 0x0800c000)
    return 3;
  else if (address >= (uint32_t *) 0x08008000)
    return 2;
  else if (address >= (uint32_t *) 0x08004000)
    return 1;
  else if (address >= (uint32_t *) 0x08000000)
    return 0;

  return -1;

}

uint32_t erase_flash(uint32_t *address) {

  uint32_t sector = flash_get_sector (address);
  if (sector == -1){
    return -1;
  }

  while (flash_state != FLASH_STATE_IDLE);

  // unlock
  FLASH->KEYR = KEY1;
  FLASH->KEYR = KEY2;

  // change the state of the flash to FLASH_STATE_ERASE
  flash_state = FLASH_STATE_ERASE;

  FLASH->CR |= FLASH_CR_SER;
  FLASH->CR &= ~(FLASH_CR_SNB);
  FLASH->CR |= (sector << FLASH_CR_SNB_Pos);
  FLASH->CR |= FLASH_CR_STRT;
  
  return 0;
}


void flash_write(uint32_t *buff, uint32_t buff_size, uint32_t address) {

  while (flash_state != FLASH_STATE_IDLE);

  // assign buffer (visible to isr)
  isr_buffer = buff;
  isr_flash_write_address = (uint32_t *)(address);
  isr_flash_write_size = buff_size;

  // change the state of flash to write
  flash_state = FLASH_STATE_WRITE;

  // unlock the flash control register
  FLASH->KEYR = KEY1;
  FLASH->KEYR = KEY2;

  // set the psize->
  FLASH->CR |= FLASH_CR_PSIZE_1; // 32 bit write;
  // program the PG bit
  FLASH->CR |= FLASH_CR_PG;

  // write one word
  *(isr_flash_write_address++) = isr_buffer [0];
}

