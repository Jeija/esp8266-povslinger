/*
 * Main menu function - All LEDs to max. brightness
 */
#include <os_type.h>

#include "ledbar_spi.h"

#ifndef _FUNCTION_FLASHLIGHT_H
#define _FUNCTION_FLASHLIGHT_H

void function_flashlight(int32_t xpos, uint8_t ypos, uint32_t millis, Color *rgb);

#endif
