#include <ets_sys.h>

#ifndef _I2CUTIL_H
#define _I2CUTIL_H

// Most I²C devices use a commons protocol for reading / writing registers, e.g. MPU-9250 and the integrated AK8963
bool i2cdevice_read(uint8_t devaddr, uint8_t regaddr, uint8_t *data, uint8_t len);
bool i2cdevice_write(uint8_t devaddr, uint8_t regaddr, uint8_t *data, uint8_t len);

bool i2cdevice_writebyte(uint8_t devaddr, uint8_t regaddr, uint8_t data);
bool i2cdevice_readbyte(uint8_t devaddr, uint8_t regaddr, uint8_t *data);

bool i2cdevice_readbit(uint8_t regaddr, uint8_t bit, bool *val);
bool i2cdevice_writebit(uint8_t regaddr, uint8_t bit, bool val);

#endif
