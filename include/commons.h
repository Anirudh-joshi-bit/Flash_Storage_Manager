#pragma once

#include "defines.h"
#include "device/stm32f401xe.h"
#include <stdbool.h>
#include <stddef.h>

uint32_t strlen(const char *msg);
// pass the function pointer to printf
void printf(void (*usartx_print) (const char*, uint32_t), const char *msg, ...);



