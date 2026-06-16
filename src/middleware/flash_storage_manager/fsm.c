#include "flash.h" // for sector addresses
#include "flash_storage_manager.h"
#include "queue.h"
#include "ring_buff.h"
#include "garbage_collector.h"
#include "stm32f401xe.h"



// functions declarations 
bool FSM_copy_metadata_to_md_sector_helper (void *dest, void *src);

//#define DEBUG

// debug
#ifdef DEBUG
void printf(void (*usartx_print) (const char*, uint32_t), const char *msg, ...);
void __usart1_print(const char *msg, uint32_t size);
#endif
// debug end


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
// assumption -> 
// 0. size returned is in bytes
uint32_t FSM_packet_get_size(FSM_Packet_header_t *pkt_header) {
  return pkt_header->packet_descriptor & FSM_PACKET_DESCRIPTOR_SIZE_MSK;
}
// return vlaue will not relocate the head  sectionn to the front !!!
uint32_t FSM_packet_get_head(FSM_Packet_header_t *pkt_header) {
  return pkt_header->packet_descriptor & FSM_PACKET_DESCRIPTOR_HEAD_MSK;
}
bool FSM_packet_is_removed(FSM_Packet_header_t *pkt_header) {
  return !(pkt_header->packet_descriptor & FSM_PACKET_DESCRIPTOR_NREMOVED_MSK);
}
bool FSM_packet_is_valid(FSM_Packet_header_t *pkt_header) {
  return !(pkt_header->packet_descriptor & FSM_PACKET_DESCRIPTOR_VALID_MSK);
}

// set fuinctions => (packet, metadata)
void FSM_packet_set_size(FSM_Packet_header_t *pkt_header, uint32_t size) {
  size &= FSM_PACKET_DESCRIPTOR_SIZE_MSK;
  pkt_header->packet_descriptor |= size;
}

void FSM_packet_set_head(FSM_Packet_header_t *pkt_header, uint32_t head) {
  head &= FSM_PACKET_DESCRIPTOR_HEAD_MSK;
  pkt_header->packet_descriptor |= head;
}

void FSM_metadata_set_head(FSM_MetaData_header_t *md_header, uint32_t head) {
  head &= FSM_METADATA_DESCRIPTOR_HEAD_MSK;
  md_header->metadata_descriptor |= head;
}

void FSM_metadata_set_size(FSM_MetaData_header_t *md_header, uint32_t size) {
  size &= FSM_METADATA_DESCRIPTOR_SIZE_MSK;
  md_header->metadata_descriptor |= size;
}

// metadata_header get functions
// assumptions -> 
// 0. size is in bytes 
uint32_t FSM_metadata_get_size(FSM_MetaData_header_t *md_header) {
  return md_header->metadata_descriptor & FSM_METADATA_DESCRIPTOR_SIZE_MSK;
}
// return vlaue will not relocate the head sectionn to the front !!!
uint32_t FSM_metadata_get_header(FSM_MetaData_header_t *md_header) {
  return md_header->metadata_descriptor & FSM_METADATA_DESCRIPTOR_HEAD_MSK;
}
bool FSM_metadata_is_valid(FSM_MetaData_header_t *md_header) {
  return md_header->metadata_descriptor & FSM_METADATA_DESCRIPTOR_VALID_MSK;
}



/*  
 *  assumption =>
 *    0. first word of the gc_sector is invalid and used to identify gc sector
 *    1. gc_sector_address is the starting address of a sector (gc sector)
 *    2. if this function is called it is assumed that metadata is written
 *      completely in the gc sector
 *    3. packet can be incompletely written
 *
 *  side effects =>
 *    0. reads flash
 *
 *  returns => 
 *    0. NULL on failure
 *    1. metadata header address on success
 *
 */

