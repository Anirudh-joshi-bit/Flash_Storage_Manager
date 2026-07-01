#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "ring_buff.h"
#include "queue.h"
#include "flash.h"
#include "garbage_collector.h"
#include "stm32f401xe.h"


// defines

// record #define FSM_RECORD_REMOVED_MSK              1

// packet
#define FSM_PACKET_DESCRIPTOR_SIZE_MSK      ((1<<20) -1)  // 19 ones
#define FSM_PACKET_DESCRIPTOR_VALID_MSK     (1<<20)
#define FSM_PACKET_DESCRIPTOR_NREMOVED_MSK  (1<<21)
#define FSM_PACKET_SEQINFO_NEXTSEQ_MSK      ((1<<10) -1) 
#define FSM_PACKET_SEQINFO_SELFSEQ_MSK      (((1<<10) -1) << (32-10))
#define FSM_PKT_HEAD_INCO                   (0x3ff<<21)       // 01111111111
#define FSM_PKT_HEAD_CO                     (0x1ff<<21)       // 00111111111

// same field for md, packet, sequence _table  bit[21] - bit[31]
#define FSM_MD_PKT_ST_DESCRIPTOR_HEAD_MSK      (~0U<<21)

// metadata
#define FSM_METADATA_DESCRIPTOR_SIZE_MSK    ((1<<20) -1)  // 19 onesfsm.
#define FSM_METADATA_DESCRIPTOR_VALID_MSK   (1<<20)
#define FSM_MD_HEAD_INCO                    (0x0ff<<21)       // 00011111111
#define FSM_MD_HEAD_CO                      (0x07f<<21)       // 00001111111

// record_metadata 
#define FSM_RECORD_MD_SEQ_MSK               ((1<<10)-1)
#define FSM_RECORD_MD_KEY_MSK               (((1<<22)-1) << (32-22))

// sequence table
#define FSM_SEQUENCE_TABLE_ID_MSK           ((1<<11)-1)
#define FSM_SEQUENCE_TABLE_VALID_MSK        (1<<19)
#define FSM_SEQUENCE_TABLE_INCO             (0x3f<<21)        // 00000111111
#define FSM_SEQUENCE_TABLE_CO               (0x1f<<21)        // 00000011111
#define FSM_SEQUENCE_TABLE_OFFSET_MSK       ((1<<20)-1)
#define FSM_SEQUENCE_TABLE_OCCUPIED_MSK     (1<<20)
#define FSM_SEQUENCE_TABLE_SEQ_NUM_MSK      (((1<<10)-1) << (32-10))

// general
// if the first address of any sector is 0xffffffff, then it is the GC sector
#define FSM_GC_SECTOR_ID                    0xffffffff      
#define FSM_LOG_BASE_ADDRESS                0x08008000

#define FSM_WRITE_BUFFER_SIZE               10*1024
#define MAX_PACKET_SIZE                     1024      // 1kB, min = 1KB 
#define MAX_FLASH_WRITE_BUFFER_SIZE         6*1024          // 6kB
#define VALID_FLAG                          0xffffffff
#define FLAGS_INIT_VALUE                    0x0fffffff
#define FSM_MAX_RECORD_COUNT                100
#define MAX_RECORD_COUNT                    100
#define FSM_LOG_SIZE                        (512-16-16-128)
#define FSM_MAX_SEQ_NUM                     1024-1  //10 bits for sseq num

#define FLASH_ST_ADDRESS                    0x08000000
#define FLASH_END_ADDRESS                   0x08080000
#define RAM_ST_ADDRESS                      0x20000000
#define RAM_END_ADDRESS                     0x20018000



// change addresses only after write / erase 
typedef struct __FSM_addresses_t {
  void *gc_end_add;
  void *gc_sector_add;        // for now -> this is constant
  void *log_end_add;
  void *md_end_add;
  void *md_sector_add;        // for now -> this is constant
  void *latest_md_add_flash;
  void *latest_md_add_ram;
  void *latest_sequence_table_add_flash;
  // may need more !!!

}FSM_addresses_t;

/*
    packet_descriptor for packet => 
    bit[0] - bit[18] = size of the packet       max val = 512KB
    bit[19]           = valid (1) invalid (0)
    bit[20]           = not removed (1) removed (0)
    bit[21] - bit[31] = flag (to identify a packet in gc_sector)

    sizeof (FSM_Packet_t) == 8Byte

    seq_info ->
    bit[0] - bit[9] ==  self seq number (10 bits)
    bit [10] - bit[20]== reserved
    bit[22] - bit[31]== next packet seq number (10-bits)

*/
// note ... size of packet and md head must be the same....
// packet = packet_header + data

