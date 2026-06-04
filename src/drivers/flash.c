#include "flash.h"
#include "commons.h"
#include "usart.h"

#define MAX_STR_SIZE 100
#define KEY1 0x45670123
#define KEY2 0xCDEF89AB

 

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

  if (address >= (uint32_t *) 0x08060000)
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
  if (address >= (uint32_t *)0x08080000 || address < (uint32_t *)0x08000000) {
    printf(__usart1_print ,"wrong address \n\r");
    return -1;
  }

  uint32_t sector = flash_get_sector (address);
  if (sector == -1){
    return -1;
  }

  // unlock
  FLASH->KEYR = KEY1;
  FLASH->KEYR = KEY2;

  FLASH->SR |= FLASH_SR_EOP |    // End of operation
               FLASH_SR_OPERR |  // Operation error
               FLASH_SR_WRPERR | // Write protection error
               FLASH_SR_PGAERR | // Programming alignment error
               FLASH_SR_PGPERR | // Programming parallelism error
               FLASH_SR_PGSERR;  // Programming sequence error

  // wait for operation to be done
  while (FLASH->SR & FLASH_SR_BSY)
    ;

  FLASH->CR |= FLASH_CR_SER;
  FLASH->CR &= ~(FLASH_CR_SNB);
  FLASH->CR |= (sector << FLASH_CR_SNB_Pos);
  FLASH->CR |= FLASH_CR_STRT;

  // wait for the flash to be erased;
  // this is not needed , erase the flash in background
  while (FLASH->SR & FLASH_SR_BSY)
    ;

  // clear the erase bit
  FLASH->CR &= ~(FLASH_CR_SER);
  // lock the control register
  FLASH->CR |= FLASH_CR_LOCK;

  printf(__usart1_print, "done erasing flash (address = %)\n\r", address);
  return 0;
}


void flash_write(const uint32_t *buff, uint32_t size, uint32_t sector,
                 uint32_t address) {

  if (address % 4) {
    printf(__usart1_print, "invalid address\n\r");
    return;
  }
  if (sector > 7 || sector < 0) { // there are 8 sectors
    printf(__usart1_print, "sector does not exist\n\r");
    return;
  }
  FLASH->SR |= FLASH_SR_EOP |    // End of operation
               FLASH_SR_OPERR |  // Operation error
               FLASH_SR_WRPERR | // Write protection error
               FLASH_SR_PGAERR | // Programming alignment error
               FLASH_SR_PGPERR | // Programming parallelism error
               FLASH_SR_PGSERR;  // Programming sequence error
  // unlock the flash control register
  FLASH->KEYR = KEY1;
  FLASH->KEYR = KEY2;

  // wait for FLASH to be idle
  while (FLASH->SR & FLASH_SR_BSY)
    ;

  // erase ------------------------------------------
  // set he SER bit -> sector erase
  FLASH->CR |= FLASH_CR_SER;
  FLASH->CR &= ~(FLASH_CR_SNB);
  FLASH->CR |= (sector << FLASH_CR_SNB_Pos);
  FLASH->CR |= FLASH_CR_STRT; // start erasing, self cleaning  !!!

  // wait for the flash to complete erasing
  while (FLASH->SR & FLASH_SR_BSY)
    ;

  // clear the erase bit
  FLASH->CR &= ~(FLASH_CR_SER);

  // programming --------------------------------------
  // set the psize->
  FLASH->CR |= FLASH_CR_PSIZE_1; // 32 bit write;
  // program the PG bit
  FLASH->CR |= FLASH_CR_PG;

  // write into the flash
  uint32_t i = 0;
  while (i < size) {
    while (FLASH->SR & FLASH_SR_BSY)
      ;
    *((uint32_t *)address) = buff[i++];
    address += 4;
  }
  printf(__usart1_print, "wrote to the flash \n\r");
  // wait for the writing to be complete
  while (FLASH->SR & FLASH_SR_BSY)
    ;

  FLASH->CR &= ~(FLASH_CR_PG);
  FLASH->CR |= FLASH_CR_LOCK;
}

