/**
*  $Id: md5.c,v 1.2 2008/03/24 20:59:12 mascarenhas Exp $
*  Hash function MD5
*  @author  Marcela Ozorio Suarez, Roberto I.
*/


#include <string.h>
/* 
 * MD5 hash in C
 */

#include "md5.h"


void md5_compress(uint32_t state[4], const uint32_t block[16]) {
	#define ROUND0(a, b, c, d, k, s, t)  ROUND_TAIL(a, b, d ^ (b & (c ^ d)), k, s, t)
	#define ROUND1(a, b, c, d, k, s, t)  ROUND_TAIL(a, b, c ^ (d & (b ^ c)), k, s, t)
	#define ROUND2(a, b, c, d, k, s, t)  ROUND_TAIL(a, b, b ^ c ^ d        , k, s, t)
	#define ROUND3(a, b, c, d, k, s, t)  ROUND_TAIL(a, b, c ^ (b | ~d)     , k, s, t)
	#define ROUND_TAIL(a, b, expr, k, s, t)    \
		a += (expr) + UINT32_C(t) + block[k];  \
		a = b + (a << s | a >> (32 - s));
	
	uint32_t a = state[0];
	uint32_t b = state[1];
	uint32_t c = state[2];
	uint32_t d = state[3];
	
	ROUND0(a, b, c, d,  0,  7, 0xD76AA478)
	ROUND0(d, a, b, c,  1, 12, 0xE8C7B756)
	ROUND0(c, d, a, b,  2, 17, 0x242070DB)
	ROUND0(b, c, d, a,  3, 22, 0xC1BDCEEE)
	ROUND0(a, b, c, d,  4,  7, 0xF57C0FAF)
	ROUND0(d, a, b, c,  5, 12, 0x4787C62A)
	ROUND0(c, d, a, b,  6, 17, 0xA8304613)
	ROUND0(b, c, d, a,  7, 22, 0xFD469501)
	ROUND0(a, b, c, d,  8,  7, 0x698098D8)
	ROUND0(d, a, b, c,  9, 12, 0x8B44F7AF)
	ROUND0(c, d, a, b, 10, 17, 0xFFFF5BB1)
	ROUND0(b, c, d, a, 11, 22, 0x895CD7BE)
	ROUND0(a, b, c, d, 12,  7, 0x6B901122)
	ROUND0(d, a, b, c, 13, 12, 0xFD987193)
	ROUND0(c, d, a, b, 14, 17, 0xA679438E)
	ROUND0(b, c, d, a, 15, 22, 0x49B40821)
	ROUND1(a, b, c, d,  1,  5, 0xF61E2562)
	ROUND1(d, a, b, c,  6,  9, 0xC040B340)
	ROUND1(c, d, a, b, 11, 14, 0x265E5A51)
	ROUND1(b, c, d, a,  0, 20, 0xE9B6C7AA)
	ROUND1(a, b, c, d,  5,  5, 0xD62F105D)
	ROUND1(d, a, b, c, 10,  9, 0x02441453)
	ROUND1(c, d, a, b, 15, 14, 0xD8A1E681)
	ROUND1(b, c, d, a,  4, 20, 0xE7D3FBC8)
	ROUND1(a, b, c, d,  9,  5, 0x21E1CDE6)
	ROUND1(d, a, b, c, 14,  9, 0xC33707D6)
	ROUND1(c, d, a, b,  3, 14, 0xF4D50D87)
	ROUND1(b, c, d, a,  8, 20, 0x455A14ED)
	ROUND1(a, b, c, d, 13,  5, 0xA9E3E905)
	ROUND1(d, a, b, c,  2,  9, 0xFCEFA3F8)
	ROUND1(c, d, a, b,  7, 14, 0x676F02D9)
	ROUND1(b, c, d, a, 12, 20, 0x8D2A4C8A)
	ROUND2(a, b, c, d,  5,  4, 0xFFFA3942)
	ROUND2(d, a, b, c,  8, 11, 0x8771F681)
	ROUND2(c, d, a, b, 11, 16, 0x6D9D6122)
	ROUND2(b, c, d, a, 14, 23, 0xFDE5380C)
	ROUND2(a, b, c, d,  1,  4, 0xA4BEEA44)
	ROUND2(d, a, b, c,  4, 11, 0x4BDECFA9)
	ROUND2(c, d, a, b,  7, 16, 0xF6BB4B60)
	ROUND2(b, c, d, a, 10, 23, 0xBEBFBC70)
	ROUND2(a, b, c, d, 13,  4, 0x289B7EC6)
	ROUND2(d, a, b, c,  0, 11, 0xEAA127FA)
	ROUND2(c, d, a, b,  3, 16, 0xD4EF3085)
	ROUND2(b, c, d, a,  6, 23, 0x04881D05)
	ROUND2(a, b, c, d,  9,  4, 0xD9D4D039)
	ROUND2(d, a, b, c, 12, 11, 0xE6DB99E5)
	ROUND2(c, d, a, b, 15, 16, 0x1FA27CF8)
	ROUND2(b, c, d, a,  2, 23, 0xC4AC5665)
	ROUND3(a, b, c, d,  0,  6, 0xF4292244)
	ROUND3(d, a, b, c,  7, 10, 0x432AFF97)
	ROUND3(c, d, a, b, 14, 15, 0xAB9423A7)
	ROUND3(b, c, d, a,  5, 21, 0xFC93A039)
	ROUND3(a, b, c, d, 12,  6, 0x655B59C3)
	ROUND3(d, a, b, c,  3, 10, 0x8F0CCC92)
	ROUND3(c, d, a, b, 10, 15, 0xFFEFF47D)
	ROUND3(b, c, d, a,  1, 21, 0x85845DD1)
	ROUND3(a, b, c, d,  8,  6, 0x6FA87E4F)
	ROUND3(d, a, b, c, 15, 10, 0xFE2CE6E0)
	ROUND3(c, d, a, b,  6, 15, 0xA3014314)
	ROUND3(b, c, d, a, 13, 21, 0x4E0811A1)
	ROUND3(a, b, c, d,  4,  6, 0xF7537E82)
	ROUND3(d, a, b, c, 11, 10, 0xBD3AF235)
	ROUND3(c, d, a, b,  2, 15, 0x2AD7D2BB)
	ROUND3(b, c, d, a,  9, 21, 0xEB86D391)
	
	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;
}



