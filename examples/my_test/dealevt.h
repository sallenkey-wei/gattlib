#ifndef __DEALEVT_H
#define __DEALEVT_H

#include "generic.h"

#define HI_UINT16(a) (((a) >> 8) & 0xFF)
#define LO_UINT16(a) ((a) & 0xFF)																	

#define BUILD_UINT16(loByte, hiByte)\
          ((uint16_t)(((loByte) & 0x00FF) + (((hiByte) & 0x00FF) << 8)))



typedef struct{
	uint8_t body_num;
	uint8_t body_encrypt;
	uint16_t body_len;
	uint8_t payload[256];
}body_t;

typedef struct{
	uint8_t header[2];
	uint8_t crc[2];
	uint8_t ver[2];
	uint16_t len;
	body_t body;
	uint8_t foot;
}Huijing_zigbee_frame_fuki_t;



int notify_data_CP(const uint8_t* data, size_t data_length, message_t *out_msg);
int deal_notify_event(generic_t *);
int huijing_format_change_pdu2uint(Huijing_zigbee_frame_fuki_t frame_fuki, uint8_t * data, uint16_t * len);
int huijing_format_change_uint2pdu(uint8_t *payload, Huijing_zigbee_frame_fuki_t *huijing_data);
void printf_huijing_frame(Huijing_zigbee_frame_fuki_t huijing_data);
void msg_merge_huijing_pdu_format(message_t msg, uint8_t *data, uint16_t *len);
int huijing_protocol_check(uint8_t * in, Huijing_zigbee_frame_fuki_t *p_frame_fuki);




#endif
