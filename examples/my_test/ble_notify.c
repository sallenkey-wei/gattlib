#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <glib.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>

#include "gattlib.h"
#include "generic.h"
#include "ble_notify.h"
#include "dealevt.h"

#define WAIT_TIME_SECONDS		(3)


extern void on_user_abort(int arg);

int put_data_to_notify_queue(generic_t * para, message_t * msg){
	struct msg_queue * send_msg = NULL;
	send_msg = calloc(1, sizeof(struct msg_queue));
	if(!send_msg)
		return -1;
	send_msg->msg = *msg;
	
	pthread_mutex_lock(&para->mutex);
	TAILQ_INSERT_HEAD(&para->notify_head, send_msg, entries);
	pthread_cond_broadcast(&para->notify_cond);
	pthread_mutex_unlock(&para->mutex);
}

int get_data_from_notify_queue(generic_t * para, message_t * msg){
	struct msg_queue *m;
	struct timespec   ts;
	struct timeval    tp;
	int ret = 0;

	if(!para || !msg){
		printf("<%s>, para or msg is null.\n", __FUNCTION__);
		return -1;
	}

	gettimeofday(&tp, NULL);

	ts.tv_sec  = tp.tv_sec;
	ts.tv_nsec = tp.tv_usec * 1000;
	ts.tv_sec += WAIT_TIME_SECONDS;

	pthread_mutex_lock(&para->mutex);

	ret = 0;
	while(TAILQ_EMPTY(&para->notify_head)/*队列为空*/ && ret == 0) {
		ret = pthread_cond_timedwait(&para->notify_cond, &para->mutex, &ts);
	}
	if(ret == 0){
		/* 从队尾取出数据 */
		m = TAILQ_LAST(&para->notify_head, msg_head);
		TAILQ_REMOVE(&para->notify_head, m, entries);
		*msg = m->msg;
		free(m);
	}
	else if(ret == ETIMEDOUT){
		printf("<%s>: recv data timeout.\n", __FUNCTION__);
	}
	pthread_mutex_unlock(&para->mutex);

	return ret;
}

static void notification_handler(const uuid_t* uuid, const uint8_t* data, size_t data_length, void* user_data) {
	int i;
	int ret = 0;
	generic_t * para = (generic_t *)user_data;
	/*

	printf("Notification Handler: ");
	
	for (i = 0; i < data_length; i++) {
		printf("%02x ", data[i]);
	}
	printf("\n");
	*/

	ret = notify_data_CP(data, data_length, &para->notify_msg);
	if(ret == 0){
		printf("Notification Handler: \n");
		for(i = 0; i < para->notify_msg.len; i++){
			printf("%02x ", para->notify_msg.buf[i]);
		}
		printf("\n");
		put_data_to_notify_queue(para, &para->notify_msg);
	}
}

static void *ble_notify_thread(void *arg) {
	generic_t * para = (generic_t *)arg;
	int ret = 0;
	gattlib_register_notification(para->connection, notification_handler, para);
	
	ret = gattlib_notification_start(para->connection, &para->notify_uuid);
	if (ret) {
		fprintf(stderr, "Fail to start notification.\n");
		pthread_exit(NULL);;
	}
	para->running = 1;
	// Catch CTRL-C
	signal(SIGINT, on_user_abort);

	para->m_main_loop = g_main_loop_new(NULL, 0);
	g_main_loop_run(para->m_main_loop);

	// In case we quit the main loop, clean the connection
	gattlib_notification_stop(para->connection, &para->notify_uuid);
	g_main_loop_unref(para->m_main_loop);
}


int ble_setup(generic_t * para){
	int ret = 0;
	para->connection = gattlib_connect(NULL, para->gaddr, GATTLIB_CONNECTION_OPTIONS_LEGACY_DEFAULT);
	if (para->connection == NULL) {
		fprintf(stderr, "Fail to connect to the bluetooth device %s.\n", para->gaddr);
		para->ble_setup_ret = -1;
		return para->ble_setup_ret;
	}
	printf("Connect to the bluetooth device %s successful.\n", para->gaddr);
	ret = pthread_create(&para->g_thread, NULL,	ble_notify_thread, para);
	if (ret != 0) {
		fprintf(stderr, "Failt to create BLE connection thread.\n");
		para->ble_setup_ret = -2;
		return para->ble_setup_ret;
	}
	para->ble_setup_ret = 0;
	return para->ble_setup_ret;
}

void ble_cleanup(generic_t * para){
	void * retval;

	switch(para->ble_setup_ret){
		case 0:
			pthread_join(para->g_thread, &retval);
		case -2:
			gattlib_disconnect(para->connection);
		case -1:
			para->running = 0;
			break;
		default:
			break;
	}
}
