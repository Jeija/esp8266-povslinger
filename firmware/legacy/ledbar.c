#include <ets_sys.h>
#include <osapi.h>
#include <gpio.h>
#include <os_type.h>

#include "ledbar.h"
#include "util.h"

/*
 * APA 102-2020 LED bar
 * Inverted Data: GPIO13
 * Inverted Clock: GPIO14
 */

#define LED_DAT_LOW() ((GPIO_OUTPUT_SET(GPIO_ID_PIN(13), 1)))
#define LED_DAT_HIGH() ((GPIO_OUTPUT_SET(GPIO_ID_PIN(13), 0)))
#define LED_CLK_LOW() ((GPIO_OUTPUT_SET(GPIO_ID_PIN(14), 1)))
#define LED_CLK_HIGH() ((GPIO_OUTPUT_SET(GPIO_ID_PIN(14), 0)))

void ledbar_init(void) {
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14);
}

void ledbar_send_byte(uint8_t byte) {
	uint8_t i;

	for (i = 0; i < 8; ++i) {
		LED_CLK_LOW();

		if (gbi(byte, i))
			LED_DAT_HIGH();
		else
			LED_DAT_LOW();

		os_delay_us(1);
		LED_CLK_HIGH();
		os_delay_us(1);
	}
}

void ledbar_send_data(Color *data, uint8_t datasize, uint8_t brightness) {
	LED_CLK_LOW();
	LED_DAT_LOW();
	os_delay_us(5);

	// Start Frame
	ledbar_send_byte(0x00);
	ledbar_send_byte(0x00);
	ledbar_send_byte(0x00);
	ledbar_send_byte(0x00);

	// Data
	uint8_t led;
	for (led = 0; led <= datasize; ++led) {
		ledbar_send_byte(0xe0 + brightness);
		ledbar_send_byte(data[led].b);
		ledbar_send_byte(data[led].g);
		ledbar_send_byte(data[led].r);
	}

	// End frame
	ledbar_send_byte(0xff);
	ledbar_send_byte(0xff);
	ledbar_send_byte(0xff);
	ledbar_send_byte(0xff);
	ledbar_send_byte(0xff);
	ledbar_send_byte(0xff);
}
