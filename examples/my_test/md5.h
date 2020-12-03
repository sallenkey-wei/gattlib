/* 
 * MD5 hash in C
 */

#ifndef _MD5_H_
#define _MD5_H_

#include <stdint.h>

void md5_hash(const uint8_t *message, uint32_t len, uint32_t hash[4]);
void md5_change_endin(uint32_t *buf, uint8_t len);
void MD5_Test(void);

#endif //_MD5_H_

