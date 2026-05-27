#pragma once
#include <stdint.h>

#define FLASH_SECTOR0_Addr                  0x08000000      // 16kB
#define FLASH_SECTOR1_Addr                  0x08004000      // 16kB
#define FLASH_SECTOR2_Addr                  0x08008000      // 16kB
#define FLASH_SECTOR3_Addr                  0x0800C000      // 16kB
#define FLASH_SECTOR4_Addr                  0x08010000      // 64kB
#define FLASH_SECTOR5_Addr                  0x08020000      // 128kB
#define FLASH_SECTOR6_Addr                  0x08040000     // 128kB
#define FLASH_SECTOR7_Addr                  0x08060000      // 128kB

void flash_write(const uint32_t *buff, uint32_t size, uint32_t sector, uint32_t address); 
void flash_erase();
uint32_t flash_get_sector (uint32_t *address);
uint32_t flash_get_sector_address (uint32_t sector_number);
uint32_t flash_get_sector_size (uint32_t sector_number);
