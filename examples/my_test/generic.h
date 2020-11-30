#ifndef __GENERIC_H
#define __GENERIC_H

#include <stdint.h>
#include <glib.h>

#include "gattlib.h"

typedef struct{
	unsigned int len;
	unsigned char buf[2048];
}message_t;

typedef struct generic{
	uuid_t notify_uuid;
	uuid_t write_uuid;
	char gaddr[20];
	unsigned char gflag;
	void* adapter;
	gatt_connection_t* connection;
	GMainLoop *m_main_loop;
	message_t notify_msg;
	
}generic_t;


#endif
