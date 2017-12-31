#ifndef _LIS2DH_H_
#define _LIS2DH_H_

#define LIS2DH_ADDR 0x18

enum lis2dh_datarate {
	LIS2DH_PWRDWN = 0,
	LIS2DH_1HZ,
	LIS2DH_10HZ,
	LIS2DH_25HZ,
	LIS2DH_50HZ,
	LIS2DH_100HZ,
	LIS2DH_200HZ,
	LIS2DH_400HZ,
	LIS2DH_1620HZ,
	LIS2DH_5376HZ
};

enum lis2dh_range {
	LIS2DH_2G = 0,
	LIS2DH_4G,
	LIS2DH_8G,
	LIS2DH_16G
};

typedef struct {
	int16_t x;
	int16_t y;
	int16_t z;
} vec3s16;

/*
 * LIS2DH driver
 * Functions with boolean return value return true if operation was successful
 * and false, if operation failed. If an error occurs during initialization, the
 * LIS2DH chip is most likely not attached to the I2C bus.
 */
bool lis2dh_init(enum lis2dh_datarate rate, enum lis2dh_range range);
void lis2dh_dump_cfg(void);
bool lis2dh_request_temperature(void);
bool lis2dh_get_temperature(int8_t *temp);

bool lis2dh_getacc(vec3s16 *out);
bool lis2dh_getx(int16_t *out);
bool lis2dh_gety(int16_t *out);
bool lis2dh_getz(int16_t *out);

/*
 * LIS2DH Vector operation utilities
 * expsmooth is an exponential smoothing operation
 * The smoothing factor must be provided as a rational fraction sf_numerator / sf_denominator,
 * so that fast integer operations can be used.
 */
vec3s16 lis2dh_expsmooth(vec3s16 current, vec3s16 previous, uint16_t sf_numerator, uint16_t sf_denominator);
vec3s16 lis2dh_vecadd(vec3s16 a, vec3s16 b);
vec3s16 lis2dh_vecmult(vec3s16 vec, int16_t scalar);

#endif
