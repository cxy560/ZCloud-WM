/**************************************************************************
 * File Name                   : tls_efuse.c
 * Author                       :
 * Version                      :
 * Date                          :
 * Description                 :
 *
 * Copyright (c) 2014 Winner Microelectronics Co., Ltd. 
 * All rights reserved.
 *
 ***************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "wm_debug.h"
#include "wm_regs.h"
#include "wm_efuse.h"
#include "wm_config.h"
#include "list.h"
#include "wm_flash.h"

#if TLS_CONFIG_VIRTUAL_EFUSE

int tls_efuse_read(u32 offset, u8 *data_buffer, u32 data_len)
{
	int err;
	
	err = tls_fls_read(TLS_EFUSE_START_ADDR + offset, data_buffer, data_len);

	if (err != TLS_FLS_STATUS_OK) {
		err = TLS_EFUSE_STATUS_EIO;
	} else {
		err = TLS_EFUSE_STATUS_OK;
	}

	return err;

}

int tls_efuse_write(u32 offset, u8 *data_buffer, u32 data_len)
{
	int err;

	err = tls_fls_write(TLS_EFUSE_START_ADDR + offset, data_buffer, data_len);
	if (err != TLS_FLS_STATUS_OK) {
		err = TLS_EFUSE_STATUS_EIO;
	} else {
		err = TLS_EFUSE_STATUS_OK;
	}

	return err;
}

#else

#define EFUSE_WORDSIZE_SINGLE      (8)
#define EFUSE_BYTESIZE_SINGLE      (EFUSE_WORDSIZE_SINGLE*4)
#define EFUSE_BYTESIZE_TOTAL      (EFUSE_BYTESIZE_SINGLE*2)

static u32 efuse_buffer[EFUSE_WORDSIZE_SINGLE];

static void _efuse_program(u8 select_idx)
{
	u32 value = 0;
	u32 cnt = 0;

	if ((select_idx != 1) && (select_idx != 2)) {return;}

	/* Enable program VDD */
	//tls_gpio_config_pin(GPIO_EFUSE_PRG, GPIO_OUTPUT); 
	//tls_gpio_setbit(GPIO_EFUSE_PRG);

	/* wait VDDQ_SETUP time */
	for (cnt = 0; cnt < 0x200; cnt++);

	/* Soft Reset */
	tls_reg_write32(HR_EFUSE_CLEAR, EFUSE_RESET);

	/* Write first word */
	cnt = 0;
	tls_reg_write32(HR_EFUSE_WDATA_PORT, efuse_buffer[cnt++]);

	/* Start program */
	tls_reg_write32(HR_EFUSE_PGM_CMD, (select_idx << 1) | 0x01);

	do {
		value = tls_reg_read32(HR_EFUSE_PGM_STTS);
		if (value & EFUSE_PGM_DONE) {break;}

		if ((cnt < EFUSE_WORDSIZE_SINGLE) && (value & EFUSE_FIFO_FREE)) {
			tls_reg_write32(HR_EFUSE_WDATA_PORT, efuse_buffer[cnt++]);
		}

	} while(1);

	/* Disable program VDD */
	//tls_gpio_clrbit(GPIO_EFUSE_PRG);
	//tls_gpio_config_pin(GPIO_EFUSE_PRG, GPIO_INPUT); 
}

int tls_efuse_read(u32 offset, u8 *data_buffer, u32 data_len)
{
	MEMCPY(data_buffer, (INT8U *)(TLS_EFUSE_START_ADDR + offset), data_len);
}

int tls_efuse_write(u32 offset, u8 *data_buffer, u32 data_len)
{
	u32 efuse_idx;
	u32 len;
	u8 *temp = (u8 *)efuse_buffer;

	if ((data_len <= 0)||(offset >= EFUSE_BYTESIZE_TOTAL) || ((offset+data_len) > EFUSE_BYTESIZE_TOTAL)){
		return TLS_EFUSE_STATUS_EINVALID;
	}

	do{
		efuse_idx = 1;

		if (offset >= EFUSE_BYTESIZE_SINGLE)	{
			efuse_idx = 2;
			offset -= EFUSE_BYTESIZE_SINGLE;
		}

		len = EFUSE_BYTESIZE_SINGLE - offset;

		if (data_len < len){
			len = data_len;
		}

		memset(temp, 0, EFUSE_BYTESIZE_SINGLE);	
		MEMCPY(temp + offset, data_buffer, len);

		_efuse_program(efuse_idx);

		offset += len;
		data_buffer += len;
		data_len -= len;
	}while(data_len > 0);

	return TLS_EFUSE_STATUS_OK; 
}
#endif


/**********************************************************************************************************
* Description: 	This function is used to get mac addr.
*
* Arguments  : 	mac		mac addr,6 byte
*
* Returns    : 	TLS_EFUSE_STATUS_OK			get success
* 			TLS_EFUSE_STATUS_EIO		get failed
**********************************************************************************************************/
int tls_get_mac_addr(u8 *mac)
{
	return tls_efuse_read(TLS_EFUSE_MACADDR_OFFSET, mac, 6);
}

/**********************************************************************************************************
* Description: 	This function is used to set mac addr.
*
* Arguments  : 	mac		mac addr,6 byte
*
* Returns    : 	TLS_EFUSE_STATUS_OK			get success
* 			TLS_EFUSE_STATUS_EIO		get failed
**********************************************************************************************************/
int tls_set_mac_addr(u8 *mac)
{
	return tls_efuse_write(TLS_EFUSE_MACADDR_OFFSET, mac, 6);
}

/**********************************************************************************************************
* Description: 	This function is used to get tx gain.
*
* Arguments  : 	txgain		tx gain,12 byte
*
* Returns    : 	TLS_EFUSE_STATUS_OK			get success
* 			TLS_EFUSE_STATUS_EIO		get failed
**********************************************************************************************************/
int tls_get_tx_gain(u8 *txgain)
{
	int ret; 
	ret = tls_efuse_read(TLS_EFUSE_TXGAIN_BG_OFFSET, txgain, 12);
	if(ret != TLS_EFUSE_STATUS_OK)
		return ret;
#ifdef TLS_CONFIG_11N
	ret = tls_efuse_read(TLS_EFUSE_TXGAIN_MCS_OFFSET, txgain+12, 17);
#endif
	return ret;
}

/**********************************************************************************************************
* Description: 	This function is used to set tx gain.
*
* Arguments  : 	txgain		tx gain,12 byte
*
* Returns    : 	TLS_EFUSE_STATUS_OK			set success
* 			TLS_EFUSE_STATUS_EIO		set failed
**********************************************************************************************************/
int tls_set_tx_gain(u8 *txgain)
{
	return tls_efuse_write(TLS_EFUSE_TXGAIN_BG_OFFSET, txgain, 12);
}

