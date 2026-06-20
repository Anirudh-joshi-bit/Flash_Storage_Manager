#pragma once

#include "defines.h"
#include <stdbool.h>
#include <stddef.h>

#define MAX(a, b) (a) > (b) ? (a): (b)
#define MIN(a, b) (a) < (b) ? (a): (b)


uint32_t strlen(const char *msg);
// pass the function pointer to printf
void printf(void (*usartx_print) (const char*, uint32_t),
                    const char *msg, ...);

void hang (void);








