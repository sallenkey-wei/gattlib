#include <stdio.h>
#include <stdlib.h>

#include "dealevt.h"
#include "md5.h"

enum fetch_notify_data_stage{
	HEADER1_STG = 0,
	HEADER2_STG,
	CRC_VER_STG,
	LEN_STG,
	BODY_STG,
	FOOT_STG,	
};

#define HEADER1 		0xfe
#define HEADER2			0xfe

typedef struct read_notify_flag {
	unsigned int cur_count;
	unsigned int exp_count;
	unsigned char fetch_stage;
} read_notify_data_flag_t;

typedef struct vender_key_cb{
	char * vender_name;
	char * vender_keyB;
}vender_key_cb_t;

static uint8_t sequence = 0;
static int polynomial = 0x0000a001;

static vender_key_cb_t vender_key_cb[] = {
	{.vender_keyB = "RLkqKKM9Vx", .vender_name = "default vender",},
	{.vender_keyB = "yCvEj4QSfq", .vender_name = "BCL",},
	{.vender_keyB = "RDqMvyjw7W", .vender_name = "BitKey",},
	{.vender_keyB = "B6rP75IKVx", .vender_name = "ATsuMi",},
	{.vender_keyB = "cMz1bLEti3", .vender_name = "RobotHome",},
};


static read_notify_data_flag_t rnd_flag = {
	.cur_count = 0,
	.exp_count = 0,
	.fetch_stage = HEADER1_STG,
};

void select_vender_keyB(generic_t * para, vender_name_t vender_name){
	para->vender_keyB = vender_key_cb[vender_name].vender_keyB;
}

int notify_data_CP(const uint8_t* data, size_t data_length, message_t *out_msg){

	int i = 0;
	int body_len = 0;
	for(i = 0; i < data_length; i++){
		if(rnd_flag.fetch_stage == HEADER1_STG){
			if(data[i] == HEADER1){
				out_msg->buf[rnd_flag.cur_count] = data[i];
				rnd_flag.fetch_stage = HEADER2_STG;
				rnd_flag.cur_count++;
			}
			else{
				printf("HEADER1_STG: error header: %#x\n", data[i]);
			}
		}
		else if(rnd_flag.fetch_stage == HEADER2_STG){
			if(data[i] == HEADER2){				
				out_msg->buf[rnd_flag.cur_count] = data[i];
				rnd_flag.fetch_stage = CRC_VER_STG;
				rnd_flag.cur_count++;
				rnd_flag.exp_count = 4 + rnd_flag.cur_count;
				
			}
			else{
				rnd_flag.fetch_stage = HEADER1_STG;
				rnd_flag.cur_count = 0;
				rnd_flag.exp_count = 0;
				printf("HEADER2_STG: error header: %#x\n", data[i]);
			}
		}
		else if(rnd_flag.fetch_stage == CRC_VER_STG){
			
			out_msg->buf[rnd_flag.cur_count] = data[i];
			rnd_flag.cur_count++;
			if(rnd_flag.cur_count >= rnd_flag.exp_count){
				rnd_flag.fetch_stage = LEN_STG;
				rnd_flag.exp_count += 2;
			}
		}
		else if(rnd_flag.fetch_stage == LEN_STG){
			out_msg->buf[rnd_flag.cur_count] = data[i];
			rnd_flag.cur_count++;
			if(rnd_flag.cur_count >= rnd_flag.exp_count){
				rnd_flag.fetch_stage = BODY_STG;
				body_len = (out_msg->buf[rnd_flag.cur_count - 1] << 8) + out_msg->buf[rnd_flag.cur_count - 2];
				rnd_flag.exp_count += (body_len + 1);/* 1是footer长度 */
			}
		}
		else if(rnd_flag.fetch_stage == BODY_STG){
			out_msg->buf[rnd_flag.cur_count] = data[i];
			rnd_flag.cur_count++;
			if(rnd_flag.cur_count >= rnd_flag.exp_count){
				rnd_flag.fetch_stage = FOOT_STG;
			}
			
		}
		if(rnd_flag.fetch_stage == FOOT_STG){
			out_msg->len = rnd_flag.cur_count;
			rnd_flag.fetch_stage = HEADER1_STG;
			rnd_flag.cur_count = 0;
			rnd_flag.exp_count = 0;	
			return 0;
		}
	}
	return -1;
}


