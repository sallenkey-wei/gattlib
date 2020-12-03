#ifndef __BLE_NOTIFY_H
#define __BLE_NOTIFY_H

int ble_setup(generic_t * para);
void ble_cleanup(generic_t * para);
int put_data_to_notify_queue(generic_t * para, message_t * msg);
int get_data_from_notify_queue(generic_t * para, message_t * msg);


#endif
