#include "flash.h" 
#include "flash_storage_manager.h"
#include "queue.h"
#include "ring_buff.h"
#include "garbage_collector.h"
#include "DEBUG.h"

// functions declaration
bool FSM_copy_packet_to_log (void *src, void *dest, 
        FSM_addresses_t *addresses, FSM_record_metadata_t *last_packet_arr);
bool FSM_copy_packet_to_log_helper  (void *src, void *dest);

FSM_MetaData_header_t* find_last_metadata_in_gc (void * gc_sector_add);
void *FSM_get_gc_end_add (uint8_t sector);
void *FSM_get_md_end_add (uint8_t sector,
              FSM_MetaData_header_t** latest_metadata);
void *FSM_get_log_end_add (uint8_t st_sector, uint8_t en_sector);
bool FSM_copy_metadata_to_md_sector (void *src,void *dest,
                                FSM_addresses_t* addresses);
bool FSM_copy_metadata_to_md_sector_helper (void *dest, void *src);

bool FSM_make_last_packet_arr (FSM_record_metadata_t* arr, 
                                FSM_addresses_t *add, void *metadata);

FSM_Packet_header_t* FSM_get_last_packet 
                (FSM_Packet_header_t* start_packet);

void *skip_the_garbage (void *iter, void *end);

bool FSM_flash_write_wrapper (uint32_t *buff, uint32_t size, void *address);
bool FSM_flash_erase_wrapper (void *address);

bool execute_time_taking_tasks1 ();   // less time expensive, these function cannot use flash_write_wraper
bool execute_time_taking_tasks2 ();   // more time expensive, these function cannot use flash_etase_wrapper

bool FSM_sequence_table_fill (FSM_MetaData_header_t* metadata);

bool FSM_copy_metadata_to_ram (void *src, void *dest);


// debug
#ifdef DEBUG
void printf(void (*usartx_print) (const char*, uint32_t),
          const char *msg, ...);
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

// md, pkt common functions
uint32_t FSM_md_pkt_get_head(void *header) {
  // or packet
  return ((FSM_MetaData_header_t *)header)->metadata_descriptor &
              FSM_MD_PKT_DESCRIPTOR_HEAD_MSK;
}
void FSM_md_pkt_set_head(void *header, uint32_t head) {
  head &= FSM_MD_PKT_DESCRIPTOR_HEAD_MSK;
  // or packet
  ((FSM_MetaData_header_t *)header)->metadata_descriptor |= head;
}


// packet getters and setters
uint32_t FSM_packet_get_size(FSM_Packet_header_t *pkt_header) {
  return pkt_header->packet_descriptor & FSM_PACKET_DESCRIPTOR_SIZE_MSK;
}
bool FSM_packet_is_removed(FSM_Packet_header_t *pkt_header) {
  return !(pkt_header->packet_descriptor &
      FSM_PACKET_DESCRIPTOR_NREMOVED_MSK);
}
bool FSM_packet_is_valid(FSM_Packet_header_t *pkt_header) {
  return !(pkt_header->packet_descriptor & FSM_PACKET_DESCRIPTOR_VALID_MSK);
}
void FSM_packet_set_size(FSM_Packet_header_t *pkt_header, uint32_t size) {
  size &= FSM_PACKET_DESCRIPTOR_SIZE_MSK;
  pkt_header->packet_descriptor |= size;
}
uint32_t FSM_packet_get_selfseqno (FSM_Packet_header_t* packet){
  return (packet->seq_info & FSM_PACKET_SEQINFO_SELFSEQ_MSK) >> (32-11);
}
uint32_t FSM_packet_get_nextseqno (FSM_Packet_header_t* packet){
  return (packet->seq_info & FSM_PACKET_SEQINFO_NEXTSEQ_MSK);
}


