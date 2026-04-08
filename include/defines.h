#pragma once
#include <stdint.h>

#define MAX_DATA_BLOCK_SIZE                 1024            // 1kB
#define MAX_FLASH_WRITE_BUFFER_SIZE         6*1024          // 6kB
#define MAX_FLAG_COUNT                      2



typedef struct __packet_t {

    uint32_t size;
    uint32_t data[MAX_DATA_BLOCK_SIZE/4 - 5];
    uint32_t *next_block;
    uint32_t crc;
    uint32_t removed;       // useful in garbage collection
    uint32_t valid;         // usefull to detect power loss

} packet_t;

typedef struct __Sector_t {

    uint32_t address;
    uint32_t size;
    uint8_t erase_count;
    uint8_t dirty_blocks;
    uint8_t sector_numder;

} Sector_t;

typedef struct __Flash_write_buffer_t {

    uint32_t size;
    uint8_t data[MAX_FLASH_WRITE_BUFFER_SIZE]; 

} Flash_write_buffer_t;

typedef struct __KeyAddr_pair_t {

    uint32_t key;
    uint32_t *address;
    uint32_t valid;
    uint32_t extendable;

} KeyAddr_pair_t;

typedef struct __MetaData_block_t {
    
    uint32_t start_word;                        // 0xffffffff
    uint32_t *secondary_sector_end_addr;
    uint32_t metadata_block_size;
    KeyAddr_pair_t *metadata_buffer;
    uint32_t        metadata_buffer_size;
    uint32_t flags [MAX_FLAG_COUNT];

} MetaData_block_t ;

