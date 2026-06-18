# functions
1. `fsm_init`
    ```c
    bool FSM_init(FSM_Sector_t flash_sectors[8], 
              FSM_write_buffer_t *fsm_wb,
              uint8_t wb[FSM_WRITE_BUFFER_SIZE],
              FSM_MetaData_header_t *metadata_in_ram,
              FSM_MetaData_header_t *metadata_in_flash,
              FSM_addresses_t* addresses);
    ```
- init the state of the flash_storage_manager

1. `FSM_get_log_end`
```c
void *FSM_get_log_end_add (uint8_t st_sector, 
            uint8_t en_sector){
```

- get the log end 
- called by `fsm_init` function

1. `find_last_metadata_in_gc`
```c
    FSM_MetaData_header_t* find_last_metadata_in_gc (void * gc_sector_add);
```

- this function finds latest metadata in the gc
- this is used in fsm_init if there is no complete metadata  present in the metadata sector

