// System
#include <user_interface.h>
#include <ets_sys.h>
#include <os_type.h>
#include <osapi.h>
#include <gpio.h>

// Project
#include "user_config.h"
#include "i2c_master.h"
#include "ledbar_spi.h"
#include "i2cdevice.h"
#include "debugudp.h"
#include "font6x8.h"
#include "lis2dh.h"
#include "util.h"

// Globals
uint32_t millis = 0;
uint32_t swipe_starttime_current = 0;
uint32_t swipe_starttime_previous = 0;
uint32_t swipe_starttime_back_current = 0;

// Timers
os_timer_t ledbar_timer;
os_timer_t accelerometer_timer;

// TODO: use temperature sensor

/*
 * Empirical method to estimate a position value based on the stored swipe start / end
 * times.
 * The return value is the current position in the frame, between 0 and `width`.
 */
int32_t get_current_xpos(uint32_t width) {
	uint32_t swipe_duration = swipe_starttime_current - swipe_starttime_previous;
	uint32_t swipetime_fwd = millis - swipe_starttime_current;
	uint32_t swipetime_bwd = 0;

	// Check if we're already swiping back again
	bool back = false;

	// Next swipe has already started, but accelerometer hasn't detected that yet
	if (swipetime_fwd > swipe_duration)
		swipetime_fwd -= swipe_duration;

	// Probably swiping back, but accelerometer hasn't detected that yet (due to delay
	// caused by low pass filtering / signal processing)
	if (swipetime_fwd > swipe_duration / 2) {
		swipetime_bwd = swipetime_fwd - swipe_duration / 2;
		swipetime_fwd = swipe_duration / 2;
		back = true;
	}

	// Accelerometer has detected swiping back: Use more exact estimate
	if (swipe_starttime_back_current > swipe_starttime_current) {
		swipetime_bwd = millis - swipe_starttime_back_current;
		swipetime_fwd = swipe_starttime_back_current - swipe_starttime_current;
		back = true;
	}

	/*
	 * Use linear acceleration model:
	 *           swipe_duration
	 *             <-------->
	 *
	 * +1/2       /\        /\
	 *           /  \      /  \
	 *     -----/----\----/----\-------> time
	 *      \  /      \  /      \  /
	 * -1/2  \/        \/        \/
	 *
	 *        <--->          <--->
	 *      backwards       forwards
	 *         Movement direction
	 *
	 *        --->           ---->
	 *     swipetime_bwd  swipetime_fwd
	 *
	 * Further explanation and derivation of the equations below can be
	 * found in ../acceleration_model/explanation.tex
	 */
	float xpos = 0;
	if (!back) {
		float t = ((float) swipetime_fwd) / (swipe_duration / 2.0) - 0.5;
		xpos = t / 4 - t * t * t / 3 + 2. / 24;
	} else {
		float t = ((float) swipetime_bwd) / (swipe_duration / 2.0) - 0.5;
		xpos = 1. / 6 - (t / 4 - t * t * t / 3 + 2. / 24);
	}

	return (uint32_t) (xpos * 6 * width);
}

bool text_getbit(char *text, uint16_t x, uint16_t y) {
	uint16_t i = 0;
	while (text[i] != '\0') {
		if (font_6x8_getbit(text[i], x - 6 * i, y))
			return true;
		++i;
	}

	return false;
}

/*
 * LED bar output update timer
 * approx 500fps due to 2ms update period
 * (requires two updates per frame)
 *
 * The current typical swipe duration is swipe_starttime_current - swipe_starttime_previous
 */
#define LEDBAR_TIMER_PERIOD 1
void ledbar_timer_cb(void) {
	millis += LEDBAR_TIMER_PERIOD;

	Color leddata[22];
	uint8_t y;
	for (y = 0; y < 22; ++y) {
		int32_t xpos = get_current_xpos(22);
		/*leddata[y].r = (xpos + y) % 6 <= 2 ? 0xff : 0x00;
		leddata[y].g = 0x00;
		leddata[y].b = (xpos + y) % 6 >= 3 ? 0xff : 0x00;*/

		leddata[y].r = 0x00;
		leddata[y].g = 0x00;
		leddata[y].b = 0x05;

		if (xpos >= 0) {
			if (text_getbit("Hello World!", xpos - 30 + ((millis / 50) % (12 * 6 + 40)), y))
				leddata[y].r = 0xff;
		}
	}


	/*
	 * The hardware SPI FIFO doesn't have enough memory for all of the LED pixels.
	 * Therefore, two functions ledbar_spi_send_data_part1 and ledbar_spi_send_data_part2
	 * have to be called in alternating turns, they each only update some of the LED pixels.
	 */
	static uint8_t part;
	++part;
	if (part % 2 == 0)
		ledbar_spi_send_data_part1(leddata, 0x1f);
	else
		ledbar_spi_send_data_part2(leddata, 0x1f);
}

/*
 * Low-pass filter for swipe detection
 * sinc filter impulse response swipe_lpf generated using `../filterdesign/acc_lpf.m`
 */
const float swipelpf_ires[] = {0.01083, 0.01356, 0.01624, 0.01880, 0.02115, 0.02322, 0.02497, 0.02632, 0.02725, 0.02772, 0.02772, 0.02725, 0.02632, 0.02497, 0.02322, 0.02115, 0.01880, 0.01624, 0.01356, 0.01083};
#define SWIPELPF_IRES_LENGTH (sizeof(swipelpf_ires) / sizeof(swipelpf_ires[0]))

static uint8_t swipelpf_lastvalue_head = 0;
static int16_t swipelpf_lastvalues[SWIPELPF_IRES_LENGTH];

