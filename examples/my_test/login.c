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

#include "gattlib.h"

#define BLE_SCAN_TIMEOUT   4

#define WRITE_UUID		"0000B001-FF4D-414E-5349-4f4E5F534552"
#define READ_UUID		"0000B002-FF4D-414E-5349-4f4E5F534552"


typedef enum { READ, WRITE} operation_t;
operation_t g_operation = READ;

static uuid_t read_uuid;
static uuid_t write_uuid;
long int value_data;
pthread_t gthread;
unsigned char gflag = 0;
char gaddr[20] = {0};

static void usage(char *argv[]) {
	printf("%s <device_address>\n", argv[0]);
}

static void *ble_connect_device(void *arg) {
	
	char* addr = arg;
	gatt_connection_t* gatt_connection;
	gattlib_primary_service_t* services;
	gattlib_characteristic_t* characteristics;
	int services_count, characteristics_count;
	char uuid_str[MAX_LEN_UUID_STR + 1];
	int ret, i;

	printf("------------START %s ---------------\n", addr);

	gatt_connection = gattlib_connect(NULL, addr, GATTLIB_CONNECTION_OPTIONS_LEGACY_DEFAULT);
	if (gatt_connection == NULL) {
		fprintf(stderr, "Fail to connect to the bluetooth device.\n");
		goto connection_exit;
	} else {
		puts("Succeeded to connect to the bluetooth device.");
	}

	ret = gattlib_discover_primary(gatt_connection, &services, &services_count);
	if (ret != 0) {
		fprintf(stderr, "Fail to discover primary services.\n");
		goto disconnect_exit;
	}

	for (i = 0; i < services_count; i++) {
		gattlib_uuid_to_string(&services[i].uuid, uuid_str, sizeof(uuid_str));

		printf("service[%d] start_handle:%02x end_handle:%02x uuid:%s\n", i,
				services[i].attr_handle_start, services[i].attr_handle_end,
				uuid_str);
	}
	free(services);

	ret = gattlib_discover_char(gatt_connection, &characteristics, &characteristics_count);
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

disconnect_exit:
	gattlib_disconnect(gatt_connection);

connection_exit:
	printf("------------DONE %s ---------------\n", addr);
	return NULL;
}

static void ble_discovered_device(void *adapter, const char* addr, const char* name, void *user_data) {
	int ret;

	if (name) {
		printf("Discovered %s - '%s'\n", addr, name);
	} else {
		printf("Discovered %s\n", addr);
	}
	
	if(strcmp(gaddr, addr) == 0){
		gflag = 1;
	}
	#if 0
	ret = pthread_create(&gthread, NULL,	ble_connect_device, gaddr);
	if (ret != 0) {
		fprintf(stderr, "Failt to create BLE connection thread.\n");
		return;
	}
	#endif
}


int main(int argc, char *argv[]) {
	int i, ret;
	size_t len = 100;
	void* adapter;
	gatt_connection_t* connection;
	gattlib_characteristic_t* characteristics;
	int characteristics_count;
	char uuid_str[MAX_LEN_UUID_STR + 1];

	if(argc < 2){
		usage(argv);
	}

	strcpy(gaddr, argv[1]);

	if (gattlib_string_to_uuid(WRITE_UUID, strlen(WRITE_UUID) + 1, &write_uuid) < 0) {
		printf("write uuid error.\n");
		return 1;
	}

	if (gattlib_string_to_uuid(READ_UUID, strlen(READ_UUID) + 1, &read_uuid) < 0) {
		printf("read uuid error.\n");
		return 1;
	}

	ret = gattlib_adapter_open(NULL, &adapter);
	if (ret) {
		fprintf(stderr, "ERROR: Failed to open adapter.\n");
		return 1;
	}


	ret = gattlib_adapter_scan_enable(adapter, ble_discovered_device, BLE_SCAN_TIMEOUT, NULL /* user_data */);
	if (ret) {
		fprintf(stderr, "ERROR: Failed to scan.\n");
		goto EXIT;
	}

	
	gattlib_adapter_scan_disable(adapter);

	if(gflag == 0){
		printf("ERROR: Failed to find %s\n", gaddr);
		return -1;
	}
	
	connection = gattlib_connect(adapter, argv[1], GATTLIB_CONNECTION_OPTIONS_LEGACY_DEFAULT);
	if (connection == NULL) {
		fprintf(stderr, "Fail to connect to the bluetooth device.\n");
		usage(argv);
		return 1;
	}

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

	if (g_operation == READ) {
		uint8_t *buffer = NULL;
		printf("Read UUID completed: ");
		fflush(stdout);
		while(1){
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
		}
		printf("\n");
	} else {
		ret = gattlib_write_char_by_uuid(connection, &write_uuid, &value_data, sizeof(value_data));
		if (ret != GATTLIB_SUCCESS) {
			char uuid_str[MAX_LEN_UUID_STR + 1];

			gattlib_uuid_to_string(&write_uuid, uuid_str, sizeof(uuid_str));

			if (ret == GATTLIB_NOT_FOUND) {
				fprintf(stderr, "Could not find GATT Characteristic with UUID %s. "
					"You might call the program with '--gatt-discovery'.\n", uuid_str);
			} else {
				fprintf(stderr, "Error while writing GATT Characteristic with UUID %s (ret:%d)\n",
					uuid_str, ret);
			}
			goto EXIT;
		}
	}
	
disconnect_exit:
	gattlib_disconnect(connection);

	
EXIT:

	gattlib_adapter_close(adapter);
	
	return ret;
}
