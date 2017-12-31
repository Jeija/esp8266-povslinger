// System
#include <ets_sys.h>
#include <osapi.h>

// Project
#include "i2c_master.h"
#include "util.h"

#define I2C_ADDR_W(addr) ((addr<<1))
#define I2C_ADDR_R(addr) ((addr<<1) | 0x01)

bool ICACHE_FLASH_ATTR i2cdevice_read(uint8_t devaddr, uint8_t regaddr, uint8_t *data, uint8_t len) {
	i2c_master_start();

	// Write I²C Address for writing register
	i2c_master_writeByte(I2C_ADDR_W(devaddr));
	if (i2c_master_getAck()) {
		os_printf("I²C device %x R: no ACK for write addr\r\n", devaddr);
		i2c_master_stop();
		return false;
	}

	// Write register address
	i2c_master_writeByte(regaddr);
	if (i2c_master_getAck()) {
		os_printf("I²C device %x R: no ACK for register\r\n", devaddr);
		i2c_master_stop();
		return false;
	}

	i2c_master_start();

	// Write I²C Address for reading
	i2c_master_writeByte(I2C_ADDR_R(devaddr));
	if (i2c_master_getAck()) {
		os_printf("I²C device %x R: no ACK for read addr\r\n", devaddr);
		i2c_master_stop();
		return false;
	}

	uint8_t i;
	for (i = 0; i < len; ++i) {
		data[i] = i2c_master_readByte();
		if (i != len - 1)
			i2c_master_send_ack();
		else
			i2c_master_send_nack();
	}

	i2c_master_stop();
	//os_printf("Just read %x from %x\r\n", *data, regaddr);

	return true;
}

bool ICACHE_FLASH_ATTR i2cdevice_write(uint8_t devaddr, uint8_t regaddr, uint8_t *data, uint8_t len) {
	i2c_master_start();

	// Write I²C Address for writing register
	i2c_master_writeByte(I2C_ADDR_W(devaddr));
	if (i2c_master_getAck()) {
		os_printf("I²C device %x W: no ACK for write addr\r\n", devaddr);
		i2c_master_stop();
		return false;
	}

	// Write register address
	i2c_master_writeByte(regaddr);
	if (i2c_master_getAck()) {
		os_printf("I²C device %x W: no ACK for register\r\n", devaddr);
		i2c_master_stop();
		return false;
	}

	uint8_t i;
	for (i = 0; i < len; ++i) {
		i2c_master_writeByte(data[i]);
		if (i2c_master_getAck()) {
			os_printf("I²C device %x W: no ACK for datawrite\r\n", devaddr);
			i2c_master_stop();
			return false;
		}
	}

	i2c_master_stop();

	return true;
}

// Single-byte operations
inline bool i2cdevice_writebyte(uint8_t devaddr, uint8_t regaddr, uint8_t data) {
	return i2cdevice_write(devaddr, regaddr, &data, 1);
}

inline bool i2cdevice_readbyte(uint8_t devaddr, uint8_t regaddr, uint8_t *data) {
	return i2cdevice_read(devaddr, regaddr, data, 1);
}

// Get bit in any register
bool ICACHE_FLASH_ATTR i2cdevice_readbit(uint8_t devaddr, uint8_t regaddr, uint8_t bit, bool *val) {
	uint8_t dat;
	if (!i2cdevice_read(devaddr, regaddr, &dat, 1)) return false;
	*val = (gbi(dat, bit) != false);
	return true;
}

// Set bit in any register
bool ICACHE_FLASH_ATTR i2cdevice_writebit(uint8_t devaddr, uint8_t regaddr, uint8_t bit, bool val) {
	uint8_t dat;
	if (!i2cdevice_read(devaddr, regaddr, &dat, 1)) return false;
	if (val)
		sbi(dat, bit);
	else
		cbi(dat, bit);
	if (!i2cdevice_write(devaddr, regaddr, &dat, 1)) return false;
	return true;
}
