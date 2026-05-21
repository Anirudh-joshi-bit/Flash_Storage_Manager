#pragma once
#include <stdint.h>


void flash_write(const uint32_t *buff, uint32_t size, uint32_t sector, uint32_t address); 
