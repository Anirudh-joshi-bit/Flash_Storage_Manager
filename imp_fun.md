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
    - 




