#pragma once
#include <stdint.h>
#include <stdbool.h>


// defines
#define MAX_DATA_BLOCK_SIZE                 1024            // 1kB
#define MAX_FLASH_WRITE_BUFFER_SIZE         6*1024          // 6kB
#define MAX_FLAG_COUNT                      2
#define PACKET_MEMBER_COUNT                 5
#define VALID_FLAG                          0xffffffff
#define MAX_PACKET_DATA_SIZE                MAX_DATA_BLOCK_SIZE/4 - PACKET_MEMBER_COUNT+1
#define FLASH_SECTOR0_Addr                  0x08000000      // 16kB
#define FLASH_SECTOR1_Addr                  0x08004000      // 16kB
#define FLASH_SECTOR2_Addr                  0x08008000      // 16kB
#define FLASH_SECTOR3_Addr                  0x0800C000      // 16kB
#define FLASH_SECTOR4_Addr                  0x08010000      // 64kB
#define FLASH_SECTOR5_Addr                  0x08020000      // 128kB
#define FLASH_SECTOR6_Addr                  0x0803ffff      // 128kB
#define FLASH_SECTOR7_Addr                  0x08060000      // 128kB
#define FLAGS_INIT_VALUE                    0x0fffffff
#define MAX_NUM_RECORD                      100             // maxumum number of records


typedef struct __packet_t {

    uint32_t data_size;
    uint32_t data [MAX_PACKET_DATA_SIZE];
    struct __packet_t *next_block;
    uint32_t removed;       // useful in garbage collection
    uint32_t valid;         // usefull to detect power loss

} Packet_t;

typedef struct __Sector_t {

    uint32_t address;
    uint32_t size;
    uint8_t erase_count;
    uint8_t dirty_blocks;
    uint8_t sector_number;

} Sector_t;

typedef struct __Flash_write_buffer_t {

    uint32_t size;
    uint8_t data[MAX_FLASH_WRITE_BUFFER_SIZE]; 

} Flash_write_buffer_t;

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
    uint32_t metadata_block_size;
    KeyAddr_pair_t metadata_buffer[MAX_NUM_RECORD];
    uint32_t        metadata_buffer_size;
    uint32_t flags [MAX_FLAG_COUNT];

} MetaData_block_t ;


typedef struct __record_next_ptr_ele_t {
  


};
typedef struct __record_next_ptr_t {
  


};

// functions
 
int8_t packet_init (Packet_t *pkt, uint32_t data_size);
int8_t sector_init (Sector_t *sector, void *address);
int8_t flash_write_buffer_init (Flash_write_buffer_t *wb);
int8_t metadata_block_init (MetaData_block_t *mdb);
int8_t keyAddr_pair_init (KeyAddr_pair_t *kap, uint32_t key, void *addr);


// flash storage manager functions

int8_t FSM_write (void *buffer, uint16_t size, bool continuity);


