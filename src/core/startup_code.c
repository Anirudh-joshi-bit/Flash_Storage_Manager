#include "commons.h"
#include <stdint.h>
#include <stdlib.h>

/*linker script symbols*/
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sidata;
extern uint32_t _sbss;
extern uint32_t _ebss;


void resetHandlerHelper(void) {
  uint32_t dest = (uint32_t)&_sdata;
  uint32_t dest_end = (uint32_t)&_edata;
  uint32_t src = (uint32_t)&_sidata;

  /*copy data from data section to ram
   * init .bss section*/
  for (uint32_t i = dest; i <= dest_end; i += 4) {
    *(uint32_t *)(i) = *(uint32_t *)(src);
    src += 4;
  }

  for (uint32_t i = (uint32_t)&_sbss; i <= (uint32_t)&_ebss; i += 4) {
    *(uint32_t *)(i) = 0;
  }
}

