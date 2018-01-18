#include "function_pixelart.h"
#include "function_pixelart_data.h"

#include <osapi.h>
// Get color channel for pixel at (x, y): 0 <= x < 12, 0 <= y < 17
// offset = byte offset in data, color = 0, 1, 2 for R, G, B
uint8_t pixelart_getbyte(uint16_t picnum, uint32_t x, uint32_t y, uint8_t color) {
	uint32_t res;

	if (picnum >= PIXELART_IMGCOUNT)
		return 0x00;

	uint32_t offset = pixelart_indices[picnum * 3];
	uint32_t width = pixelart_indices[picnum * 3 + 1];
	uint32_t height = pixelart_indices[picnum * 3 + 2];

	if (x >= width || y >= height)
		return 0x00;

	const uint8_t *addr = &pixelart_data[offset + (y * width + x) * 3 + color];

	// Source: https://github.com/esp8266/Arduino/blob/master/cores/esp8266/pgmspace.h
	asm(	"extui	%0, %1, 0, 2\n"		/* Extract offset within word (in bytes) */ \
		"sub	%1, %1, %0\n"		/* Subtract offset from addr, yielding an aligned address */ \
		"l32i.n	%1, %1, 0x0\n"		/* Load word from aligned address */ \
		"slli	%0, %0, 3\n"		/* Mulitiply offset by 8, yielding an offset in bits */ \
		"ssr	%0\n"			/* Prepare to shift by offset (in bits) */ \
		"srl	%0, %1\n"		/* Shift right; now the requested byte is the first one */ \
		:"=r"(res), "=r"(addr) \
		:"1"(addr) \
	:);

	return (uint8_t) res;
}

#define NEW_PICTURE_PERIOD 5000
void function_pixelart(int32_t xpos, uint8_t ypos, uint32_t millis, Color *rgb) {
	// Choose new random picture every NEW_PICTURE_PERIOD milliseconds
	static uint8_t picnum = 0;
	static uint32_t last_new_picnum = 0;
	if (millis - last_new_picnum > NEW_PICTURE_PERIOD) {
		last_new_picnum = millis;
		picnum = os_random() % PIXELART_IMGCOUNT;
	}


	rgb->r = pixelart_getbyte(picnum, xpos, ypos, 0);
	rgb->g = pixelart_getbyte(picnum, xpos, ypos, 1);
	rgb->b = pixelart_getbyte(picnum, xpos, ypos, 2);
}
