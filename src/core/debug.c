#include "DEBUG.h"
#include <stdint.h>
#include <stdarg.h>
#include "flash_storage_manager.h"

void printf(void (*usartx_print) (const char*, uint32_t) ,
                  const char *msg, ...);


#ifdef DEBUG

  void DEBUG_assert (bool val) {

    if (!val)
      while (1);

    return ;
  }
  
  void DEBUG_test (void){
     return ;
  }

#else 

  void DEBUG_assert (bool val) {
    return ;
  }

#endif

