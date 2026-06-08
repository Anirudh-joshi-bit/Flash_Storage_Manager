#include "flash.h" // for sector addresses
#include "flash_storage_manager.h"
#include "queue.h"
#include "ring_buff.h"


/* 
 note !!! for setting functions of packet_header and metadata_header

 the fileds are not cleared before writing the value as most of 
 the time packet header is located in the   flash ... clearing bits 
 will result in all bits of the field to set to 0... writing / setting a 
 bit in the field will require erasing the entire flash sector

 option -> set (0), set (value)   follow this sequence if you want to clear 
 the field before writting to it 
*/

// packet_header get functions
uint32_t FSM_packet_get_size (FSM_Packet_header_t* pkt_header){
  return pkt_header-> packet_descriptor & FSM_PACKET_DESCRIPTOR_SIZE_MSK;
}
// return vlaue will not relocate the head  sectionn to the front !!!
uint32_t FSM_packet_get_head (FSM_Packet_header_t* pkt_header){
  return pkt_header-> packet_descriptor & FSM_PACKET_DESCRIPTOR_HEAD_MSK;
}
bool FSM_packet_is_removed (FSM_Packet_header_t* pkt_header){
  return !(pkt_header-> packet_descriptor &
                  FSM_PACKET_DESCRIPTOR_NREMOVED_MSK);
}
bool FSM_packet_is_valid (FSM_Packet_header_t* pkt_header){
  return !(pkt_header-> packet_descriptor &
                  FSM_PACKET_DESCRIPTOR_VALID_MSK);
}

// set fuinctions => (packet, metadata)
void FSM_packet_set_size (FSM_Packet_header_t* pkt_header, uint32_t size){
  size &= FSM_PACKET_DESCRIPTOR_SIZE_MSK;
  pkt_header-> packet_descriptor |= size;
}

void FSM_packet_set_head (FSM_Packet_header_t* pkt_header, uint32_t head){
  head &= FSM_PACKET_DESCRIPTOR_HEAD_MSK;
  pkt_header-> packet_descriptor |= head;
}

void FSM_metadata_set_head (FSM_MetaData_header_t* md_header, uint32_t head){
  head &= FSM_METADATA_DESCRIPTOR_HEAD_MSK;
  md_header-> metadata_descriptor |= head;
}

void FSM_metadata_set_size (FSM_MetaData_header_t* md_header, uint32_t size){
  size &= FSM_METADATA_DESCRIPTOR_SIZE_MSK;
  md_header-> metadata_descriptor |= size;
}

// metadata_header get functions
uint32_t FSM_metadata_get_size (FSM_MetaData_header_t* md_header){
 return md_header-> metadata_descriptor & FSM_METADATA_DESCRIPTOR_SIZE_MSK;
}
// return vlaue will not relocate the head sectionn to the front !!!
uint32_t FSM_metadata_get_header (FSM_MetaData_header_t* md_header){
 return md_header-> metadata_descriptor & FSM_METADATA_DESCRIPTOR_HEAD_MSK;
}
bool FSM_metadata_is_valid (FSM_MetaData_header_t* md_header){
 return md_header-> metadata_descriptor & FSM_METADATA_DESCRIPTOR_VALID_MSK;
}



// init functins -> 
void FSM_Packet_init(FSM_Packet_header_t *pkt, uint16_t data_size) {

  uint32_t min_size = data_size + sizeof(FSM_Packet_header_t);
  if (min_size > MAX_PACKET_SIZE)
    min_size = MAX_PACKET_SIZE;

  pkt->packet_descriptor = 0;
  pkt->packet_descriptor |= (FSM_PKT_HEAD & FSM_PACKET_DESCRIPTOR_HEAD_MSK) |
                            FSM_PACKET_DESCRIPTOR_VALID_MSK |
                            FSM_PACKET_DESCRIPTOR_NREMOVED_MSK |
                            (min_size & FSM_PACKET_DESCRIPTOR_SIZE_MSK);
}

