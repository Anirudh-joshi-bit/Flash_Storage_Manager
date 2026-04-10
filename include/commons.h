#pragma once

#include "defines.h"
#include "device/stm32f401xe.h"
#include <stdbool.h>
#include <stddef.h>

uint32_t strlen(const char *msg);
void printf(const char *msg, uint32_t address);
int8_t packet_init (Packet_t *pkt, uint32_t data_size);
int8_t sector_init (Sector_t *sector, void *address);
int8_t flash_write_buffer_init (Flash_write_buffer_t *wb);
int8_t metadata_block_init (MetaData_block_t *mdb);
int8_t keyAddr_pair_init (KeyAddr_pair_t *kap, uint32_t key, void *addr);


// flash storage manager functions

int8_t FSM_write (void *buffer, uint16_t size, bool continuity);



