#pragma once
#include "stm32f401xe.h"
#include <stdint.h>


#define MAX_STR_SIZE 100
#define KEY1 0x45670123
#define KEY2 0xCDEF89AB

#define FLASH_STATE_WRITE   0x0
#define FLASH_STATE_ERASE   0x1
#define FLASH_STATE_READ    0x3
#define FLASH_STATE_IDLE    0x4

#define FLASH_SECTOR0_Addr                  0x08000000      // 16kB
#define FLASH_SECTOR1_Addr                  0x08004000      // 16kB
#define FLASH_SECTOR2_Addr                  0x08008000      // 16kB
#define FLASH_SECTOR3_Addr                  0x0800C000      // 16kB
#define FLASH_SECTOR4_Addr                  0x08010000      // 64kB
#define FLASH_SECTOR5_Addr                  0x08020000      // 128kB
#define FLASH_SECTOR6_Addr                  0x08040000      // 128kB
#define FLASH_SECTOR7_Addr                  0x08060000      // 128kB

extern volatile uint8_t flash_state;


void flash_init (void);
void flash_write(void *buff, uint32_t size, void *address);
uint32_t flash_erase(void *address);
uint32_t flash_get_sector (void  *address);
void *flash_get_sector_address (uint32_t sector_number);
uint32_t flash_get_sector_size (uint32_t sector_number);

