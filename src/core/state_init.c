#include "defines.h"

int8_t packet_init(Packet_t *pkt, uint32_t data_size) {
  if (data_size > MAX_DATA_BLOCK_SIZE / 4 - PACKET_MEMBER_COUNT + 1)
    return -1;

  pkt->data_size = data_size;
  pkt->valid = VALID_FLASH;

  return 0;
}

int8_t sector_init (Sector_t *sector, void *address){

  if (address < (void *)0x08000000 ||
      address > (void *)0x08080000 ){
    return -1;
  }

  if (address >= (void *) 0x08060000){
    sector->sector_number = 7;
    sector->size = 128*1024;
    sector->address = FLASH_SECTOR7_Addr;
  }
  else if (address >= (void *) 0x08040000){
    sector->sector_number = 6;
    sector->size = 128*1024;
    sector->address = FLASH_SECTOR6_Addr;
  }
  else if (address >= (void *) 0x08020000){
    sector->sector_number = 5;
    sector->size = 128*1024;
    sector->address = FLASH_SECTOR5_Addr;
  }
  else if (address >= (void *) 0x08010000){
    sector->sector_number = 4;
    sector->size = 64*1024;
    sector->address = FLASH_SECTOR4_Addr;
  }
  else if (address >= (void *) 0x0800c000){
    sector->sector_number = 3;
    sector->size = 16*1024;
    sector->address = FLASH_SECTOR3_Addr;
  }
  else if (address >= (void *) 0x08008000){
    sector->sector_number = 2;
    sector->size = 16*1024;
    sector->address = FLASH_SECTOR2_Addr;
  }
  else if (address >= (void *) 0x08004000){
    sector->sector_number = 1;
    sector->size = 16*1024;
    sector->address = FLASH_SECTOR1_Addr;
  }
  else{
    sector->sector_number = 0;
    sector->size = 16*1024;
    sector->address = FLASH_SECTOR0_Addr;
  }

  return 0;
}

int8_t flash_write_buffer_init (Flash_write_buffer_t *wb){
  wb -> size = 0;

  return 0;
}

int8_t metadata_block_init (MetaData_block_t *mdb){

  mdb-> start_word = 0xffffffff;
  for (uint8_t i=0; i<MAX_FLAG_COUNT; i++){
    mdb->flags [i] = FLAGS_INIT_VALUE; 
  }

  return 0;
}

int8_t keyAddr_pair_init (KeyAddr_pair_t *kap, uint32_t key, void *addr){

  if (key == VALID_FLASH ||
      addr < (void *) 0x08000000 ||
      addr > (void *) 0x08080000){

    return -1;
  }

  kap-> key = key;
  kap-> address = addr;
  kap-> valid = 0x0fffffff;

  return 0;
}


