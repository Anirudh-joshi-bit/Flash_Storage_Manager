#include "../include/commons.h"
#include "../include/USART.h"
#include "../include/Ring_buff.h"

uint8_t buff[RING_BUFF_SIZE];
Sector_t flash_sectors [8];
Ring_buff_t usart1_ring_buffer;
Packet_t temp;
Flash_write_buffer_t flash_write_buffer;
MetaData_block_t metadata;

int main (void){

  // System init;
  __usart1_init();

  Ring_buff_init (&usart1_ring_buffer);

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

  while (usart1_ring_buffer.size < 30);

  Ring_buff_read (&usart1_ring_buffer, buff, RING_BUFF_SIZE);    
  printf ("read from ring buffer", 0x0);


  while (1);
  return 0;
}
