#include <os_type.h>

#ifndef _LEDBAR_H_
#define _LEDBAR_H_

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} Color;

void ledbar_init(void);

// Max. brightness: 0x1f
void ledbar_send_data(Color *data, uint8_t length, uint8_t brightness);

#endif
