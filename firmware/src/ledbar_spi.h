#include <os_type.h>

#ifndef _LEDBAR_SPI_H_
#define _LEDBAR_SPI_H_

#define LEDBAR_PIXELS 22

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} Color;

void ledbar_spi_init(void);

// Max. brightness: 0x1f
void ledbar_spi_send_data_part1(Color *data, uint8_t brightness);
void ledbar_spi_send_data_part2(Color *data, uint8_t brightness);

#endif
