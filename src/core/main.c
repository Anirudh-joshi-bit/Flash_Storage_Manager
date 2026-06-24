#include "commons.h"
#include "defines.h"
#include "flash.h"
#include "flash_storage_manager.h"
#include "ring_buff.h"
#include "usart.h"
#include "DEBUG.h"
#include "stm32f401xe.h"


uint8_t buff[RING_BUFF_SIZE];
FSM_Sector_t flash_sectors[8];

volatile Ring_buff_t usart1_ring_buffer;
volatile Ring_buff_t usart2_ring_buffer;
volatile Ring_buff_t usart6_ring_buffer;

queue_t usart1_request_q;
queue_t usart2_request_q;
queue_t usart6_request_q;

FSM_request_pair_t usart1_request[USART1_MAX_REQUEST];
FSM_request_pair_t usart2_request[USART2_MAX_REQUEST];
FSM_request_pair_t usart6_request[USART6_MAX_REQUEST];

FSM_record_request_t rr1;
FSM_record_request_t rr2;
FSM_record_request_t rr3;

uint8_t wb[FSM_WRITE_BUFFER_SIZE];
FSM_write_buffer_t fsm_wb;

FSM_record_request_t rr[3];

// metadata ->
FSM_MetaData_header_t fsm_ram_metadata;
FSM_MetaData_header_t *fsm_flash_metadata;         // fill this from flash
// stores the address of most recent valid  metadata  
// that is present in the flash

// init in fsm init function
FSM_addresses_t fsm_addresses;

FSM_record_metadata_t last_packet_arr [MAX_RECORD_COUNT *
                                  sizeof (FSM_record_metadata_t)];
uint32_t number_record = 0;     // number of record


int main(void) {

  // System init;
  __usart1_init();
  // __usart2_init();
  // __usart6_init();

  flash_init ();

/*
  // only for the first time 
  // erase sector from 1 - 8 


  for (int i=1; i<8; i++){
    void * sector  = flash_get_sector_address(i);
    flash_erase(sector);
  }
   printf(__usart1_print, "done erasign the flash \n\r");

  hang ();
*/

  DEBUG_test (); // for now .. test wil be conducted on getter and setter


  // __usart1_print("hii there", 10);

  // ring buffers init ->
  Ring_buff_init(&usart1_ring_buffer);
  Ring_buff_init(&usart2_ring_buffer);
  Ring_buff_init(&usart6_ring_buffer);

  // queue init ->
  queue_init(&usart1_request_q, (void *)usart1_request,
             sizeof(usart1_request) * USART1_MAX_REQUEST);
  queue_init(&usart2_request_q, (void *)usart2_request,
             sizeof(usart2_request) * USART2_MAX_REQUEST);
  queue_init(&usart6_request_q, (void *)usart6_request,
             sizeof(usart6_request) * USART6_MAX_REQUEST);

  FSM_record_request_init(&rr1, &usart1_ring_buffer, &usart1_request_q);
  FSM_record_request_init(&rr2, &usart2_ring_buffer, &usart2_request_q);
  FSM_record_request_init(&rr3, &usart6_ring_buffer, &usart6_request_q);

  // fsm_init
  if (FSM_init(flash_sectors, &fsm_wb, wb, &fsm_ram_metadata,
        fsm_flash_metadata, &fsm_addresses, last_packet_arr, 
        &number_record)) 
  {
    printf (__usart1_print,"[ERROR] FSM_init execution failed");
    hang ();
  }

  DEBUG_printf (__usart1_print,"[atatus] FSM_init returns successfully\n\r");

  // hang here
  
  hang ();

  
  // fsm make requests
  // at this poiint, usart 2 and usart 6 are not working properly i dont know
  // why make request only on usrt 1 for now
  /*********************** start request for usart 1 *************************/

  
  /* todo -> map string(key) to numebr / index */


  // there are 5 requests in the request queue
  FSM_request_pair_t usart1_r1;
  const char *key1 = "key1";
  FSM_request_pair_init(&usart1_r1, (uint8_t *)key1, 5 * 1024);

  FSM_request_pair_t usart1_r2;
  const char *key2 = "key2";
  FSM_request_pair_init(&usart1_r2, (uint8_t *)key2, 5 * 1024);

  FSM_request_pair_t usart1_r3;
  const char *key3 = "key3";
  FSM_request_pair_init(&usart1_r3, (uint8_t *)key3, 5 * 1024);

  FSM_request_pair_t usart1_r4;
  const char *key4 = "key4";
  FSM_request_pair_init(&usart1_r4, (uint8_t *)key4, 5 * 1024);

  queue_push(rr1.q, &usart1_r1, sizeof(FSM_request_pair_t));
  queue_push(rr1.q, &usart1_r2, sizeof(FSM_request_pair_t));
  queue_push(rr1.q, &usart1_r3, sizeof(FSM_request_pair_t));
  queue_push(rr1.q, &usart1_r4, sizeof(FSM_request_pair_t));
  /*********************** end request for usart 1 *************************/

  /*********************** start request for usart 2 *************************/

  /*********************** end request for usart 2 *************************/

  /*********************** start request for usart 6 *************************/

  /*********************** end request for usart 6 *************************/

  /*********************** start serving requests *************************/

  rr[0] = rr1;
  rr[1] = rr2;
  rr[2] = rr3;



  /***********************not for now****************************/
  // make it work for one source request  first
  //FSM_start_serving_request(rr, 1);
  /***********************not for now****************************/



  /*********************** end serving requests *************************/

  /*********************** post serving code *************************/

  printf(__usart1_print, "give me some data");

  while (Ring_buff_size(&usart1_ring_buffer) < 31)
    ;

  Ring_buff_read(&usart1_ring_buffer, buff, RING_BUFF_SIZE);
  printf(__usart1_print, "read from ring buffer");

  while (1)
    ;
  return 0;
}

void hang (void){
  while (1);
}