void metadata_header_init(FSM_MetaData_header_t *mdh, uint32_t metadata_size) {

  mdh->metadata_descriptor = 0;
  mdh->metadata_descriptor |= (FSM_MD_HEAD & FSM_METADATA_DESCRIPTOR_HEAD_MSK)|
                              FSM_METADATA_DESCRIPTOR_VALID_MSK |
                          (metadata_size & FSM_METADATA_DESCRIPTOR_SIZE_MSK);
}

int8_t sector_init(FSM_Sector_t *sector, void *address) {

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

int8_t record_metadata_init(FSM_record_metadata_t *kap, uint32_t key,
                            void *addr) {

  if (key == VALID_FLAG || addr < (void *)0x08000000 ||
      addr > (void *)0x08080000) {

    return -1;
  }

  kap->key = key;
  kap->address = addr;
  kap->removed = 0x0fffffff;

  return 0;
}

/* array, size is to init rr-> q*/

void FSM_record_request_init(FSM_record_request_t *rr, volatile Ring_buff_t *rb,
                             queue_t *q) {

  rr->rb = rb, rr->q = q;
}

void FSM_write_buffer_init(FSM_write_buffer_t *fsm_wb, uint8_t *buffer,
                           uint32_t size) {
  fsm_wb->data = buffer;
  fsm_wb->capacity = size;
  fsm_wb->size = 0;
}

void FSM_request_pair_init(FSM_request_pair_t *rp, uint8_t *key,
                           uint32_t size) {

  rp->key = key;
  rp->size = size;
}

// better package these addresses in a structure (fsm)
bool FSM_init(FSM_Sector_t flash_sectors[8], FSM_write_buffer_t *fsm_wb,
              uint8_t wb[FSM_WRITE_BUFFER_SIZE],
              FSM_MetaData_header_t *metadata_in_ram,
              FSM_MetaData_header_t *metadata_in_flash) {

  sector_init(&flash_sectors[0], (void *)FLASH_SECTOR0_Addr);
  sector_init(&flash_sectors[1], (void *)FLASH_SECTOR1_Addr);
  sector_init(&flash_sectors[2], (void *)FLASH_SECTOR2_Addr);
  sector_init(&flash_sectors[3], (void *)FLASH_SECTOR3_Addr);
  sector_init(&flash_sectors[4], (void *)FLASH_SECTOR4_Addr);
  sector_init(&flash_sectors[5], (void *)FLASH_SECTOR5_Addr);
  sector_init(&flash_sectors[6], (void *)FLASH_SECTOR6_Addr);
  sector_init(&flash_sectors[7], (void *)FLASH_SECTOR7_Addr);

  FSM_write_buffer_init(fsm_wb, wb, FSM_WRITE_BUFFER_SIZE);
  metadata_header_init(metadata_in_ram, 0); // check

  // find the gc_sector
  uint32_t *gc_sector = flash_get_sector_address(5);
  if (*gc_sector == 0xffffffff)
    gc_sector = flash_get_sector_address(6);
  if (*gc_sector == 0xffffffff)
    gc_sector = flash_get_sector_address(7);

  // make the array ...... from flssh_metadata

  // when a metadata block is dropped, the first field is set to 0x0, and
  // the latest  metadata   block starts with 0xffffffff

  uint32_t *metadata_sector_address = flash_get_sector_address(1);
  uint32_t metadata_sector_size = flash_get_sector_size(1);
  uint32_t *iter_flash = metadata_sector_address;

  // put metadata into ram
  while (iter_flash < metadata_sector_address + metadata_sector_size) {
    // search for FSM_MD_PKT_HEAD = oxfffffffe
    if (*iter_flash == 0xffffffff) {
      // powerloss | metadata has never been written
      // case 1 powerloss
      // search in the gc_sector;
    }

    // assuming all the metadata  blocks are consistant->
    // structure => metadata_header   buffer(of size = header->md buffer size)

    // todo
    // break this for now !!!!!
    break;
  }

  return true;
}
