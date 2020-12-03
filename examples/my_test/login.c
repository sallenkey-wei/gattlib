/*
 *
 *  GattLib - GATT Library
 *
 *  Copyright (C) 2016-2019  Olivier Martin <olivier@labapart.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "generic.h"
#include "dealevt.h"
#include "ble_notify.h"

#define USE_ADAPTER 0

#define BLE_SCAN_TIMEOUT   4

#define NOTIFY_UUID		"0000B001-FF4D-414E-5349-4f4E5F534552"
#define WRITE_UUID		"0000B002-FF4D-414E-5349-4f4E5F534552"


static void usage(char *argv[]) {
	printf("%s <device_address>\n", argv[0]);
}

static generic_t param = {
	.gaddr = {0},
	.gflag = 0,
	.adapter = NULL,
	.connection = NULL,
	.m_main_loop = NULL,
	.notify_msg = {
					.len = 0, 
					.buf = {0},
				  },
	.notify_head = TAILQ_HEAD_INITIALIZER(param.notify_head),
	.running = 0,
};

void on_user_abort(int arg) {
	extern generic_t param;
	g_main_loop_quit(param.m_main_loop);
}


static int init(generic_t * para, char * addr){
	if (gattlib_string_to_uuid(WRITE_UUID, strlen(WRITE_UUID) + 1, &para->write_uuid) < 0) {
		printf("<init>:write uuid error.\n");
		return 1;
	}

	if (gattlib_string_to_uuid(NOTIFY_UUID, strlen(NOTIFY_UUID) + 1, &para->notify_uuid) < 0) {
		printf("<init>:notify uuid error.\n");
		return 1;
	}
	if(addr == NULL){
		printf("<%s>: addr is null\n", __FUNCTION__);
	}
	strcpy(para->gaddr, addr);
	
	select_vender_keyB(para, default_vender);

	pthread_mutex_init(&para->mutex, NULL);
	pthread_cond_init(&para->notify_cond, NULL);
		
	return 0;
}

int ble_wirte(generic_t * para, message_t * msg){	
	int ret = 0;
	if(!para->running){
		printf("<%s>: para->running is 0.\n", __FUNCTION__);
		return -1;
	}
	ret = gattlib_write_char_by_uuid(para->connection, &para->write_uuid, msg->buf, msg->len);
	if (ret != GATTLIB_SUCCESS) {
		char uuid_str[MAX_LEN_UUID_STR + 1];

		gattlib_uuid_to_string(&para->write_uuid, uuid_str, sizeof(uuid_str));

		if (ret == GATTLIB_NOT_FOUND) {
			fprintf(stderr, "Could not find GATT Characteristic with UUID %s. "
				"You might call the program with '--gatt-discovery'.\n", uuid_str);
		} else {
			fprintf(stderr, "Error while writing GATT Characteristic with UUID %s (ret:%d)\n",
				uuid_str, ret);
		}
	}
	return ret;
}

static void ble_discovered_device(void *adapter, const char* addr, const char* name, void *user_data) {
	int ret;
	generic_t * para = user_data;
	if (name) {
		printf("Discovered %s - '%s'\n", addr, name);
	} else {
		printf("Discovered %s\n", addr);
	}

	if(para){
		if(strcmp(para->gaddr, addr) == 0){
			para->gflag = 1;
		}
	}
	else{
		printf("<%s>: para is null\n", __FUNCTION__);
	}
}



int main(int argc, char *argv[]) {
	int i, ret;
	message_t * notify_msg;
	message_t out_msg;
	int timeout_cnt = 0;
	
	gattlib_characteristic_t* characteristics;
	int characteristics_count;
	char uuid_str[MAX_LEN_UUID_STR + 1];

	if(argc < 2){
		usage(argv);
	}
	
	ret = init(&param, argv[1]);
	if(ret){
		return ret;
	}

#if USE_ADAPTER
	ret = gattlib_adapter_open(NULL, &param.adapter);
	if (ret) {
		fprintf(stderr, "ERROR: Failed to open adapter.\n");
		return 1;
	}

	ret = gattlib_adapter_scan_enable(param.adapter, ble_discovered_device, BLE_SCAN_TIMEOUT, &param);
	if (ret) {
		fprintf(stderr, "ERROR: Failed to scan.\n");
		goto adapter_close_exit;
	}
	
	gattlib_adapter_scan_disable(param.adapter);

	if(param.gflag == 0){
		printf("ERROR: Failed to find %s\n", param.gaddr);
		goto adapter_close_exit;
	}
#endif

	ret = ble_setup(&param);
	if(ret){
		goto ble_cleanup_exit;
	}
	notify_msg = (message_t *)malloc(sizeof(message_t));
	while(get_data_from_notify_queue(&param, notify_msg)){
		timeout_cnt++;
		if(timeout_cnt >= 3){
			printf("<%s>: get_data_from_notify_queue time out more than %d times.\n", timeout_cnt);
			free(notify_msg);
			goto ble_cleanup_exit;
		}
		
	}
	
	deal_notify_event(&param, notify_msg, out_msg.buf, &out_msg.len);
	free(notify_msg);

	ret = ble_wirte(&param, &out_msg);
	printf("send below data to ble:\n");
	for(i = 0; i < out_msg.len; i++){
			printf("%02x ", out_msg.buf[i]);
	}
	printf("\n");
	if(ret){
		goto ble_cleanup_exit;
	}
	

	#if 0
	ret = gattlib_discover_char(connection, &characteristics, &characteristics_count);
	if (ret != 0) {
		fprintf(stderr, "Fail to discover characteristics.\n");
		goto disconnect_exit;
	}
	for (i = 0; i < characteristics_count; i++) {
		gattlib_uuid_to_string(&characteristics[i].uuid, uuid_str, sizeof(uuid_str));

		printf("characteristic[%d] properties:%02x value_handle:%04x uuid:%s\n", i,
				characteristics[i].properties, characteristics[i].value_handle,
				uuid_str);
	}
	free(characteristics);
	#endif


	
#if 0
	if (g_operation == READ) {
		uint8_t *buffer = NULL;
		printf("Read UUID completed: ");
		fflush(stdout);
		//while(1){
			ret = gattlib_read_char_by_uuid(connection, &read_uuid, (void **)&buffer, &len);
			if (ret != GATTLIB_SUCCESS) {
				char uuid_str[MAX_LEN_UUID_STR + 1];

				gattlib_uuid_to_string(&read_uuid, uuid_str, sizeof(uuid_str));

				if (ret == GATTLIB_NOT_FOUND) {
					fprintf(stderr, "Could not find GATT Characteristic with UUID %s. "
						"You might call the program with '--gatt-discovery'.\n", uuid_str);
				} else {
					fprintf(stderr, "Error while reading GATT Characteristic with UUID %s (ret:%d)\n", uuid_str, ret);
				}
				printf("\n");
				goto EXIT;
			}

			
			for (i = 0; i < len; i++) {
				printf("%02x ", buffer[i]);
			}
			

			free(buffer);
			fflush(stdout);
		//}
		printf("\n");
	} else 

	{

	}
#endif
ble_cleanup_exit:	
	ble_cleanup(&param);
adapter_close_exit:
#if USE_ADAPTER
	gattlib_adapter_close(param.adapter);
#endif
	puts("Done");
	return ret;
}