void swipelpf_retain(int16_t acc) {
	swipelpf_lastvalues[swipelpf_lastvalue_head] = acc;
	swipelpf_lastvalue_head++;
	if (swipelpf_lastvalue_head >= SWIPELPF_IRES_LENGTH)
		swipelpf_lastvalue_head = 0;
}

int16_t swipelpf_getfiltered(void) {
	float retval = 0;
	uint16_t i;

	// Convolute previously stored values with impulse response
	for (i = 0; i < SWIPELPF_IRES_LENGTH; ++i) {
		uint16_t valueIdx = (swipelpf_lastvalue_head + 1 + i) % SWIPELPF_IRES_LENGTH;
		retval += swipelpf_lastvalues[valueIdx] * swipelpf_ires[i];
	}

	return (int16_t) retval;
}

/*
 * Watch accelerometer values and set control variables for LED bar output function
 *
 * First, the constant component (gravity) is removed by removing the component
 * produced by an exponential smoothing filter.
 *
 * Let `signal` be the z-axis acceleration values. Then the beginning of a swipe
 * action can be detected by finding the maxima in `signal`. In order to reduce
 * noise, we reframe the question to finding positive-to-negative sign changes in
 * diff(conv(signal, swipelpf_ires))
 */
#define ACCELEROMETER_TIMER_PERIOD 2
#define ACCELEROMETER_SMOOTHING_NUMERATOR 1
#define ACCELEROMETER_SMOOTHING_DENOMINATOR 50
bool accelerometer_previous_diff_positive = true;
vec3s16 lis2dh_lpf;

void accelerometer_timer_cb(void) {
	vec3s16 acc;
	lis2dh_getx(&acc.x);
	acc.y = 0;
	acc.z = 0;
	//lis2dh_getacc(&acc);

	// Restart accelerometer if chip has (likely) crashed
	if (acc.x == 0 && acc.y == 0 && acc.z == 0) {
		os_printf("LIS2DH crashed, reinit\r\n");
		lis2dh_init(LIS2DH_200HZ, LIS2DH_4G);
	}

	// Gravity compensation: Remove low-pass-filtered component (approximation for gravity) from acceleration
	lis2dh_lpf = lis2dh_expsmooth(acc, lis2dh_lpf, ACCELEROMETER_SMOOTHING_NUMERATOR, ACCELEROMETER_SMOOTHING_DENOMINATOR);
	acc = lis2dh_vecadd(acc, lis2dh_vecmult(lis2dh_lpf, -1));

	// Remember last values for peak detection using low pass filter,
	// convolute with low pass filter impulse response, differentiate
	static int16_t filtered_current = 0;
	static int16_t filtered_previous = 0;

	swipelpf_retain(acc.x);
	filtered_previous = filtered_current;
	filtered_current = swipelpf_getfiltered();

	int16_t diff = filtered_current - filtered_previous;

	// Detect positive-to-negative sign changes
	// This detection completely fails if accelerometer is not in swiping movement, but we don't care
	// Due to filtering with the low pass filter, sign change detection is delayed by
	// `ACCELEROMETER_TIMER_PERIOD * SWIPELPF_IRES_LENGTH / 2` milliseconds
	if (accelerometer_previous_diff_positive && diff < 0) {
		accelerometer_previous_diff_positive = false;

		swipe_starttime_previous = swipe_starttime_current;
		swipe_starttime_current = millis - ACCELEROMETER_TIMER_PERIOD * SWIPELPF_IRES_LENGTH / 2;
	} else if (!accelerometer_previous_diff_positive && diff > 0) {
		swipe_starttime_back_current = millis - ACCELEROMETER_TIMER_PERIOD * SWIPELPF_IRES_LENGTH / 2;
		accelerometer_previous_diff_positive = true;
	}

	debugudp_aggregate16bit(DEBUGUDP_CAT_ACCELEROMETER, acc.x);
}

/*
 * Initialization
 */
void ICACHE_FLASH_ATTR wifi_init(void) {
	wifi_set_opmode(STATION_MODE);

	// These credentials are used for development only and may be published
	char ssid[32] = "pov_testnet";
	char pass[64] = "wothNonjeOgFijutFeHuchzelsOltulimyoddIsh";

	struct station_config sta_conf;
	sta_conf.bssid_set = 0;
	os_memcpy(&sta_conf.ssid, ssid, 32);
	os_memcpy(&sta_conf.password, pass, 64);

	wifi_station_set_config(&sta_conf);
	wifi_station_connect();
}

void ICACHE_FLASH_ATTR user_init(void) {
	/*** Initialization ***/
	uart_div_modify(0, UART_CLK_FREQ / BAUD);
	os_delay_us(1000);
	i2c_master_gpio_init();
	ledbar_spi_init();
	debugudp_init();
	wifi_init();
	if (!lis2dh_init(LIS2DH_400HZ, LIS2DH_4G))
		os_printf("LIS2DH init failed!");

	os_printf("\r\n\r\nESP8266 starting up with baudrate %d\r\n", BAUD);

	/*** Start all timers ***/
	os_delay_us(10000);

	os_timer_setfn(&ledbar_timer, (os_timer_func_t *)ledbar_timer_cb, NULL);
	os_timer_arm(&ledbar_timer, LEDBAR_TIMER_PERIOD, 1);

	os_timer_setfn(&accelerometer_timer, (os_timer_func_t *)accelerometer_timer_cb, NULL);
	os_timer_arm(&accelerometer_timer, ACCELEROMETER_TIMER_PERIOD, 1);
}
