#include <os_type.h>

#ifndef _DEBUGUDP_H
#define _DEBUGUDP_H

/*
 * Utilities to automatically and periodically transmit debugging information in UDP packets
 * Constant package size is pre-allocated so that sending debug info is computationally efficient
 * (though not very memory efficient).
 *
 * debugudp automatically aggregates data for each debugging category,
 * e.g. a category for the accelerometer, for system events, WiFi events, ...
 * Debug information for every category is sent whenever debugudp_timer_cb is called.
 */
#define DEBUGUDP_PORT 1234
#define DEBUGUDP_QUEUELEN 10
#define DEBUGUDP_PKGSIZE 512
#define DEBUGUDP_TIMER_PERIOD 200

enum debugudp_category {
	DEBUGUDP_CAT_ACCELEROMETER = 0,
	DEBUGUDP_CATEGORYCOUNT
};

typedef struct {
	uint16_t length;
	uint8_t data[DEBUGUDP_PKGSIZE];
} debugudp_pkg;

void debugudp_init(void);
void debugudp_timer_cb(void);
void debugudp_aggregate(enum debugudp_category category, uint16_t length, uint8_t *data);

#define debugudp_aggregate8bit(category, byte) ((debugudp_aggregate(category, 1, (uint8_t *) &(byte))))
#define debugudp_aggregate16bit(category, halfword) ((debugudp_aggregate(category, 2, (uint8_t *) &(halfword))))
#define debugudp_aggregate32bit(category, word) ((debugudp_aggregate(category, 4,  (uint8_t *) &(word))))

#endif
