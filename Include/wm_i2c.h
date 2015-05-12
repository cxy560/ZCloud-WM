/***************************************************************************** 
* 
* File Name : wm_i2c.h 
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

#ifndef WM_I2C_H
#define WM_I2C_H

#include "wm_type_def.h"

/**********************************************************************************************************
* Description: 	This function is used to initial i2c interface.
*
* Arguments  : 	
*
* Returns    : 
* 		
**********************************************************************************************************/
void tls_i2c_init(void);

/**********************************************************************************************************
* Description: 	This function is used to stop i2c.
*
* Arguments  : 	
*
* Returns    : 
* 		
**********************************************************************************************************/
void tls_i2c_stop(void);

/**********************************************************************************************************
* Description: 	This function is used to wait ack signal.
*
* Arguments  : 	
*
* Returns    : 
* 		
**********************************************************************************************************/
int tls_i2c_wait_ack(void);

/**********************************************************************************************************
* Description: 	This function is used to write one byte to i2c slave.
*
* Arguments  : 	data		:	the data want to transfer
*				ifstart	:	if transfer start signal to the slave
*
* Returns    : 
* 		
**********************************************************************************************************/
void tls_i2c_write_byte(u8 data,u8 ifstart);


/**********************************************************************************************************
* Description: 	This function is used to read one byte from i2c slave.
*
* Arguments  : 	ifack		:	if transfer ack signal to the slave
*				ifstop	:	if transfer stop signal to the slave
*
* Returns    : 		the data from the slave.
* 		
**********************************************************************************************************/
u8 tls_i2c_read_byte(u8 ifack,u8 ifstop);

#endif
