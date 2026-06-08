#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "ring_buff.h"
#include "queue.h"
#include "flash.h"
#include "garbage_collector.h"


// defines

// packet
#define FSM_PACKET_DESCRIPTOR_SIZE_MSK      ((1<<20) -1)  // 19 ones
#define FSM_PACKET_DESCRIPTOR_VALID_MSK     (1<<20)
#define FSM_PACKET_DESCRIPTOR_NREMOVED_MSK  (1<<21)
#define FSM_PACKET_DESCRIPTOR_HEAD_MSK      (~0U<<21)

// metadata
#define FSM_METADATA_DESCRIPTOR_SIZE_MSK    ((1<<20) -1)  // 19 ones
#define FSM_METADATA_DESCRIPTOR_VALID_MSK   (1<<20)
#define FSM_METADATA_DESCRIPTOR_HEAD_MSK    (~0U<<21)

// general
#define FSM_PKT_HEAD                        0x3ff<<21       // 01111111111
#define FSM_MD_HEAD                         0x1ff<<21       // 00111111111
#define FSM_WRITE_BUFFER_SIZE               10*1024
#define MAX_PACKET_SIZE                     1024            // 1kB
#define MAX_FLASH_WRITE_BUFFER_SIZE         6*1024          // 6kB
#define VALID_FLAG                          0xffffffff
#define FLAGS_INIT_VALUE                    0x0fffffff


typedef struct __FSM_record_metadata_t {
    // this is record !!!

    uint32_t key;
    void *address;
    void *next_en;      // not sure 
    uint32_t removed;

} FSM_record_metadata_t;

/*
    packet_descriptor for packet => 
    bit[0] - bit[18] = size of the packet       max val = 512KB
    bit[19]           = valid (1) invalid (0)
    bit[20]           = not removed (1) removed (0)
    bit[21] - bit[31] = flag (to identify a packet in gc_sector)

    sizeof (FSM_Packet_t) == 8Byte
*/

// packet = packet_header + data
typedef struct __FSM_Packet_header_t {

    uint32_t packet_descriptor;
    struct __FSM_Packet_header_t *next_packet;

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


typedef struct __FSM_Sector_t {

    uint32_t address;
    uint32_t size;
    uint8_t erase_count;
    uint8_t dirty_blocks;
    uint8_t sector_number;

} FSM_Sector_t;

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
              FSM_MetaData_header_t *ram_metadata, 
              FSM_MetaData_header_t *flash_metadata);
int8_t sector_init (FSM_Sector_t *sector, void *address);
int8_t record_metadata_init (FSM_record_metadata_t *rec_metadata, uint32_t key, void *addr);
void FSM_record_request_init (FSM_record_request_t *rr, 
          volatile Ring_buff_t *rb, queue_t *q);
void FSM_write_buffer_init (FSM_write_buffer_t *fsm_wb, 
          uint8_t *buffer, uint32_t size);
void FSM_request_pair_init (FSM_request_pair_t *rp, uint8_t *key, 
          uint32_t size);
int8_t FSM_write (void *buffer, uint16_t size, bool continuity);
bool FSM_start_serving_request (FSM_record_request_t *rr_array, uint8_t size);
bool FSM_flash_write (FSM_write_buffer_t* fsm_wb,  uint32_t *address, 
                            FSM_Sector_t flash_sectors[8]);
void FSM_Packet_init(FSM_Packet_header_t *pkt, uint16_t data_size) ;
void metadata_header_init(FSM_MetaData_header_t *mdb, uint32_t metadata_size) ;
