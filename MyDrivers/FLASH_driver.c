#include "../include/commons.h"
#define MAX_STR_SIZE 100
#define KEY1 0x45670123
#define KEY2 0xCDEF89AB

void flash_write(const uint32_t *buff, uint32_t size, uint32_t sector,
                 uint32_t address) {

  if (address % 4) {
    printf("invalid address\n\r", 0x0);
    return;
  }
  if (sector > 7 || sector < 0) { // there are 8 sectors
    printf("sector does not exist\n\r", 0x0);
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
  printf("wrote to the flash \n\r", 0x0);
  // wait for the writing to be complete
  while (FLASH->SR & FLASH_SR_BSY)
    ;

  FLASH->CR &= ~(FLASH_CR_PG);
  FLASH->CR |= FLASH_CR_LOCK;
}
