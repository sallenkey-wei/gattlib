#ifndef __GENERIC_H
#define __GENERIC_H

#include <stdint.h>
#include <glib.h>
#include <sys/queue.h>

#include "gattlib.h"

typedef struct{
	uint16_t len;
	uint8_t buf[2048];
}message_t;


struct msg_queue{
	message_t msg;
	TAILQ_ENTRY(msg_queue) entries;
};

typedef struct generic{
	uuid_t notify_uuid;
	uuid_t write_uuid;
	char gaddr[20];
	unsigned char gflag;
	void* adapter;
	gatt_connection_t* connection;
	GMainLoop *m_main_loop;
	message_t notify_msg;
	uint8_t vender_keyA[16];
	char * vender_keyB;
	uint32_t security_key[4];
	volatile uint8_t running;
	int ble_setup_ret;
	TAILQ_HEAD(msg_head, msg_queue) notify_head;
	pthread_mutex_t mutex;
	pthread_t g_thread;
	pthread_cond_t notify_cond;
	
}generic_t;


#endif
