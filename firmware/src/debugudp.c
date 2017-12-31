#include <user_interface.h>
#include <os_type.h>
#include <espconn.h>
#include <osapi.h>
#include <mem.h>

#include "debugudp.h"

// Index in queue array corresponds to category type
static debugudp_pkg queue[DEBUGUDP_CATEGORYCOUNT];
static struct espconn *conn_debug;
static os_timer_t debugudp_timer;

// Initialize package queue, every packet starts with a byte for category identification
static void ICACHE_FLASH_ATTR debugudp_reset_queue(void) {
	uint8_t cat;
	for (cat = 0; cat < DEBUGUDP_CATEGORYCOUNT; ++cat) {
		queue[cat].length = 1;
		queue[cat].data[0] = cat;
	}
}

void ICACHE_FLASH_ATTR debugudp_init(void) {
	// Initialize UDP connection
	uint8_t debug_server_ip[] = { 192, 168, 0, 100 };
	conn_debug = (struct espconn *) os_zalloc(sizeof(struct espconn));
	conn_debug->type = ESPCONN_UDP;
	conn_debug->state = ESPCONN_NONE;
	conn_debug->proto.udp = (esp_udp *) os_zalloc(sizeof(esp_udp));
	conn_debug->proto.udp->local_port = espconn_port();
	conn_debug->proto.udp->remote_port = DEBUGUDP_PORT;
	os_memcpy(conn_debug->proto.udp->remote_ip, debug_server_ip, 4);

	// Reset category package queue
	debugudp_reset_queue();

	// Add transmission timer
	os_timer_setfn(&debugudp_timer, (os_timer_func_t *)debugudp_timer_cb, NULL);
	os_timer_arm(&debugudp_timer, DEBUGUDP_TIMER_PERIOD, 1);
}

void ICACHE_FLASH_ATTR debugudp_timer_cb(void) {
	espconn_create(conn_debug);

	uint8_t cat;
	for (cat = 0; cat < DEBUGUDP_CATEGORYCOUNT; ++cat)
		espconn_send(conn_debug, queue[cat].data, queue[cat].length);
	debugudp_reset_queue();

	espconn_delete(conn_debug);
}

void ICACHE_FLASH_ATTR debugudp_aggregate(enum debugudp_category category, uint16_t length, uint8_t *data) {
	// Additional information does not fit in queue packet for given category
	if (queue[category].length + length >= DEBUGUDP_PKGSIZE) {
		os_printf("[debugudp] package buffer overflow\n");
		return;
	}

	// Add information from packet to category-specific packet in queue
	os_memcpy(&queue[category].data[queue[category].length], data, length);
	queue[category].length += length;
}
