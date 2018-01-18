#include "function_text.h"

void function_text(int32_t xpos, uint8_t ypos, uint32_t millis, Color *rgb) {
	rgb->r = 0x00;
	rgb->g = 0x00;
	rgb->b = 0x05;

	if (xpos >= 0) {
		if (text_getbit("POV Display!", xpos - 30 + ((millis / 50) % (12 * 6 + 40)), ypos))
			rgb->r = 0xff;
	}
}
