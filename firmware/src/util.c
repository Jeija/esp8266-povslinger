// System
#include <ets_sys.h>
#include <osapi.h>
#include <gpio.h>
#include <os_type.h>
#include <user_interface.h>

// Configuration
#include "user_config.h"

// Project
#include "i2c_master.h"
#include "util.h"

void i2c_scanbus() {
	// Scan I²C Bus
	os_printf("\r\n=== Scanning I²C Bus ===\r\n");
	uint8_t i;
	for (i = 0; i < 0xff; ++i) {
		i2c_master_start();
		i2c_master_writeByte(i);
		if (!i2c_master_getAck()) os_printf("ACK for address: 0x%02x\r\n", i);
		i2c_master_stop();
	}
	os_printf("========================\r\n\r\n");
}