void md5_change_endin(uint32_t *buf, uint8_t len)
{
    uint8_t temp[4];
    
    uint8_t t;
    
    for(uint8_t i = 0; i < len; i++)
    {
        memcpy(temp, &buf[i], 4);
        t = temp[0];
        temp[0] = temp[3];
        temp[3] = t;
        
        t = temp[1];
        temp[1] = temp[2];
        temp[2] = t;
        
        memcpy(&buf[i], temp, 4);
    }
}




void md5_hash(const uint8_t *message, uint32_t len, uint32_t hash[4]) 
 {
	hash[0] = (uint32_t)(0x67452301);
	hash[1] = (uint32_t)(0xEFCDAB89);
	hash[2] = (uint32_t)(0x98BADCFE);
	hash[3] = (uint32_t)(0x10325476);
	
	uint32_t i;
	for (i = 0; len - i >= 64; i += 64)
		md5_compress(hash, (uint32_t *)(message + i));  // Type-punning
	
	uint32_t block[16];
	uint8_t *byteBlock = (uint8_t *)block;  // Type-punning
	
	uint32_t rem = len - i;
	memcpy(byteBlock, message + i, rem);
	
	byteBlock[rem] = 0x80;
	rem++;
	if (64 - rem >= 8)
		memset(byteBlock + rem, 0, 56 - rem);
	else {
		memset(byteBlock + rem, 0, 64 - rem);
		md5_compress(hash, block);
		memset(block, 0, 56);
	}
	block[14] = len << 3;
	block[15] = len >> 29;
	md5_compress(hash, block);
}



void MD5_Test(void)
{
#if 0
    uint8_t temp[40];
    const uint8_t device_addr[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    const uint8_t device_model[4] = {0x56, 0x78, 0x45, 0x88};
    const uint8_t random_key[8] = {0x89, 0x77, 0x12, 0x37, 0x78, 0x11, 0x01, 0x94};
    const uint8_t model_key[20] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};

    uint8_t sec_key[16];
    uint8_t device_key[16];
        
    memcpy(temp, device_addr, sizeof(device_addr));
    memcpy(temp+sizeof(device_addr), model_key, sizeof(model_key));
    
    md5_hash(temp, sizeof(device_addr)+sizeof(model_key), (uint32_t *)device_key);
   
    memset(temp, 0x00, sizeof(temp));
    memcpy(temp, random_key, sizeof(random_key));
    memcpy(temp+sizeof(random_key), device_key, sizeof(device_key));
    md5_hash(temp, sizeof(device_key)+sizeof(random_key), (uint32_t *)sec_key);
        
    uint8_t out[16];
    md5_hash("a", 1, (uint32_t *)out);
    md5_change_endin((uint32_t *)out, 4);
        
    uint8_t temp1[16];
    memcpy(temp1, (uint8_t *)out, 16);
#else
    uint8_t output[16];
    uint8_t input[60];
    input[0] = 0x01;
    input[1] = 0x23;
    input[2] = 0x45;
    input[3] = 0x67;
    input[4] = 0x89;
    input[5] = 0xAB;
    input[6] = 0xCD;
    input[7] = 0xEF;
    
    strncpy((char*)&input[8], "ctjLtIH8pHkbRhg93D87", 20);
    md5_hash(input, 28, (uint32_t *)output);
    
    uint8_t input1[16];
    uint8_t out[16] = {0};
    input1[0] = 0x31;
    input1[1] = 0x32;
    input1[2] = 0x33;
    input1[3] = 0x34;
    input1[4] = 0x35;
    input1[5] = 0x36;
    
    //out {0x44, 0x73, 0x26, 0xc0, 0x9d, 0x66, 0xce, 0x49, 0xf6, 0x13, 0x60, 0xf0, 0xbe, 0x78, 0x31, 0x17}
    md5_hash(input1, 6, (uint32_t *)out);
    
    strncpy((char*)&input[0], "C8:93:46:91:8F:85", 17);
    strncpy((char*)&input[17], "ctjLtIH8pHkbRhg93D87", 20);
    md5_hash(input, 37, (uint32_t *)output);
#endif  
    
    
    while(1);
}

