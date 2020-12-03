#ifndef _AES_TINY_H_
#define _AES_TINY_H_
#include <stdint.h>
    


// #define the macros below to 1/0 to enable/disable the mode of operation.
//
// ECB enables the basic ECB 16-byte block algorithm. Both can be enabled simultaneously.

void AES128_ECB_encrypt(uint8_t* input, const uint8_t* key, uint8_t *output);
void AES128_ECB_decrypt(uint8_t* input, const uint8_t* key, uint8_t *output);

void AES_Test(void);

#define MODE_NOPADDING		0x00
#define MODE_PKCS7_PADDING	0x01
//return : bytes encrypted in out buff
uint32_t AES128_EncryptECB
(
uint8_t bMode,
uint8_t* pbInput,
uint32_t uInputLen,
uint8_t *pbOut,
uint32_t *puOutBufLen,	//in: out buff len,out: bytes return
const uint8_t* pbKey
);
//return : bytes of plain text with padding-removed 
uint32_t AES128_DecryptECB
(
uint8_t bMode,
uint8_t* pbInput,
uint32_t uInputLen,
uint8_t *pbOut,
uint32_t *puOutBufLen,	//in: out buff len,out: bytes return
const uint8_t* pbKey
);
#endif //_AES_H_