int handle_huijing_data(generic_t * para, Huijing_zigbee_frame_fuki_t * huijing_pdu, uint8_t * out, uint16_t * len)
{
	int ret = 0;
	uint16_t u16NewCmd = BUILD_UINT16(huijing_pdu->body.payload[0], huijing_pdu->body.payload[1]);
	message_t tmp_msg = {.len = 0, .buf = {0}};

	switch(u16NewCmd)
	{
		case BT_CMD_BUILD_LINK:
		{
			printf("-------收到锁发送密钥------\r\n");
			if(huijing_pdu->body.body_len >= 33){
				uint16_t vender_id;
				uint8_t vender_keyAB_CB[26];
			
				vender_id = BUILD_UINT16(huijing_pdu->body.payload[2], huijing_pdu->body.payload[3]);
				select_vender_keyB(para, vender_id);
				/* 厂商编号也是vender_keyA的一部分 */
				memcpy(para->vender_keyA, &huijing_pdu->body.payload[2], sizeof(para->vender_keyA));				
				memcpy(vender_keyAB_CB, para->vender_keyA, sizeof(para->vender_keyA));
				memcpy(vender_keyAB_CB + sizeof(para->vender_keyA), para->vender_keyB, strlen(para->vender_keyB));
				md5_hash(vender_keyAB_CB, sizeof(para->vender_keyA) + strlen(para->vender_keyB), para->security_key);
				u16NewCmd = BT_CMD_BUILD_LINK_BACK;
				memcpy(tmp_msg.buf, &u16NewCmd, 2);
				tmp_msg.len += 2;
				memcpy(tmp_msg.buf + 2, para->security_key, 16);
				tmp_msg.len += 16;
				msg_merge_huijing_pdu_format(tmp_msg, out, len, 0);
			}
			else{
				printf("body_len error\n");
				return -1;
			}
			break;
		}
		default:
			break;
	}
	return ret;
}

	
int deal_notify_event(generic_t * para, message_t * msg, uint8_t * out, uint16_t * len){
	Huijing_zigbee_frame_fuki_t huijing_pdu;
	int ret = 0;

	if(para == NULL){
		printf("<%s>: para is null.\n", __FUNCTION__);
		return -1;
	}

	ret = huijing_protocol_check(msg->buf, &huijing_pdu);
	if(ret)
	{
		printf("<%s>: huijing_protocol_check failed.\n", __FUNCTION__);
		return ret;
	}

	handle_huijing_data(para, &huijing_pdu, out, len);

	return 0;	
}




static uint16_t getCRC(uint8_t *bytes, int offset, int len) {
	    int CRC = 0x0000ffff;
	    int i, j;
	    for (i = 0; i < len; i++) {
	        CRC ^= ((int) bytes[offset + i] & 0x000000ff);
	        for (j = 0; j < 8; j++) {
	            if ((CRC & 0x00000001) != 0) {
	                CRC >>= 1;
	                CRC ^= polynomial;
	            } else {
	                CRC >>= 1;
	            }
	        }
	    }
	    
	    return (uint16_t) (CRC & 0xffff);
}


static uint16_t xor_crc(Huijing_zigbee_frame_fuki_t frame_fuki)
{
	int crc_part_len = frame_fuki.len + sizeof(frame_fuki.ver) + sizeof(frame_fuki.len) + sizeof(frame_fuki.foot);
	uint8_t * huijing_uint_stream = calloc(crc_part_len, 1);
	int offset = 0;
	memcpy(huijing_uint_stream+offset, frame_fuki.ver, sizeof(frame_fuki.ver));
	offset += sizeof(frame_fuki.ver);
	memcpy(huijing_uint_stream+offset, &frame_fuki.len, sizeof(frame_fuki.len));
	offset += sizeof(frame_fuki.len);
	memcpy(huijing_uint_stream+offset, &frame_fuki.body.body_num, sizeof(frame_fuki.body.body_num));
	offset += sizeof(frame_fuki.body.body_num);
	memcpy(huijing_uint_stream+offset, &frame_fuki.body.body_encrypt, sizeof(frame_fuki.body.body_encrypt));
	offset += sizeof(frame_fuki.body.body_encrypt);
	memcpy(huijing_uint_stream+offset, &frame_fuki.body.body_len, sizeof(frame_fuki.body.body_len));
	offset += sizeof(frame_fuki.body.body_len);
	memcpy(huijing_uint_stream+offset, frame_fuki.body.payload, frame_fuki.body.body_len);
	offset += frame_fuki.body.body_len;
	memcpy(huijing_uint_stream+offset, &frame_fuki.foot, sizeof(frame_fuki.foot));
	offset += sizeof(frame_fuki.foot);
	if(offset != crc_part_len){
		printf("<xor_crc> in Huijing, offset != crc_part_len\n");
		return -1;
	}
	uint16_t crc = getCRC(huijing_uint_stream, 0, crc_part_len);
	free(huijing_uint_stream);
	return crc;
}

int huijing_protocol_check(uint8_t * in, Huijing_zigbee_frame_fuki_t *p_frame_fuki)
{
	memset(p_frame_fuki, 0, sizeof(Huijing_zigbee_frame_fuki_t));
	huijing_format_change_uint2pdu(in, p_frame_fuki);
	if(BUILD_UINT16(p_frame_fuki->header[0], p_frame_fuki->header[1]) != 0xFEFE)
		return -1;
	if(BUILD_UINT16(p_frame_fuki->crc[0], p_frame_fuki->crc[1]) != xor_crc(*p_frame_fuki))
		return -1;
	return 0;
}