// check !!!
FSM_MetaData_header_t* find_last_metadata_in_gc (uint32_t * gc_sector_add){
  flash_get_sector (gc_sector_add);
  uint32_t *gc_end_add = gc_sector_add + 
                  flash_get_sector_size(flash_get_sector (gc_sector_add));
  
  uint32_t *gc_iter = ++gc_sector_add; // assumption 0
  FSM_MetaData_header_t* last_metadata = NULL;

  while (gc_iter < gc_end_add){
    if ((*gc_iter & FSM_METADATA_DESCRIPTOR_HEAD_MSK) == FSM_MD_HEAD_CO){
      // this is a metadata
      last_metadata = (FSM_MetaData_header_t *)gc_iter;
      uint32_t md_size = 
                FSM_metadata_get_size ((FSM_MetaData_header_t*) gc_iter);
      gc_iter += md_size/4;
    }else
    if (((*gc_iter & FSM_PACKET_DESCRIPTOR_HEAD_MSK) == FSM_PKT_HEAD_CO) || 
       ((*gc_iter & FSM_PACKET_DESCRIPTOR_HEAD_MSK) == FSM_PKT_HEAD_INCO)){
      // this is a packet
      uint32_t pkt_size = 
                FSM_packet_get_size ((FSM_Packet_header_t*) gc_iter);
      gc_iter += pkt_size/4;
    }
  }
  return last_metadata;
}


// init functins ->
void FSM_Packet_init(FSM_Packet_header_t *pkt, uint16_t data_size) {

  uint32_t min_size = data_size + sizeof(FSM_Packet_header_t);
  if (min_size > MAX_PACKET_SIZE)
    min_size = MAX_PACKET_SIZE;

  pkt->packet_descriptor = 0;
  pkt->packet_descriptor |= (FSM_PKT_HEAD_INCO &
                            FSM_PACKET_DESCRIPTOR_HEAD_MSK) |
                            FSM_PACKET_DESCRIPTOR_VALID_MSK |
                            FSM_PACKET_DESCRIPTOR_NREMOVED_MSK |
                            (min_size & FSM_PACKET_DESCRIPTOR_SIZE_MSK);
}

void metadata_header_init(FSM_MetaData_header_t *mdh, uint32_t metadata_size) {

  mdh->metadata_descriptor = 0;
  mdh->metadata_descriptor |=
      (FSM_MD_HEAD_INCO & FSM_METADATA_DESCRIPTOR_HEAD_MSK) |
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
              FSM_MetaData_header_t *metadata_in_flash,
              FSM_addresses_t* addresses) {

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
  uint32_t *gc_sector = flash_get_sector_address(7);
  if (*gc_sector != FSM_GC_SECTOR_ID)
    gc_sector = flash_get_sector_address(6);
  if (*gc_sector != FSM_GC_SECTOR_ID)
    gc_sector = flash_get_sector_address(5);




    // init the addresses 









  // make the array ...... from flssh_metadata

  // when a metadata block is dropped, the first field is set to 0x0, and
  // the latest  metadata   block starts with 0xffffffff

  uint32_t *metadata_sector_address = flash_get_sector_address(1);
  uint32_t metadata_sector_size = flash_get_sector_size(1);
  uint32_t *iter_flash = metadata_sector_address;

  // prev metadata (if the current one is incomplete)
  FSM_MetaData_header_t *prev_metadata = NULL;

  // put metadata into ram
  
  while (iter_flash < metadata_sector_address + metadata_sector_size) {
    //  search the metadata
    uint32_t md_sector_header = *iter_flash & 
                          FSM_METADATA_DESCRIPTOR_HEAD_MSK;
    if (*iter_flash == 0xffffffff) {
      // fsm has never started     
      metadata_header_init (metadata_in_ram, 0);
      metadata_in_flash =
              (FSM_MetaData_header_t*)flash_get_sector_address (1);
      break;
    }

    if (md_sector_header != FSM_MD_HEAD_CO){
      if (prev_metadata == NULL){
        FSM_MetaData_header_t* metadata_in_gc =
                                  find_last_metadata_in_gc (gc_sector);
        if (metadata_in_gc == NULL){
        #ifdef DEBUG
          printf (__usart1_print, "metadata not completed, prev md == null, \
                              md not present in the gc");
        #endif
          return 0;
        }

        // copy the metadata from gc to md sector
        metadata_in_flash = metadata_in_gc;
        //FSM_copy_metadata_to_md_sector ();
      }
    }
    else {

    }
  }

  return true;
}


