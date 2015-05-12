#ifndef WM_CRYPTO_H
#define WM_CRYPTO_H

/*************
	AES 
**************/

/**********************************************************************************************************
* Description: 	Encrypt plain data by 128 AES crypto
*
* Arguments  : 	key :  the key for encryption;
*                          iv    :  the IV value for encryption;			
*                          data:  where the plain data stored;
*                          data_len: length of the plain data to be encrypted
* Returns    : 
*                          0:  finish Encryption successfully;
*                        -1:  Error;
* Note: Encrypted data will be placed into the plain @data area                 
**********************************************************************************************************/
int  aes_128_cbc_encrypt(const u8 *key, const u8 *iv, u8 *data,
						 size_t data_len);


/**********************************************************************************************************
* Description: 	Decrypt data by 128 AES crypto
*
* Arguments  : 	key :  the key for encryption;
*                          iv    :  the IV value for encryption;			
*                          data:  where the encrypted data stored;
*                          data_len: length of the encrypted data to be decrypted;
* Returns    : 
*                          0:  finish Decryption successfully;
*                        -1:  Error;
* Note: plain data will be placed into the encrypted @data area                 
**********************************************************************************************************/

int  aes_128_cbc_decrypt(const u8 *key, const u8 *iv, u8 *data,
						 size_t data_len);



/*************
	RC4 
**************/

/**********************************************************************************************************
* Description: 	XOR RC4 stream to given data with skip-stream-start
*
* Arguments  : 	key :  RC4 key;
*                          keylen: RC4 key length;			
*                          data: data to be XOR'ed with RC4 stream;
*                          data_len: length of the plain data to be encrypted;
* Returns    : 
*                          0:  finish Encryption/Decryption successfully;
*                        -1:  Error;
* Note: this function should be used for Encryption & Decryption both. For the Encryption, the plain @data 
*         will be replaced by the encrypted output, and vice versa;
*        
**********************************************************************************************************/
int rc4(const u8 *key, size_t keylen, u8 *data, size_t data_len);


/*************
	MD5 
**************/
/**********************************************************************************************************
* Description:	MD5 hash for data vector
*
* Arguments  :	addr: Pointers to the data area;
*				len: Lengths of the data block;			
*				mac: Buffer for the hash (16 bytes)
* Returns	 : 
*				0:  finish caculation successfully;
*			     -1:  Error;
* Note: 
**********************************************************************************************************/
int md5(const u8 *addr, int len, u8 *mac);

/**********************************************************************************************************
* Description: 	HMAC-MD5 over data buffer (RFC 2104)
*
* Arguments  : 	key :  Key for HMAC operations;
*                          keylen: Length of the key in bytes;			
*                          data: data to be caculated;
*                          data_len: length of the plain data to be caculated;
*                          mac: Buffer for the hash (16 bytes)
* Returns    : 
*                          0:  finish caculation successfully;
*                        -1:  Error;
* Note: 
**********************************************************************************************************/

int hmac_md5(const u8 *key, size_t key_len, const u8 *data, size_t data_len, u8 *mac);


/*************
	SHA1 
**************/
/**********************************************************************************************************
* Description:	SHA1 hash for data vector
*
* Arguments  :	addr: Pointers to the data area;
*				len: Lengths of the data block;			
*				mac: Buffer for the hash (20 bytes)
* Returns	 : 
*				0:  finish caculation successfully;
*				-1:  Error;
* Note: 			
**********************************************************************************************************/
int sha1(const u8 *addr, int len, u8 *mac);

#endif

