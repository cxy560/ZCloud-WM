/***************************************************************************** 
* 
* File Name : wm_i2c.c 
* 
* Description: i2c Driver Module 
* 
* Copyright (c) 2014 Winner Microelectronics Co., Ltd. 
* All rights reserved. 
* 
* Author : dave
* 
* Date : 2014-8-13
*****************************************************************************/ 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "wm_regs.h"
#include "wm_i2c.h"

void tls_i2c_init(void)
{
	u32	Value = 0;

	/* pin config */
	Value = tls_reg_read32(HR_IOCTL_GP_SDIO_I2C);
	Value &= 0x0fff;
	tls_reg_write32(HR_IOCTL_GP_SDIO_I2C, Value);

	/* set i2c clk division, sysclk:40Mhz, rate: 400kbps, i2c clk div: 19 */
	tls_reg_write32(HR_I2C_PRER_LO, 0x20);
	tls_reg_write32(HR_I2C_PRER_HI, 0);

	/* disable internal poll-down */
	Value = tls_reg_read32(HR_IOCTL_PE);
	Value &= ~0x0300;
	tls_reg_write32(HR_IOCTL_PE, Value);

	/* enable I2C | Disable Int*/
	tls_reg_write32(HR_I2C_CTRL, I2C_CTRL_INT_DISABLE | I2C_CTRL_ENABLE);

}


void tls_i2c_stop(void)
{
	tls_reg_write32(HR_I2C_CR_SR, I2C_CR_STO);
	while(tls_reg_read32(HR_I2C_CR_SR) & I2C_SR_TIP);
}

int tls_i2c_wait_ack(void)
{
	u16 errtime=0;
	u32 value;

	while(tls_reg_read32(HR_I2C_CR_SR) & I2C_SR_TIP);
	value = tls_reg_read32(HR_I2C_CR_SR);
	while(value & I2C_SR_NAK)
	{
		errtime ++;
		if(errtime > 512)
		{
			printf("wait ack err\n");
			tls_i2c_stop();
			return WM_FAILED;
		}
		value = tls_reg_read32(HR_I2C_CR_SR);
	}

	return WM_SUCCESS;
}


//ifstart : 是否发送start信号
void tls_i2c_write_byte(u8 data,u8 ifstart)
{
	tls_reg_write32(HR_I2C_TX_RX, data);
	if(ifstart)
		tls_reg_write32(HR_I2C_CR_SR, I2C_CR_STA | I2C_CR_WR);
	else
		tls_reg_write32(HR_I2C_CR_SR, I2C_CR_WR);
	while(tls_reg_read32(HR_I2C_CR_SR) & I2C_SR_TIP);
}

//ifack =1 ,发送ack，=0，发送nack
//ifstop = 1;读取之后发送停止信号，0 读之后不发送停止信号
u8 tls_i2c_read_byte(u8 ifack,u8 ifstop)
{
	u8 data;
	u32 value = I2C_CR_RD;

	if(!ifack)
		value |= I2C_CR_NAK;
	if(ifstop)
		value |= I2C_CR_STO;
	
	tls_reg_write32(HR_I2C_CR_SR, value);
	/* Waiting finish */
	while(tls_reg_read32(HR_I2C_CR_SR) & I2C_SR_TIP);
	data = tls_reg_read32(HR_I2C_TX_RX);

	return data;
}

