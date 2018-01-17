#include <osapi.h>

#include "lis2dh_registers.h"
#include "i2cdevice.h"
#include "lis2dh.h"

/*** Internal I²C functions ***/
#define lis2dh_write(regaddr, data, len) (i2cdevice_write(LIS2DH_ADDR, regaddr, data, len))
#define lis2dh_read(regaddr, data, len) (i2cdevice_read(LIS2DH_ADDR, regaddr, data, len))
#define lis2dh_readbyte(regaddr, data) (i2cdevice_readbyte(LIS2DH_ADDR, regaddr, data))
#define lis2dh_writebyte(regaddr, data) (i2cdevice_writebyte(LIS2DH_ADDR, regaddr, data))
#define lis2dh_readbit(regaddr, bit, val) (i2cdevice_readbit(LIS2DH_ADDR, regaddr, bit, val))
#define lis2dh_writebit(regaddr, bit, val) (i2cdevice_writebit(LIS2DH_ADDR, regaddr, bit, val))

#define REGADDR_AUTOINCREMENT 0x80

bool ICACHE_FLASH_ATTR lis2dh_read_lh_registers(uint8_t lregister, uint16_t *out) {
	uint8_t msb_lsb[2];
	bool succ;

	succ = lis2dh_read(REGADDR_AUTOINCREMENT | lregister, msb_lsb, 2);
	*out = (((uint16_t) msb_lsb[1]) << 8) | msb_lsb[0];

	return succ;
}

bool ICACHE_FLASH_ATTR lis2dh_init(enum lis2dh_datarate rate, enum lis2dh_range range) {
	uint8_t succ;

	// Check Connection
	uint8_t whoami;
	succ = lis2dh_readbyte(LIS2DH_WHO_AM_I, &whoami);
	if (whoami != LIS2DH_I_AM_MASK)
		return false;

	// Enable Temperature Sensor
	succ = succ & lis2dh_writebyte(LIS2DH_TEMP_CFG_REG, LIS2DH_TEMP_EN_MASK);

	// Enable XYZ Accelerometer Axes, disable low power mode, configure data rate
	// Data rate configurtion: 0b0110 = 200 Hz, see "Data rate configuration" in Datasheet
	succ = succ & lis2dh_writebyte(LIS2DH_CTRL_REG1, ((rate << 4) | LIS2DH_ODR_MASK) | LIS2DH_XYZ_EN_MASK);

	// Set acceleration range, set high resolution mode (HR) and enable Block Data Update (BDU)
	// makes sure the high and low register contain sensor values for the same sample
	succ = succ & lis2dh_writebyte(LIS2DH_CTRL_REG4, ((range << 4) | LIS2DH_FS_MASK) | LIS2DH_HR_MASK | LIS2DH_BDU_MASK);

	return true;
}

/*
 * Debugging
 */
void ICACHE_FLASH_ATTR lis2dh_dump_cfg(void) {
	uint8_t stat, reg1, reg2, reg3, reg4, tempcfg;
	lis2dh_readbyte(LIS2DH_STATUS_REG_AUX, &stat);
	lis2dh_readbyte(LIS2DH_CTRL_REG1, &reg1);
	lis2dh_readbyte(LIS2DH_CTRL_REG2, &reg2);
	lis2dh_readbyte(LIS2DH_CTRL_REG3, &reg3);
	lis2dh_readbyte(LIS2DH_CTRL_REG4, &reg4);
	lis2dh_readbyte(LIS2DH_TEMP_CFG_REG, &tempcfg);

	os_printf("\r\nLIS2DH Register Dump:\r\n");
	os_printf("stat: 0x%02x\r\n", stat);
	os_printf("reg1: 0x%02x\r\n", reg1);
	os_printf("reg2: 0x%02x\r\n", reg2);
	os_printf("reg3: 0x%02x\r\n", reg3);
	os_printf("reg4: 0x%02x\r\n", reg4);
	os_printf("tempcfg: 0x%02x\r\n", tempcfg);
	os_printf("\r\n");
}

/*
 * Temperature Sensing
 * TODO: not working
 */
bool ICACHE_FLASH_ATTR lis2dh_request_temperature(void) {
	return lis2dh_writebyte(LIS2DH_TEMP_CFG_REG, LIS2DH_TEMP_EN_MASK);
}

bool ICACHE_FLASH_ATTR lis2dh_get_temperature(int8_t *temp) {
	uint8_t stat;
	lis2dh_readbyte(LIS2DH_STATUS_REG_AUX, &stat);

	// Datasheet specifies that both TEMP_H and TEMP_L register have to be read,
	// but actual temperature value in degrees Celsius is stored in TEMP_H.
	if (stat & LIS2DH_TDA_MASK) {
		uint16_t temp_reg;
		bool succ = lis2dh_read_lh_registers(LIS2DH_OUT_TEMP_L, &temp_reg);
		*temp = (int8_t) (temp_reg >> 8);
		return succ;
	}

	return false;
}

/*
 * Accelerometer Sensing
 */
bool lis2dh_getx(int16_t *out) {
	return lis2dh_read_lh_registers(LIS2DH_OUT_X_L, (uint16_t *) out);
}

bool lis2dh_gety(int16_t *out) {
	return lis2dh_read_lh_registers(LIS2DH_OUT_Y_L, (uint16_t *) out);
}

bool lis2dh_getz(int16_t *out) {
	return lis2dh_read_lh_registers(LIS2DH_OUT_Z_L, (uint16_t *) out);
}

bool lis2dh_getacc(vec3s16 *out) {
	uint8_t xyz_msb_lsb[6];
	bool succ;

	succ = lis2dh_read(REGADDR_AUTOINCREMENT | LIS2DH_OUT_X_L, xyz_msb_lsb, 6);
	out->x = (((uint16_t) xyz_msb_lsb[1]) << 8) | xyz_msb_lsb[0];
	out->y = (((uint16_t) xyz_msb_lsb[3]) << 8) | xyz_msb_lsb[2];
	out->z = (((uint16_t) xyz_msb_lsb[5]) << 8) | xyz_msb_lsb[4];

	return succ;
}

/*
 * Vector operation utilities
 */
vec3s16 ICACHE_FLASH_ATTR lis2dh_expsmooth(vec3s16 current, vec3s16 previous, uint16_t sf_numerator, uint16_t sf_denominator) {
	current.x = (sf_denominator - sf_numerator) * previous.x / sf_denominator + sf_numerator * current.x / sf_denominator;
	current.y = (sf_denominator - sf_numerator) * previous.y / sf_denominator + sf_numerator * current.y / sf_denominator;
	current.z = (sf_denominator - sf_numerator) * previous.z / sf_denominator + sf_numerator * current.z / sf_denominator;

	return current;
}

vec3s16 ICACHE_FLASH_ATTR lis2dh_vecadd(vec3s16 a, vec3s16 b) {
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;

	return a;
}

vec3s16 ICACHE_FLASH_ATTR lis2dh_vecmult(vec3s16 vec, int16_t scalar) {
	vec.x *= scalar;
	vec.y *= scalar;
	vec.z *= scalar;

	return vec;
}