// metadaata getters and setters
void FSM_metadata_set_size(FSM_MetaData_header_t *md_header, uint32_t size) {
  size &= FSM_METADATA_DESCRIPTOR_SIZE_MSK;
  md_header->metadata_descriptor |= size;
}
uint32_t FSM_metadata_get_size(FSM_MetaData_header_t *md_header) {
  return md_header->metadata_descriptor & FSM_METADATA_DESCRIPTOR_SIZE_MSK;
}
bool FSM_metadata_is_valid(FSM_MetaData_header_t *md_header) {
  return md_header->metadata_descriptor & FSM_METADATA_DESCRIPTOR_VALID_MSK;
}

// record metadata
uint32_t FSM_record_md_get_seqno (FSM_record_metadata_t* rmd){
  return (rmd->md_entry & FSM_RECORD_MD_SEQ_MSK) ;
}
uint32_t FSM_record_md_get_key (FSM_record_metadata_t* rmd){
  return (rmd->md_entry & FSM_RECORD_MD_KEY_MSK) >> (32-11);
}

/*check*/
// seq_table functions 
// ... no setter functions for now as seq table is mostly present in flash
bool FSM_sequence_table_iscomplete (FSM_sequence_table_t *st){
  return (st->seq_tale_header & FSM_SEQUENCE_TABLE_ID_MSK) == 
                  FSM_SEQUENCE_TABLE_CO;
}
bool FSM_sequence_table_isvalid (FSM_sequence_table_t* st){
  return st->seq_tale_header & FSM_SEQUENCE_TABLE_VALID_MSK;
}
bool FSM_sequence_table_isoccupied (FSM_sequence_table_t* st,
                                            uint32_t seq_num){
  if (seq_num > FSM_MAX_SEQ_NUM) return NULL;

  uint32_t *table_iter = (void*)st + 4;
  table_iter += seq_num ;
  uint32_t word = *(uint32_t *)table_iter;
  
  return word & FSM_SEQUENCE_TABLE_OCCUPIED_MSK;
}
void *FSM_sequence_table_get_address (FSM_sequence_table_t* st,
                                            uint32_t seq_num){
  if (seq_num > FSM_MAX_SEQ_NUM) return NULL;

  uint32_t *table_iter = (void*)st+4;
  table_iter += seq_num;
  uint32_t word = *(uint32_t *) table_iter;

  uint32_t offset = word & (FSM_SEQUENCE_TABLE_OFFSET_MSK);
  return (void*) 0x08000000 + offset;
}

