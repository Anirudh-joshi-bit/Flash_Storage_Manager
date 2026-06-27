#define DEBUG
#include <stdbool.h>
#include <stdint.h>

#ifdef DEBUG
  #define DEBUG_printf(fun, str, ...) \
    printf(fun, "[DEBUG]    " str, ##__VA_ARGS__) 

  #define DEBUG_assert(expr) \
    ((expr) ? 0 : __DEBUG_assert("[ASSERTION FAILED]    " #expr, __FILE__, __LINE__))


#else 
  #define  DEBUG_printf(fun, str, ...)
  #define DEBUG_assert(expr)

#endif


void __DEBUG_assert (const char* expr, const char *file, uint32_t line);
void __DEBUG_test (void);
