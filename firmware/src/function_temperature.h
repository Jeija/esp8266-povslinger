/*
 * Main menu function - Show temperature bar
 */
#include <os_type.h>

#include "ledbar_spi.h"

#ifndef _FUNCTION_TEMPERATURE_H
#define _FUNCTION_TEMPERATURE_H

void function_temperature(int32_t xpos, uint8_t ypos, uint32_t millis, Color *rgb);

#endif
