#include "defines.h"
#include "flash_storage_manager.h"

int8_t packetisation(Packet_t *pkt, uint32_t *data_buff, uint16_t data_size) {

  for (uint16_t i = 0; i < data_size; i++) {
    pkt->data[i] = data_buff[i];
  }
  return 0;
}

int8_t fsm_write(uint32_t key, uint32_t *buff, uint32_t size,
                 bool continuation) {
  int div = size / MAX_PACKET_DATA_SIZE;
  int rem = size % MAX_PACKET_DATA_SIZE;

  uint32_t *buffer_address = buff;
  Packet_t pkt;

  int8_t init_ret = packet_init(&pkt, MAX_PACKET_DATA_SIZE);
  if (init_ret)
    return init_ret;

  for (int i = 0; i < div; i++) {
    packetisation(&pkt, buffer_address, MAX_PACKET_DATA_SIZE);
    /*change the nextblock*/
    /*function layer2 call*/

    buffer_address += MAX_PACKET_DATA_SIZE;
  }
  if (rem){
    init_ret = packet_init(&pkt, rem);
    packetisation(&pkt, buffer_address, rem);
    /*change the nextblock*/
    /*fucntion layer2 call*/
  }
}

int8_t __fsm_write_to_write_buffer(Packet_t *pkt){

 return 0;
}