// make sure that sequence table is valid before calling this function
FSM_Packet_header_t* FSM_get_next_packet (FSM_Packet_header_t *packet,
                        FSM_sequence_table_t *st){
  /*TODO*/
  uint32_t nextpacket_seqnum = FSM_packet_get_nextseqno(packet);
  if (!FSM_sequence_table_isoccupied(st, nextpacket_seqnum)){
    DEBUG_printf (__usart1_print, "in FSM_get_next_packet function, nextpacket_seqnum is not occupied");
    return NULL;
  }
  return FSM_sequence_table_get_address(st, nextpacket_seqnum);
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



// init functins ->
bool FSM_Packet_header_init(FSM_Packet_header_t *pkt, uint16_t data_size) {
  
  if (data_size > MAX_PACKET_SIZE - sizeof (FSM_Packet_header_t)){
    printf (__usart1_print, "[ERROR]    in FSM_Packet_init function, data "
                  "size is too big \n\r");
    return false;
  }
  
  uint32_t min_size = data_size + sizeof(FSM_Packet_header_t);
  if (min_size > MAX_PACKET_SIZE)
    min_size = MAX_PACKET_SIZE;

  pkt->packet_descriptor = 0;
  pkt->packet_descriptor |= (FSM_PKT_HEAD_INCO &
                            FSM_MD_PKT_DESCRIPTOR_HEAD_MSK) |
                            FSM_PACKET_DESCRIPTOR_VALID_MSK |
                            FSM_PACKET_DESCRIPTOR_NREMOVED_MSK |
                            (min_size & FSM_PACKET_DESCRIPTOR_SIZE_MSK);
  return true;
}

// always called for metadata in ram => complete
bool metadata_header_init(FSM_MetaData_header_t *mdh, 
                                  uint32_t metadata_size) {

  if (metadata_size > MAX_RECORD_COUNT * sizeof (FSM_record_metadata_t)-
      sizeof (FSM_MetaData_header_t)){
        return 0;
      }
  
  mdh->metadata_descriptor = 0;
  mdh->metadata_descriptor |=
      (FSM_MD_HEAD_CO & FSM_MD_PKT_DESCRIPTOR_HEAD_MSK) |
      FSM_METADATA_DESCRIPTOR_VALID_MSK |
      (metadata_size & FSM_METADATA_DESCRIPTOR_SIZE_MSK);
  return 1;
}

bool FSM_sequence_table_init (FSM_sequence_table_t *st){
  // not needed 
  for (uint32_t i=0; i< FSM_SEQUENCE_TABLE_ARR_SIZE; i++){
    st-> table[i] = 0x0;
  }
  return true;
}

bool sector_init(FSM_Sector_t *sector, void *address) {

  if (address < (void *)0x08000000 || address > (void *)0x08080000) {
    return false;
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

  return true;
}

bool record_metadata_init(FSM_record_metadata_t *kap,
                            uint32_t key, void *addr)
{
  if (addr < (void *)0x08000000 ||
      addr > (void *)0x08080000) 
  {
     return false;
  }
  kap->key = key | FSM_RECORD_REMOVED_MSK;
  kap->packet_add= addr;

  return true;
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
              FSM_addresses_t* addresses,
              FSM_record_metadata_t *last_packet_arr,
              uint32_t *number_record){


  sector_init(&flash_sectors[0], (void *)FLASH_SECTOR0_Addr);
  sector_init(&flash_sectors[1], (void *)FLASH_SECTOR1_Addr);
  sector_init(&flash_sectors[2], (void *)FLASH_SECTOR2_Addr);
  sector_init(&flash_sectors[3], (void *)FLASH_SECTOR3_Addr);
  sector_init(&flash_sectors[4], (void *)FLASH_SECTOR4_Addr);
  sector_init(&flash_sectors[5], (void *)FLASH_SECTOR5_Addr);
  sector_init(&flash_sectors[6], (void *)FLASH_SECTOR6_Addr);
  sector_init(&flash_sectors[7], (void *)FLASH_SECTOR7_Addr);

  FSM_write_buffer_init(fsm_wb, wb, FSM_WRITE_BUFFER_SIZE);
  FSM_sequence_table_init (addresses->latest_sequence_table_add_flash);
  
  //metadata_header_init(metadata_in_ram, 0); // check

  // find the gc_sector
  void *gc_sector = flash_get_sector_address(7);
  // if (*(uint32_t *)gc_sector != (uint32_t)FSM_GC_SECTOR_ID)
  //   gc_sector = flash_get_sector_address(6);
  // if (*(uint32_t *)gc_sector != (uint32_t)FSM_GC_SECTOR_ID)
  //   gc_sector = flash_get_sector_address(5);
  

  FSM_MetaData_header_t* latest_metadata = NULL;


  // init the addresses
  addresses -> gc_sector_add = flash_get_sector_address(7);
  // first word of gc if 0xffffffff
  addresses -> gc_end_add = FSM_get_gc_end_add(7);

  if (addresses-> gc_end_add >= flash_get_sector_address(7) +
          flash_get_sector_size(7)){

    DEBUG_printf (__usart1_print, "in FSM_init function, gc_end is "
          "exceeding the 7th sector boundary \n\r");
    flash_erase ((void *)addresses->gc_sector_add);
    addresses -> gc_end_add = addresses-> gc_sector_add + 4;
  }
  // sector 1 is for metadata
  addresses -> md_sector_add = flash_get_sector_address(1);
  addresses -> md_end_add = FSM_get_md_end_add (1, &latest_metadata);
  addresses -> log_end_add = FSM_get_log_end_add (2, 6);


  // populate md_ram and md_flash
  if (latest_metadata == NULL){     
    // _______________________________________there is no metadata in md_sector 
    // there is no metadata | metadata is in the gc sector and power was
    // lost in prev reset before  completely writing the md to md sector
    
    //TODO check this function 
    latest_metadata = find_last_metadata_in_gc(addresses -> gc_sector_add);
    if (!latest_metadata){
      // _______________________________________latest metadata is not in the gc sector
      metadata_header_init(addresses -> latest_md_add_ram,
            sizeof (FSM_MetaData_header_t)); // init metadata in ram

      addresses -> latest_md_add_flash = addresses-> md_end_add; 
      FSM_copy_metadata_to_md_sector(addresses-> latest_md_add_ram, addresses->md_end_add, addresses);
    
    DEBUG_printf(__usart1_print, "inside function FSM_ini, latest"
        "metadata is NULL and there is no complete metadata is gc, value of"
        "md_end_add = %x\n\r", addresses -> md_end_add);

      addresses -> md_end_add = addresses -> md_sector_add +
                        sizeof (FSM_MetaData_header_t);
    }
    else {
      // _______________________________________latest metadata is in the gc sector
      // latest_metadata is a valid metadata but lies in the gc sector 
      // a valid md found in gc sector
      
      DEBUG_printf (__usart1_print, "inside function FSM_ini, a valid"
            " md found in the gc sector\n\r");
      
      /*if metadata is found is the gc sector, and there is no valid metadata in 
       * metadata sector, then erase the md_sector to save future gc operation
       *  but this hurts the wearleveling for md sector
       *  erase part can be removed
       * */
      
// note   
      flash_erase (addresses->md_sector_add);      // note... whenever erasing a sector -> increament the erase count for that sector
      addresses -> md_end_add = addresses -> md_sector_add;
//

// note
      addresses-> latest_md_add_flash = addresses -> md_end_add;
      FSM_copy_metadata_to_md_sector (latest_metadata,
            addresses-> md_end_add, addresses);
      //addresses -> md_end_add += FSM_metadata_get_size(latest_metadata);
      // metadta is copied from gc sector to md sector
//
      // make a function to copy md to ram
      FSM_copy_metadata_to_ram (addresses -> latest_md_add_flash, 
                                addresses -> latest_md_add_ram);
    }
  }
  
  else {
    // there is a valid latest_metadata in md_sector
    addresses -> latest_md_add_flash = latest_metadata;
    FSM_copy_metadata_to_ram (addresses -> latest_md_add_flash, 
                              addresses -> latest_md_add_ram);
  }




  // make the array ...... from flssh_metadata
  /*TODO*/
    /*
     * 1. read metadata and get teh record's first packet address
     * 2. for each record, follow the link and get to the last packet for
     * that  record
     * */

  // max size of the last packet array is
  //      FSM_MAX_RECORD_COUNT * sizeof (FSM_record_metadata_t);
  
  // using ram metadata to make the lastpacket arr
  if (FSM_metadata_get_size(addresses -> latest_md_add_ram) - sizeof (FSM_MetaData_header_t)
        >  FSM_MAX_RECORD_COUNT * sizeof (FSM_record_metadata_t)) 
  {
    printf (__usart1_print, "[WARNING]    there is too many records !!!\n\r");
    printf (__usart1_print, "[TIP]    increase the max record count \n\r");
    return false;
  }
  
  FSM_record_metadata_t *iter = addresses -> latest_md_add_ram+ 
                                sizeof (FSM_MetaData_header_t);
  FSM_record_metadata_t *iter_end = addresses -> latest_md_add_ram+
                                    FSM_metadata_get_size(addresses -> latest_md_add_ram);
  
  FSM_record_metadata_t *iter_last_packet_ar = last_packet_arr;

  while (iter < iter_end){ 
    *(FSM_record_metadata_t *)iter =
                        *(FSM_record_metadata_t *)iter_last_packet_ar;

    (*number_record) ++;   

    iter ++;
    iter_last_packet_ar ++;

  }

  DEBUG_printf (__usart1_print, "number of record = %d \n\r",
                                *number_record);

  if (!*number_record) return true;   // empty flash

  // read to get the last packet address 
  iter_last_packet_ar = last_packet_arr;
  FSM_record_metadata_t* iter_last_packet_ar_end =
              (void *)last_packet_arr + *number_record;


  while (iter_last_packet_ar <= iter_last_packet_ar_end){
    //FSM_get_last_packet ();
    iter_last_packet_ar -> packet_add = 
                  FSM_get_last_packet (iter_last_packet_ar -> packet_add);
    iter_last_packet_ar ++;
  }


  FSM_sequence_table_fill (addresses -> latest_md_add_ram);
  /*TODO*/
  return true;
}

// check !!!
// gc_sector may have md / packet / sequence_table
FSM_MetaData_header_t* find_last_metadata_in_gc (void * gc_sector_add){

  void *gc_end_add = gc_sector_add +
                  flash_get_sector_size(flash_get_sector (gc_sector_add));

  void *gc_iter = gc_sector_add + 4;
  FSM_MetaData_header_t* last_metadata = NULL;

  while (gc_iter < gc_end_add){
    if ((*(uint32_t *)gc_iter & FSM_MD_PKT_DESCRIPTOR_HEAD_MSK) == FSM_MD_HEAD_CO){
      // this is a metadata
      last_metadata = (FSM_MetaData_header_t *)gc_iter;
      uint32_t md_size =
                FSM_metadata_get_size ((FSM_MetaData_header_t*) gc_iter);
      gc_iter += md_size;
    }else
    if (((*(uint32_t *)gc_iter & FSM_MD_PKT_DESCRIPTOR_HEAD_MSK) == FSM_PKT_HEAD_CO) ||
       ((*(uint32_t *)gc_iter & FSM_MD_PKT_DESCRIPTOR_HEAD_MSK) == FSM_PKT_HEAD_INCO)){
      // this is a packet
      uint32_t pkt_size =
                FSM_packet_get_size ((FSM_Packet_header_t*) gc_iter);
      gc_iter += pkt_size;
    }
    else 
      return last_metadata;
  }
  return last_metadata;
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
//  3. may change the content of addresses
bool FSM_copy_metadata_to_md_sector (void *src,void *dest,
                                FSM_addresses_t* addresses){

  uint32_t md_size = FSM_metadata_get_size ((FSM_MetaData_header_t*)src);
  uint32_t metadata_sector = flash_get_sector(dest);
  void *md_sector_address =  flash_get_sector_address (metadata_sector);
  void *md_sector_end_address = md_sector_address +
                flash_get_sector_size(flash_get_sector (md_sector_address));

  if (dest + md_size >= md_sector_end_address){

    DEBUG_printf (__usart1_print, "FSM_copy_metadata_to_md_sector -> \
        no space for writing md in src ... erasing md sector");

    // 1. src -> gc_end_address -> possible -> write
    //                               else -> erase gc_sector and write
    // 2. erase the md sector
    // 3. recursion

    uint32_t gc_sector_size = 
      flash_get_sector_size (flash_get_sector(addresses -> gc_sector_add));
    uint32_t gc_remaining_size = gc_sector_size -
        ((uint32_t)(addresses -> gc_end_add - addresses-> gc_sector_add));

    if (gc_remaining_size <= md_size){
      // erase the gc_sector firsta
      flash_erase ((void *)addresses-> gc_sector_add);
      addresses -> gc_end_add = addresses -> gc_sector_add + 4;
    }
    FSM_copy_metadata_to_md_sector_helper (src, addresses->gc_end_add);
    void *gc_end_prev = addresses -> gc_end_add;
    // update gc_end_add , md_end_add
    addresses-> gc_end_add += md_size;

    // erase md_ector
    flash_erase ((void *)md_sector_address);
    addresses -> md_end_add = addresses-> md_sector_add;

    // recursion
    FSM_copy_metadata_to_md_sector (gc_end_prev, addresses-> md_end_add, addresses);
    
    return true;
 
  }
  else {
    FSM_copy_metadata_to_md_sector_helper (src, dest);
    addresses-> md_end_add += md_size;
  }
    return true;
}

/*
 * assumption ->
 *  1. there is enough space to copy
 *  2. dest is in the flash range
 *
 * perpose -> copy the data from src to dest
 *
 * */

bool FSM_copy_metadata_to_md_sector_helper (void *src, void *dest) {
  if (src < (void *)FLASH_ST_ADDRESS || src >= (void *)FLASH_END_ADDRESS && src < (void *)RAM_ST_ADDRESS || src >= (void *)RAM_END_ADDRESS) {

  DEBUG_printf (__usart1_print, "in function FSM_copy_metadata_to_md_sector_helper, src is neither in flash nor in ram address range");

  DEBUG_assert(src < (void *)FLASH_ST_ADDRESS || src >= (void *)FLASH_END_ADDRESS && src < (void *)RAM_ST_ADDRESS || src >= (void *)RAM_END_ADDRESS);
  }
  
#ifdef DEBUG
  if (*(uint32_t *)dest != 0xffffffff) while (1);
#endif

  uint32_t md_size = FSM_metadata_get_size ((FSM_MetaData_header_t*)src);
  uint32_t metadata_descriptor = *(uint32_t *)src;

 // #ifdef DEBUG
  if ((metadata_descriptor & FSM_MD_PKT_DESCRIPTOR_HEAD_MSK) !=
      FSM_MD_HEAD_CO && flash_get_sector(dest) != -1)  {

    DEBUG_printf (__usart1_print, "in FSM_copy_metadata_to_md_sector"
        " function, src metadata is not written completely\n\r");
  }
 // #endif

  metadata_descriptor &= ~FSM_MD_PKT_DESCRIPTOR_HEAD_MSK; // clear the head
  metadata_descriptor |= FSM_MD_HEAD_INCO; // set incomplete bits

  // assuming metadata header is 4Bytes
  // copy metadata header
  if (sizeof (FSM_MetaData_header_t) != 4){
    DEBUG_printf (__usart1_print,"in function "  
        "FSM_copy_metadata_to_md_sector_helper, "
        "size of metadata_header_t is not 4, hence add a copy function "
        "to copy the metadata header to flash\n\r");
  }
   
  *(uint32_t *)(src) = metadata_descriptor; 
  // iter += 4;
  // dest += 4;
  

  // dont need a while loop
  
  flash_write (src, md_size/4 *4, dest);
  void *temp = md_size/4 *4 + src;

  // assuming littleendian
  // make a packet and write it to the flash
  uint32_t word_data = *(uint32_t *)(temp);
  // clear the most significant byte
  uint32_t remaining = (src+md_size) - temp;

  uint32_t word = 0xffffffff << (8*remaining);

  // check
  word_data |= word;

  flash_write (&word_data, 1, dest+md_size/4 *4);

  // complete the transaction 
  // check
  uint32_t complete_descriptor = 
    (FSM_MD_HEAD_CO | ~(FSM_MD_PKT_DESCRIPTOR_HEAD_MSK)) &  metadata_descriptor ;

  flash_write (&complete_descriptor, 1, dest);

  return 1;
}

// dest is in the ram address range
bool FSM_copy_metadata_to_ram (void *src, void *dest){
  // copy the header first
  *(FSM_MetaData_header_t* )dest = *(FSM_MetaData_header_t* ) src;
  src += sizeof (FSM_MetaData_header_t);
  dest += sizeof (FSM_MetaData_header_t);
  
  void *iter = src;
  void *iter_end = FSM_metadata_get_size(src) + src;

  while (iter < iter_end){
    *(FSM_record_metadata_t *)dest = *(FSM_record_metadata_t *)src; 
    iter += sizeof (FSM_record_metadata_t);
  }
  return true;
}

/*
 * retutns gc_end_address
 * */

void *FSM_get_gc_end_add (uint8_t sector){
  // first word of the gc sector is 0xffffffff
  void *iter = flash_get_sector_address (sector) + 4;
  void *sector_end = iter + flash_get_sector_size(sector) - 4;
  // gcoceed if no addresses are NULL;
  while (iter < sector_end) {
    uint32_t word = *(uint32_t *)iter;
    if (word == 0xffffffff) return iter;
    uint32_t head = FSM_md_pkt_get_head(iter);
    uint32_t md_pkt_size = 0;
    if (head == FSM_MD_HEAD_CO){
      // md_head
      md_pkt_size = FSM_metadata_get_size (iter);
    }
    else if (head == FSM_MD_HEAD_INCO) {
      void *ret = skip_the_garbage (iter, sector_end);
      if (!ret) {
        DEBUG_printf(__usart1_print, "returned to FSM_get_gc_end_add from \
            skip_the_garbage function with NULL return value \n\r");
      }
      return ret;
    }

    else if (head == FSM_PKT_HEAD_CO){
      // pkt head
      md_pkt_size = FSM_packet_get_size (iter);
    }
    else if ( head == FSM_PKT_HEAD_INCO) {
    }

    //#ifdef DEBUG
    else{
      DEBUG_printf (__usart1_print, "in fsm_get_gc_end_add function,"
              "head is not 0xffffffff or FSM_MD(PKT)_HEAD_CO/INCO");
    }
    //#endif

    iter += md_pkt_size;
  }
  return iter;
}


// returns the end free address of md_sector and populate latest_metadata
void *FSM_get_md_end_add (uint8_t sector,
        FSM_MetaData_header_t** latest_metadata){

  void *iter = flash_get_sector_address(sector);
  void *sector_end = iter + flash_get_sector_size(sector);

  while (iter < sector_end && *(uint32_t *)iter != 0xffffffff){

    uint32_t word = *(uint32_t *)iter;
    uint32_t head = FSM_md_pkt_get_head (iter);
    if ( head == FSM_MD_HEAD_CO ){
      // md is written completely
      *latest_metadata = (FSM_MetaData_header_t *)iter;
      iter += FSM_metadata_get_size (iter);
    }
    else if (head == FSM_MD_HEAD_INCO){
      // md is written incompletely
      void *ret = skip_the_garbage(iter, sector_end);
      if (!ret) {
        DEBUG_printf(__usart1_print, "returned  to FSM_get_md_end_add from \
            skip_the_garbage function and the return value is NULL\n\r");
        return NULL;
      }
      return ret;
    }
    else {
      DEBUG_printf( __usart1_print, "in FSM_get_md_end_add \
          function, iter is not 0xffffffff or FSM_MD_HEAD_INCO or \
          FSM_MD_HEAD_CO\n\n");
      DEBUG_printf (__usart1_print, "instead it is %x\n\r", \
          *(uint32_t *)iter);
      DEBUG_printf (__usart1_print, "value of iter is, %x\n\r", \
          iter);
      // hang
      DEBUG_assert (0);
    }
  }
  return iter;

}
// sector is the starting address of the log
// assuming the log is continuous and does not have gc sector in the range
void *FSM_get_log_end_add (uint8_t st_sector, uint8_t end_sector){

  // log will only contain  complete / incomplete packets
  // there is only one incomplete packet in the log and it must be at the end

  void *iter = flash_get_sector_address(st_sector);
  void *end_add = flash_get_sector_address(end_sector) +
          flash_get_sector_size(end_sector);


  while (iter < end_add){
    uint32_t word = *(uint32_t *) iter;
    if (word == 0xffffffff){
        DEBUG_printf (__usart1_print, "in FSM_get_log_end_add, function,"
                  "at address %x, log end is found... see the hexdump\n\r", iter);
      return iter;
    }
    uint32_t head = FSM_md_pkt_get_head(iter);

    if (head == FSM_PKT_HEAD_CO){
      iter += FSM_metadata_get_size(iter);
    }
    else if (head == FSM_PKT_HEAD_INCO) {

      void *ret = skip_the_garbage(iter, end_add);
      if (!ret) {
        DEBUG_printf(__usart1_print, "returned  to FSM_get_log_end_add  from\
            skip_the_garbage function and the return value is NULL\n\r");
        return NULL;
      }
      return ret;
    }
  }
  // should nevet get here if this function is correct
  return NULL;
}


FSM_Packet_header_t* FSM_get_last_packet 
                (FSM_Packet_header_t* start_packet)
{
  if (!start_packet) {
    printf (__usart1_print, "[ERROR]    in function FSM_get_last_packet, "
        "start packet is NULL");
  }
  FSM_Packet_header_t* iter = start_packet;

  while (iter -> next_packet != (FSM_Packet_header_t *)0xffffffff){
    iter = iter->next_packet;
  }
  return iter; 
}

void *skip_the_garbage (void *iter, void *end){

  while (iter < end && *(uint32_t *)iter != 0xffffffff)
    iter += 4;
  if (iter >= end){
    DEBUG_printf (__usart1_print, "in skip the garbage function, \
        md is incompletely written and cant find a free space in the \
        gc sector \n\r");
    return NULL;
  }
  else 
    return iter;
}


bool FSM_sequence_table_fill (FSM_MetaData_header_t *metadata){

  /*TODO*/ 

  return 0;
}



















/*
 *  assumptions -> 
 *  1. dest is always in flash (log)
 *  2. packet in the src contains the flash address of next packet
 *  
 *  sideeffect ->
 *  1. write to flash (log)
 *  2. change addresses (log_end_add, gc_end_add (in case of gc), )
 *
 *
 *
 *
 *
 *  plan -> 
 *  copy packet from src to the log (src can be from write buffer or from 
 *    gc !!)  -> clear the next packet pointer as it might contain garbage value
 *
 *  flash_write the packet header and packet payload
 *
 *  update the next packet pointer of the packet from lastpacket arr 
 *
 *  update lastpacket arr 
 *
 *  update addr
 * */

bool FSM_copy_packet_to_log (void *src, void *dest, 
        FSM_addresses_t *addresses, FSM_record_metadata_t *last_packet_arr){


  return true;
}
bool FSM_copy_packet_to_log_helper  (void *src, void *dest){
/* TODO */
  return true;
}

// if flash is busy writing, then execute some timetaking tasks.... 
bool FSM_flash_write_wrapper (uint32_t *buff, uint32_t size, void *address){

  if (flash_state == FLASH_STATE_IDLE){
    flash_write (buff, size, address);
    return false;
  }
  execute_time_taking_tasks1 ();
  flash_write (buff, size, address);

  return true;
}

// operations that are somewhat time taking (but less than writing to the flash)
bool execute_time_taking_tasks1 (){
  // call some functions

  return true;
}
// if, then execute some timetaking tasks.... 
bool FSM_flash_erase_wrapper (void *address){

  if (flash_state == FLASH_STATE_IDLE){
    flash_erase (address);
    return false;
  }
  execute_time_taking_tasks2 ();
  flash_erase (address);

  return true;
}
// operations that are somewhat time taking (
// but less than erase to the flash)
bool execute_time_taking_tasks2 (){
  // call some functions
  
  return  true;
}
  
FSM_Packet_header_t* get_next_packet (FSM_Packet_header_t *packet_header){
  /*TODO*/
  
 // st is either in the gc sector / in the md sector
  return NULL;
}

// assumption --> dest is in the flash range (gc sector / md sector)
bool FSM_copy_sequence_table_to_flash (void *src, void *dest){
  /* TODO */
  return true;
}