typedef struct __FSM_Packet_header_t {

    uint32_t packet_descriptor;
    uint32_t seq_info;
    // then thers is the payload

} FSM_Packet_header_t;

/*
    metadata_descriptor for packet => 
    bit[0] - bit[18] = size of the metadata_block       max val ~ 512KB
    bit[19]           = valid (1) invalid (0)
    bit[20]           = reserved
    bit[21] - bit[31] = head (to identify a metadata in gc_sector)

    sizeof (FSM_Packet_t) == 8Byte
*/

// copy metadata carefully as this block does not  contain the buffer
typedef struct __FSM_MetaData_header_t {
  uint32_t metadata_descriptor;
} FSM_MetaData_header_t ;



// each sequence number is mapped to the offset from 0x08000000
//
// seq_table_header  ->
//  sequence_table identifier = bit[31] - bit[21]
//  reserved                 =  bit[20]
//  valid flag               =  bit[19] 
//  reserved                 =  bit[18] - bit[0]
//
// table [i] => 
//  bit[0] - bit[19]    == offset from 0x08000000
//  bit[20]             == occupied seq number
//  bit[21]             == reserved
// bit[22] - bit[31]    == seqnumebr

typedef struct __FSM_sequence_table_t {
  uint32_t seq_table_header;
  uint32_t table [FSM_MAX_SEQ_NUM+1]; 
} FSM_sequence_table_t;


typedef struct __FSM_Sector_t {

    uint32_t address;
    uint32_t size;
    uint8_t erase_count;
    uint8_t dirty_blocks;
    uint8_t sector_number;

} FSM_Sector_t;


// md_entry -> mapps seqnumber -> key
// if lsb of key is 0 -> record is removed
// bit[0] - bit[9]     == seq_number
// bit[10] - bit[31]    == key
//
typedef struct __FSM_record_metadata_t {
  uint32_t md_entry;
} FSM_record_metadata_t;


typedef struct __FSM_write_buffer_t {

    uint8_t *data;
    uint32_t capacity;
    uint32_t size;          // size in bytes
    

} FSM_write_buffer_t;

/* 
 pairing ring buffer and record request ... ring buffer is from where requests
 will be served 
*/
typedef struct __FSM_record_request_t {
  
  volatile Ring_buff_t *rb;
  queue_t *q;

} FSM_record_request_t;


/* request -> key to uniquely identfy a record, size of the record*/
typedef struct __FSM_request_pair_t {
  uint8_t *key;
  uint32_t size;

} FSM_request_pair_t;



// functions
 

bool FSM_init(FSM_Sector_t flash_sectors[8], FSM_write_buffer_t *fsm_wb,
              uint8_t wb[FSM_WRITE_BUFFER_SIZE],
              FSM_addresses_t* addresses,
              FSM_record_metadata_t *last_packet_arr,
              uint32_t *number_record);     // stores the num of record) 
                
bool sector_init (FSM_Sector_t *sector, void *address);
bool record_metadata_init (FSM_record_metadata_t *rec_metadata, 
                            uint32_t key, void *addr);
void FSM_record_request_init (FSM_record_request_t *rr, 
          volatile Ring_buff_t *rb, queue_t *q);
void FSM_write_buffer_init (FSM_write_buffer_t *fsm_wb, 
          uint8_t *buffer, uint32_t size);
void FSM_request_pair_init (FSM_request_pair_t *rp, uint8_t *key, 
          uint32_t size);
//int8_t FSM_write (void *buffer, uint16_t size, bool continuity);
bool FSM_start_serving_request (FSM_record_request_t *rr_array, uint8_t size);
bool FSM_flash_write (FSM_write_buffer_t* fsm_wb,  uint32_t *address, 
                           FSM_Sector_t flash_sectors[], uint32_t sector_num);
bool FSM_Packet_init(FSM_Packet_header_t *pkt, uint16_t data_size) ;
bool metadata_header_init(FSM_MetaData_header_t *mdb, 
                      uint32_t metadata_size) ;

bool FSM_copy_packet_to_log (void *src, void *dest, 
        FSM_addresses_t *addresses, FSM_record_metadata_t *last_packet_arr);
bool FSM_copy_packet_to_log_helper  (void *src, void *dest);


