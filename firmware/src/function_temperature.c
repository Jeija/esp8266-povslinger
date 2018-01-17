#include "lis2dh.h"
#include "function_temperature.h"

void function_temperature(int32_t xpos, uint8_t ypos, uint32_t millis, Color *rgb) {
	static int8_t temperature = 20;
	static uint32_t last_update = 0;

	// Update temperature approx. every 200ms
	if (millis - last_update > 200) {
		last_update = millis;
		int8_t t;
		lis2dh_dump_cfg();
		if(lis2dh_get_temperature(&t))
			temperature = t;
		lis2dh_request_temperature();
	}

	// Map temperature value to number of LEDs
	// 11 LEDs = 20 deg C, every LED corresponds to 1 deg C
	// Outside temperature range: All LEDs red
	if (temperature >= 20 - 11 && temperature <= 20 + 11) {
		uint8_t ledcount = temperature - 20 + 11;
		if (ledcount >= 22 - ypos)
			rgb->b = 0x10;
	} else {
		rgb->r = 0x01;
	}
}
