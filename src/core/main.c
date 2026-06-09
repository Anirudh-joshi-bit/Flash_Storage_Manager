#include "commons.h"
#include "defines.h"
#include "flash_storage_manager.h"
#include "ring_buff.h"
#include "usart.h"

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


int main(void) {

  // System init;
  __usart1_init();
  flash_init ();
  __usart1_print("hii there", 10);

  // fsm init
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
        fsm_flash_metadata)) {
    printf (__usart1_print,"error in FSM_init");
    hang ();
  }


  // fsm make requests
  // at this poiint, usart 2 and usart 6 are not working properly i dont know
  // why make request only on usrt 1 for now
  /*********************** start request for usart 1 *************************/

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

  // make it work for one source request  first
  FSM_start_serving_request(rr, 1);

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

