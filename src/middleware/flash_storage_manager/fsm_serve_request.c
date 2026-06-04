#include "ring_buff.h"
#include <flash_storage_manager.h>

#define FSM_PACKET_DATA_SIZE MAX_PACKET_SIZE-sizeof (FSM_Packet_header_t)

extern FSM_write_buffer_t fsm_wb;

bool FSM_start_serving_request (FSM_record_request_t *rr_array, uint8_t size){

  for (uint32_t i=0; i<size; i++){
    // todo
    FSM_record_request_t *rr = &rr_array[i];
    if (Ring_buff_size(rr->rb) >= FSM_PACKET_DATA_SIZE){
      uint32_t data_size = FSM_PACKET_DATA_SIZE;

      // todo
      if (rr->q.front)

      uint8_t data_buffer[FSM_PACKET_DATA_SIZE]
    }
  


  }
  return true;
}