// caheck
// assumptions -> 
//  0. dest address is always in the flash region
//  1. src -> range (RAM | FLASH)
//  2. gc sector end address exists
//  
//side effect -> 
//  0. write to flash 
//  1. possibly read flash
//  2. may erase metadata sector
//
bool FSM_copy_metadata_to_md_sector (void *src,void *dest,
                                FSM_addresses_t* addresses){

// debug mode  => write all the values

  uint32_t md_size = FSM_metadata_get_size ((FSM_MetaData_header_t*)src);
  uint32_t metadata_sector = flash_get_sector(src);
  void *md_sector_address =  flash_get_sector_address (metadata_sector);
  void *md_sector_end_address = md_sector_address + 
                flash_get_sector_size(flash_get_sector (md_sector_address));

  if (dest + md_size >= md_sector_end_address){
    #ifdef DEBUG
    printf (__usart1_print, "FSM_copy_metadata_to_md_sector -> \
        no space for writing md in src ... erasing md sector");
    #endif
    
    // 1. src -> gc_end_address -> possible -> write 
    //                               else -> erase gc_sector and write
    // 2. erase the md sector 
    // 3. recursion
  
    uint32_t gc_sector_size = flash_get_sector_size (gc_sector);
    uint32_t gc_remaining_size = gc_sector_size - 
        ((uint32_t)(addresses -> gc_end_add - addresses-> gc_sector_add));

    if (gc_remaining_size <= md_size){
      // erase the gc_sector firsta
      flash_erase (addresses-> gc_sector_add);
      addresses -> gc_end_add = addresses -> gc_sector_add;
    }
    // write md to gc_sector
    FSM_copy_metadata_to_md_sector_helper (src, addresses->gc_end_add);

    // erase md_ector
    flash_erase (md_sector_address);

    // recursion 
    FSM_copy_metadata_to_md_sector (addresses->gc_end_add, dest, addresses);

    // update gc_end_add , md_end_add
    addresses-> gc_end_add += md_size;
    addresses-> md_end_add += md_size;

  }
 
  FSM_copy_metadata_to_md_sector_helper (src, dest);

  addresses-> md_end_add += md_size;
    return 1;
}
 
bool FSM_copy_metadata_to_md_sector_helper (void *src, void *dest) {

  // destination is always a flassh address !!! 
  // if the content is ffffffff and we want to write  
  // anything -> error (debug)
  
  // assuming first descriptor is 4byte long

  uint32_t md_size = FSM_metadata_get_size ((FSM_MetaData_header_t*)src);
  uint32_t metadata_descriptor = *(uint32_t *)dest;
  void *iter = src;

  #ifdef DEBUG
  if ((metadata_descriptor & FSM_METADATA_DESCRIPTOR_HEAD_MSK) != 
      FSM_MD_HEAD_CO && flash_get_sector(dest) != -1)  {

    printf (__usart1_print, "in FSM_copy_metadata_to_md_sector function,"
            "src metadata is not written completely\n\r");
  }
  #endif

  metadata_descriptor &= ~FSM_METADATA_DESCRIPTOR_HEAD_MSK; // clear the head
  metadata_descriptor |= FSM_MD_HEAD_INCO; // set incomplete bits
 

  #ifdef DEBUG
    if (*(uint32_t *)dest != 0xffffffff){
      printf (__usart1_print, "error in FSM_copy_metadata_to_md_sector,\
          trying to write to an unerased address in flash\n\r");
    }
  #endif

  *(uint32_t *)(iter) = metadata_descriptor;
  iter += 4;
  dest += 4;

  #ifdef DBUG
    while (iter != (src + md_size)){
  #endif

  #ifndef DEBUG
  while (iter < (src + md_size) ){
  #endif 

  #ifdef DEBUG
    if (*(uint32_t *)dest != 0xffffffff){
      printf (__usart1_print, "error in FSM_copy_metadata_to_md_sector,\
          trying to write to an unerased address in flash\n\r");
    }
  #endif
    
    // if word writable
    if ((src+md_size) - iter >= 4){
      *(uint32_t *) dest = *(uint32_t *) iter;
      iter += 4;
      dest += 4;
    }
    else {
      // assuming littleendian
      // make a packet and write it to the flash
      uint32_t word = 0xffffffff;
      uint32_t word_data = *(uint32_t *)(iter);
      // clear the most significant byte 
      uint32_t remaining = (src+md_size) - iter;

      // check 
      word_data |= word << (4-remaining) * 4;

      *(uint32_t *) dest = word_data;
      iter += 4;
      dest += 4;
    }
  }
  
  // complete the transaction 
  
  
  return 1;
}


