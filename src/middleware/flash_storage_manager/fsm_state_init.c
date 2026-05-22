#include "flash_storage_manager.h"
#include "queue.h"
#include "ring_buff.h"

int8_t packet_init(Packet_t *pkt, uint32_t data_size) {
  if (data_size > MAX_DATA_BLOCK_SIZE / 4 - PACKET_MEMBER_COUNT + 1)
    return -1;

  pkt->data_size = data_size;
  pkt->removed = VALID_FLAG;
  pkt->valid = VALID_FLAG;

  return 0;
}

int8_t sector_init(Sector_t *sector, void *address) {

  if (address < (void *)0x08000000 || address > (void *)0x08080000) {
    return -1;
  }

  if (address >= (void *)0x08060000) {
    sector->sector_number = 7;
    sector->size = 128 * 1024;
    sector->address = FLASH_SECTOR7_Addr;
  } else if (address >= (void *)0x08040000) {
    sector->sector_number = 6;
    sector->size = 128 * 1024;
    sector->address = FLASH_SECTOR6_Addr;
  } else if (address >= (void *)0x08020000) {
    sector->sector_number = 5;
    sector->size = 128 * 1024;
    sector->address = FLASH_SECTOR5_Addr;
  } else if (address >= (void *)0x08010000) {
    sector->sector_number = 4;
    sector->size = 64 * 1024;
    sector->address = FLASH_SECTOR4_Addr;
  } else if (address >= (void *)0x0800c000) {
    sector->sector_number = 3;
    sector->size = 16 * 1024;
    sector->address = FLASH_SECTOR3_Addr;
  } else if (address >= (void *)0x08008000) {
    sector->sector_number = 2;
    sector->size = 16 * 1024;
    sector->address = FLASH_SECTOR2_Addr;
  } else if (address >= (void *)0x08004000) {
    sector->sector_number = 1;
    sector->size = 16 * 1024;
    sector->address = FLASH_SECTOR1_Addr;
  } else {
    sector->sector_number = 0;
    sector->size = 16 * 1024;
    sector->address = FLASH_SECTOR0_Addr;
  }

  return 0;
}

int8_t metadata_block_init(MetaData_block_t *mdb) {

  mdb->start_word = 0xffffffff;
  for (uint8_t i = 0; i < MAX_FLAG_COUNT; i++) {
    mdb->flags[i] = FLAGS_INIT_VALUE;
  }

  return 0;
}

int8_t keyAddr_pair_init(KeyAddr_pair_t *kap, uint32_t key, void *addr) {

  if (key == VALID_FLAG || addr < (void *)0x08000000 ||
      addr > (void *)0x08080000) {

    return -1;
  }

  kap->key = key;
  kap->address = addr;
  kap->valid = 0x0fffffff;

  return 0;
}

/* array, size is to init rr-> q*/

void FSM_record_request_init(FSM_record_request_t *rr, volatile Ring_buff_t *rb,
                             queue_t *q) {
  rr->rb = rb, rr->q = q;
}


void FSM_write_buffer_init (FSM_write_buffer_t *fsm_wb, uint8_t *buffer, uint32_t size){
  fsm_wb -> data = buffer;
  fsm_wb -> size = size;

}

void FSM_request_pair_init (FSM_request_pair_t *rp, uint8_t *key, uint32_t size){
  
  rp-> key = key;
  rp-> size = size;

}
