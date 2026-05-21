#include "commons.h"
#include "usart.h"
#include "ring_buff.h"
#include "flash_storage_manager.h"


uint8_t buff[RING_BUFF_SIZE];
Sector_t flash_sectors [8];
Packet_t temp;
Flash_write_buffer_t flash_write_buffer;
MetaData_block_t metadata;


volatile Ring_buff_t *usart1_ring_buffer;
queue_t usart1_request_q;
volatile Ring_buff_t *usart2_ring_buffer;
queue_t usart2_request_q;
volatile Ring_buff_t *usart3_ring_buffer;
queue_t usart3_request_q;

FSM_request_pair usart1_request [USART1_MAX_REQUEST];
FSM_request_pair usart2_request [USART2_MAX_REQUEST];
FSM_request_pair usart3_request [USART3_MAX_REQUEST];

FSM_record_request rr1;
FSM_record_request rr2;
FSM_record_request rr3;


int main (void){

  // System init;
  __usart1_init();
  
  usart1_ring_buffer = (rr1.rb);
  usart2_ring_buffer = (rr2.rb);
  usart3_ring_buffer = (rr3.rb);

  // ring buffers are initialised inside the FSM_record_request_init function
  FSM_record_request_init(&rr1, (void *)usart1_request, USART1_MAX_REQUEST);
  FSM_record_request_init(&rr2, (void *)usart2_request, USART2_MAX_REQUEST);
  FSM_record_request_init(&rr3, (void *)usart3_request, USART3_MAX_REQUEST);
  


  sector_init (&flash_sectors[0],(void *) FLASH_SECTOR0_Addr);
  sector_init (&flash_sectors[1],(void *) FLASH_SECTOR1_Addr);
  sector_init (&flash_sectors[2],(void *) FLASH_SECTOR2_Addr);
  sector_init (&flash_sectors[3],(void *) FLASH_SECTOR3_Addr);
  sector_init (&flash_sectors[4],(void *) FLASH_SECTOR4_Addr);
  sector_init (&flash_sectors[5],(void *) FLASH_SECTOR5_Addr);
  sector_init (&flash_sectors[6],(void *) FLASH_SECTOR6_Addr);
  sector_init (&flash_sectors[7],(void *) FLASH_SECTOR7_Addr);

  packet_init (&temp, MAX_PACKET_DATA_SIZE);

  flash_write_buffer_init (&flash_write_buffer);
  metadata_block_init (&metadata);


  printf ("give me some data", 0x0);

  while (Ring_buff_size(usart1_ring_buffer) < 31);

  Ring_buff_read (usart1_ring_buffer, buff, RING_BUFF_SIZE);
  printf ("read from ring buffer", 0x0);


  while (1);
  return 0;
}
