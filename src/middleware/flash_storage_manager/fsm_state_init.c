#include "flash_storage_manager.h"
#include "queue.h"
#include "ring_buff.h"
#include <stdatomic.h>


bool FSM_Packet_init(FSM_Packet_t *pkt, uint32_t *data_buff, uint16_t data_size) {

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
  fsm_wb -> capacity = size;
  fsm_wb -> size = 0;

}

// to write packet and data into the write buffer, we need to have packet and
// ring buffer where the data lives !!! 
// data will be extraced from the write buffer
uint8_t FSM_write_buffer_write (FSM_write_buffer_t *fsm_wb, FSM_Packet_header_t*
                        fsm_packet, Ring_buff_t *rb, uint32_t data_size){
  // limit check
  if (data_size > MAX_PACKET_SIZE - sizeof (FSM_Packet_header_t))
    return 1;

  // if cannot be written to the wb, then copy the whole data into flash and continue
  // todo !!!!!
  

  // fsm_wb-> size is in bytes
  uint8_t *wb_end = (uint8_t *)fsm_wb + fsm_wb -> size;

  // copy packet ->  
  *((uint32_t *) wb_end++) = fsm_packet-> data_size;
  *((FSM_Packet_header_t **) wb_end++) = fsm_packet-> next_packet;
  *((uint32_t *) wb_end++) = fsm_packet-> removed;
  *((uint32_t *) wb_end++) = fsm_packet-> valid;


  uint32_t temp_size = MAX_PACKET_SIZE - sizeof (FSM_Packet_header_t);
  if (temp_size > data_size ) temp_size = data_size;

  uint8_t data[temp_size];
  Ring_buff_read(rb, data, temp_size);
 
  for (int i=0; i<data_size; i++){
    *(wb_end++) = data[i];
  }

  // update the size of fsm_wb
  fsm_wb -> size += temp_size + sizeof (FSM_Packet_header_t);

  return 0;
}




void FSM_request_pair_init (FSM_request_pair_t *rp, uint8_t *key, uint32_t size){
  
  rp-> key = key;
  rp-> size = size;

}
