#define DEBUG
#include <stdbool.h>

#ifdef DEBUG
  #define DEBUG_printf(fun, str, ...) \
    printf(fun, "[DEBUG] " str, ##__VA_ARGS__) 

#else 
  #define  DEBUG_printf(fun, str, ...)
#endif

void DEBUG_assert (bool val);
void DEBUG_test (void);