int huijing_format_change_uint2pdu(uint8_t *payload, Huijing_zigbee_frame_fuki_t *huijing_data)
{
	if(BUILD_UINT16(payload[0],payload[1]) == 0xfefe){
		memcpy(huijing_data->header,payload, 2);
		payload += 2;
		memcpy(huijing_data->crc,payload, 2);
		payload += 2;
		memcpy(huijing_data->ver,payload, 2);
		payload += 2;	
		huijing_data->len = BUILD_UINT16(payload[0], payload[1]);
		payload += 2;
		huijing_data->body.body_num = *payload++;
		huijing_data->body.body_encrypt = *payload++;
		huijing_data->body.body_len = BUILD_UINT16(payload[0], payload[1]);
		payload += 2;
		memcpy(huijing_data->body.payload, payload, huijing_data->body.body_len);
		payload +=huijing_data->body.body_len;
		huijing_data->foot = *payload;
	}

	return 0;
}


int huijing_format_change_pdu2uint(Huijing_zigbee_frame_fuki_t frame_fuki, uint8_t * data, uint16_t * len)
{
	int offset = 0;
	*len = sizeof(frame_fuki.header) + sizeof(frame_fuki.crc) + sizeof(frame_fuki.ver) + sizeof(frame_fuki.len) \
			+ frame_fuki.len + sizeof(frame_fuki.foot);
	memcpy(data+offset, frame_fuki.header, sizeof(frame_fuki.header));
	offset += sizeof(frame_fuki.header);
	memcpy(data+offset, frame_fuki.crc, sizeof(frame_fuki.crc));
	offset += sizeof(frame_fuki.crc);
	memcpy(data+offset, frame_fuki.ver, sizeof(frame_fuki.ver));
	offset += sizeof(frame_fuki.ver);
	memcpy(data+offset, &frame_fuki.len, sizeof(frame_fuki.len));
	offset += sizeof(frame_fuki.len);
	memcpy(data+offset, &frame_fuki.body.body_num, sizeof(frame_fuki.body.body_num));
	offset += sizeof(frame_fuki.body.body_num);
	memcpy(data+offset, &frame_fuki.body.body_encrypt, sizeof(frame_fuki.body.body_encrypt));
	offset += sizeof(frame_fuki.body.body_encrypt);
	memcpy(data+offset, &frame_fuki.body.body_len, sizeof(frame_fuki.body.body_len));
	offset += sizeof(frame_fuki.body.body_len);
	memcpy(data+offset, frame_fuki.body.payload, frame_fuki.body.body_len);
	offset += frame_fuki.body.body_len;
	memcpy(data+offset, &frame_fuki.foot, sizeof(frame_fuki.foot));
	offset += sizeof(frame_fuki.foot);
	if(*len != offset){
		printf("<%s> error\n", __FUNCTION__);
		return -1;
	}
	return 0;
}

void printf_huijing_frame(Huijing_zigbee_frame_fuki_t huijing_data)
{

		int i = 0;
		/*header打印*/
		printf("huijing_data.header :");
		for(i = 0; i < 2; i++){
			printf(" %02x",huijing_data.header[i]);
		}
		printf("\n");	
		/*crc打印*/
		printf("huijing_data.crc :");
		for(i = 0; i < 2; i++){
			printf(" %02x",huijing_data.crc[i]);
		}
		printf("\n");	
		/*ver打印*/		
		printf("huijing_data.ver :");
		for(i = 0; i < 2; i++){
			printf(" %02x",huijing_data.ver[i]);
		}
		printf("\n");	

		printf("huijing_data.len: %d \n", huijing_data.len);	
		printf("huijing_data.body.body_num : %02x \n", huijing_data.body.body_num);
		printf("huijing_data.body.body_encrypt : %02x \n", huijing_data.body.body_encrypt);
		printf("huijing_data.body.body_len : %d \n", huijing_data.body.body_len);

		/*payload打印*/					
		printf("huijing_data.body.payload :");
		for(i = 0; i< huijing_data.body.body_len; i++){
			printf(" %02x",huijing_data.body.payload[i]);
		}	
		printf("\n");		

		printf("huijing_data.foot :%02x \n",huijing_data.foot);
}

void msg_merge_huijing_pdu_format(message_t msg, uint8_t *data, uint16_t *len, uint8_t encrypt)
{
	Huijing_zigbee_frame_fuki_t frame_fuki;
	frame_fuki.header[0] = 0xFE;
	frame_fuki.header[1] = 0xFE;
	frame_fuki.ver[0] = 0x01;
	frame_fuki.ver[1] = 0x00;
	
	frame_fuki.body.body_num = sequence++;
	frame_fuki.body.body_encrypt = encrypt;
	if(encrypt){
		/* 加密 */
	}
	else{
		frame_fuki.body.body_len = msg.len;
		memcpy(frame_fuki.body.payload, msg.buf, msg.len);
	}
	frame_fuki.len = sizeof(frame_fuki.body.body_num)+ sizeof(frame_fuki.body.body_encrypt) \
					+ sizeof(frame_fuki.body.body_len) + frame_fuki.body.body_len;
	frame_fuki.foot = 0x3B;
	uint16_t crc = xor_crc(frame_fuki);
	memcpy(frame_fuki.crc, &crc, sizeof(crc));
	if(huijing_format_change_pdu2uint(frame_fuki, data, len)){
		printf("<%s> error\n", __FUNCTION__);
	}
	return ;
}



