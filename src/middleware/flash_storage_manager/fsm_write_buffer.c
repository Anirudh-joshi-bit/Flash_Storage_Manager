#include "flash_storage_manager.h"

// to write packet and data into the write buffer, we need to have packet and
// ring buffer where the data lives !!!
// data will be extraced from the write buffer

extern FSM_Sector_t flash_sectors[8];
extern void *fsm_flash_write_end_address;


uint8_t FSM_write_buffer_write(FSM_write_buffer_t *fsm_wb,
                               FSM_Packet_header_t *fsm_packet, 
                               Ring_buff_t *rb, uint32_t data_size) {
  // limit check
  if (data_size > MAX_PACKET_SIZE - sizeof(FSM_Packet_header_t))
    return 1;

  // if cannot be written to the wb, then copy the whole data into flash and
  // continue
  if (data_size + sizeof(FSM_Packet_header_t) >
      fsm_wb->size + fsm_wb->capacity) {
    // write to the flash
    FSM_flash_write(fsm_wb, fsm_flash_write_end_address, flash_sectors);
  }

  // fsm_wb-> size is in bytes
  uint8_t *wb_end = (uint8_t *)fsm_wb + fsm_wb->size;

  // copy packet ->
  *((uint32_t *)wb_end++) = fsm_packet->packet_descriptor;
  *((FSM_Packet_header_t **)wb_end++) = fsm_packet->next_packet;

  uint32_t temp_size = MAX_PACKET_SIZE - sizeof(FSM_Packet_header_t);
  if (temp_size > data_size)
    temp_size = data_size;

  uint8_t data[temp_size];
  Ring_buff_read(rb, data, temp_size);

  for (int i = 0; i < data_size; i++) {
    *(wb_end++) = data[i];
  }

  // update the size of fsm_wb
  fsm_wb->size += temp_size + sizeof(FSM_Packet_header_t);

  return 0;
}

uint8_t fsm_flash_write_buffer_read() {
  // if needed
  return 0;
}
