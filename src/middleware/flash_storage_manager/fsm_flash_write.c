#include "flash_storage_manager.h"
#include "DEBUG.h"

// cpoying the contenet of write buffer to flash !!!!
bool FSM_flash_write(FSM_write_buffer_t *fsm_wb, uint32_t *address,
                     FSM_Sector_t flash_sectors[], uint32_t sector_num) {
  // flash write  wrapper
  uint32_t sector_number = flash_get_sector(address);
  uint32_t available_sector_size = flash_get_sector_size(sector_number) -
                                    flash_sectors[sector_number].size;
  if (available_sector_size < fsm_wb->size){

    garbage_collector_run (/* arguments */);
  }
    

  



    return true;
}
