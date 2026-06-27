#include "DEBUG.h"
#include <stdint.h>
#include <stdarg.h>


void printf(void (*usartx_print) (const char*, uint32_t) ,
                  const char *msg, ...);
void __usart1_print(const char *msg, uint32_t size);

#ifdef DEBUG

  void __DEBUG_assert (const char* msg, const char *file, uint32_t line_num) {
    printf (__usart1_print, "%s in %s at %d", msg, file, line_num);
  }
  
  void __DEBUG_test (void){
     return ;
  }

#endif

