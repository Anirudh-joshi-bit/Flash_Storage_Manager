#pragma once

#include "defines.h"
#include "device/stm32f401xe.h"
#include <stdbool.h>
#include <stddef.h>

uint32_t strlen(const char *msg);
void printf(const char *msg, uint32_t address);

