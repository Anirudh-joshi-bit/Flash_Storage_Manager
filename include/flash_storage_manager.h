#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "ring_buff.h"
#include "queue.h"
#include "flash.h"
#include "garbage_collector.h"


// defines
#define MAX_PACKET_SIZE                     1024            // 1kB
#define MAX_FLASH_WRITE_BUFFER_SIZE         6*1024          // 6kB
#define MAX_FLAG_COUNT                      2
#define VALID_FLAG                          0xffffffff
#define FLAGS_INIT_VALUE                    0x0fffffff


// packet = packet_header + data
typedef struct __FSM_Packet_header_t {

    uint32_t data_size;
    struct __FSM_Packet_header_t *next_packet;
    uint32_t removed;       // useful in garbage collection
    uint32_t valid;         // usefull to detect power loss

} FSM_Packet_header_t;

typedef struct __Sector_t {

    uint32_t address;
    uint32_t size;
    uint8_t erase_count;
    uint8_t dirty_blocks;
    uint8_t sector_number;

} Sector_t;

typedef struct __FSM_write_buffer_t {

    uint8_t *data;
    uint32_t capacity;
    uint32_t size;          // size in bytes
    

} FSM_write_buffer_t;

typedef struct __KeyAddr_pair_t {
    // this is record !!!

    uint32_t key;
    void *address;
    void *next_en;
    uint32_t valid;

} KeyAddr_pair_t;

typedef struct __MetaData_block_t {
    
    uint32_t start_word;                        // 0xffffffff
    void *secondary_sector_end_addr;
    uint32_t metadata_buffer_size;
    KeyAddr_pair_t *metadata_buffer;
    uint32_t flags [MAX_FLAG_COUNT];

} MetaData_block_t ;


typedef struct __record_next_ptr_ele_t {
  


}record_next_ptr_ele_t;

typedef struct __record_next_ptr_t {
  


} record_next_ptr_t;

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
 
int8_t packet_init (FSM_Packet_header_t *pkt, uint32_t data_size);
int8_t sector_init (Sector_t *sector, void *address);
int8_t metadata_block_init (MetaData_block_t *mdb);
int8_t keyAddr_pair_init (KeyAddr_pair_t *kap, uint32_t key, void *addr);
void FSM_record_request_init (FSM_record_request_t *rr, volatile Ring_buff_t *rb, queue_t *q);
void FSM_write_buffer_init (FSM_write_buffer_t *fsm_wb, uint8_t *buffer, uint32_t size);
void FSM_request_pair_init (FSM_request_pair_t *rp, uint8_t *key, uint32_t size);

// flash storage manager functions

int8_t FSM_write (void *buffer, uint16_t size, bool continuity);
bool FSM_start_serving_request (FSM_record_request_t *rr_array, uint8_t size);

bool FSM_flash_write (FSM_write_buffer_t* fsm_wb,  uint32_t *address, 
                            Sector_t flash_sectors[8]);

