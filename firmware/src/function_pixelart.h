/*
 * Main menu function - Show random pixel art images
 */
#include <os_type.h>

#include "ledbar_spi.h"

#ifndef _FUNCTION_PIXELART_H
#define _FUNCTION_PIXELART_H

void function_pixelart(int32_t xpos, uint8_t ypos, uint32_t millis, Color *rgb);

#endif
