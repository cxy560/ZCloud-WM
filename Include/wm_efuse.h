/***************************************************************************** 
* 
* File Name : wm_gpio.h
* 
* Description: GPIO Driver Module 
* 
* Copyright (c) 2014 Winner Microelectronics Co., Ltd. 
* All rights reserved. 
* 
* Author : dave 
* 
* Date : 2014-6-26
*****************************************************************************/ 
#ifndef WM_EFUSE_H
#define WM_EFUSE_H

#define TLS_EFUSE_STATUS_OK      (0)
#define TLS_EFUSE_STATUS_EINVALID      (1)
#define TLS_EFUSE_STATUS_EIO      (2)

#if TLS_CONFIG_VIRTUAL_EFUSE
#define TLS_EFUSE_START_ADDR      (0x200)
#else
#define TLS_EFUSE_START_ADDR      (0x00210000)
#endif

#define TLS_EFUSE_SPIFLASH_PARAM_OFFSET      (0x00)
#define TLS_EFUSE_SPIFLASH_PARAM_SIZE      (26)
#define TLS_EFUSE_TXGAIN_BG_OFFSET  (0x24)
#define TLS_EFUSE_TXGAIN_BG_SIZE  (12)

#define TLS_EFUSE_MACADDR_OFFSET      (0x30)
#define TLS_EFUSE_MACADDR_SIZE      (6)
#define TLS_EFUSE_LOLEAKAGE_OFFSET    (0x38)
#define TLS_EFUSE_LOLEAKAGE_SIZE      (2)
#define TLS_EFUSE_IQMISMATCH_OFFSET      (0x3a)
#define TLS_EFUSE_IQMISMATCH_SIZE      (2)
#define TLS_EFUSE_PLL_OFFSET      (0x3C)
#define TLS_EFUSE_PLL_SIZE      (1)

#define TLS_EFUSE_TXGAIN_MCS_OFFSET (0x3D)
#define TLS_EFUSE_TXGAIN_MCS_SIZE (17)


/**********************************************************************************************************
* Description: 	This function is used to get mac addr.
*
* Arguments  : 	mac		mac addr,6 byte
*
* Returns    : 	TLS_EFUSE_STATUS_OK			get success
* 			TLS_EFUSE_STATUS_EIO		get failed
**********************************************************************************************************/
int tls_get_mac_addr(u8 *mac);

/**********************************************************************************************************
* Description: 	This function is used to set mac addr.
*
* Arguments  : 	mac		mac addr,6 byte
*
* Returns    : 	TLS_EFUSE_STATUS_OK			get success
* 			TLS_EFUSE_STATUS_EIO		get failed
**********************************************************************************************************/
int tls_set_mac_addr(u8 *mac);

/**********************************************************************************************************
* Description: 	This function is used to get tx gain.
*
* Arguments  : 	txgain		tx gain,12 byte
*
* Returns    : 	TLS_EFUSE_STATUS_OK			get success
* 			TLS_EFUSE_STATUS_EIO		get failed
**********************************************************************************************************/
int tls_get_tx_gain(u8 *txgain);

/**********************************************************************************************************
* Description: 	This function is used to set tx gain.
*
* Arguments  : 	txgain		tx gain,12 byte
*
* Returns    : 	TLS_EFUSE_STATUS_OK			set success
* 			TLS_EFUSE_STATUS_EIO		set failed
**********************************************************************************************************/
int tls_set_tx_gain(u8 *txgain);

/**********************************************************************************************************
* Description: 	This function is used to write efuse value.
*
* Arguments  : 	offset:  offset value
*               data_buffer: data pointer
*               data_len:  len to write data
*
* Returns    : 	TLS_EFUSE_STATUS_OK			set success
* 			TLS_EFUSE_STATUS_EIO		set failed
**********************************************************************************************************/
int tls_efuse_write(u32 offset, u8 *data_buffer, u32 data_len);

/**********************************************************************************************************
* Description: 	This function is used to read efuse value.
*
* Arguments  : 	offset:  offset value
*               data_buffer: data pointer to save
*               data_len:  len to read data
*
* Returns    : 	TLS_EFUSE_STATUS_OK			get success
* 			TLS_EFUSE_STATUS_EIO		get failed
**********************************************************************************************************/
int tls_efuse_read(u32 offset, u8 *data_buffer, u32 data_len);
#endif /* WM_EFUSE_H */
