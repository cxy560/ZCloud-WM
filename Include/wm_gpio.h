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
* Date : 2014-6-4
*****************************************************************************/ 

#ifndef WM_GPIO_H
#define WM_GPIO_H

#include "wm_type_def.h"

#define TLS_GPIO_DIR_OUTPUT	0
#define TLS_GPIO_DIR_INPUT	1

#define TLS_GPIO_ATTR_FLOATING	0
#define TLS_GPIO_ATTR_PULLHIGH	1
#define TLS_GPIO_ATTR_PULLLOW	2

#define TLS_GPIO_INT_TRIG_RISING_EDGE	0
#define TLS_GPIO_INT_TRIG_FALLING_EDGE	1
#define TLS_GPIO_INT_TRIG_DOUBLE_EDGE	2
#define TLS_GPIO_INT_TRIG_HIGH_LEVEL	3
#define TLS_GPIO_INT_TRIG_LOW_LEVEL	4

#define GPIO_INTTERRUPT                   (25UL)



/**********************************************************************************************************
* Description: 	This function is used to config gpio function.
*
* Arguments  : 	gpio_pin		is gpio pin num,start from 0,end to 20.
* 		dir			is gpio dir,define as follow:
* 					dir == TLS_GPIO_DIR_OUTPUT	output
* 					dir == TLS_GPIO_DIR_INPUT	input
* 		attr			is gpio attribute,define as follow:
* 					attr == TLS_GPIO_ATTR_FLOATING	:floating status
* 					atrr == TLS_GPIO_ATTR_PULLHIGH	:pull high
* 					attr == TLS_GPIO_ATTR_PULLLOW	:pull low 			
*
* Returns    : 	null
* Note	     :	From gpio3 to gpio7,attribute can set to TLS_GPIO_ATTR_PULLHIGH,but can not set to TLS_GPIO_ATTR_PULLLOW,
* 		the default attribute is TLS_GPIO_ATTR_PULLHIGH.
* 		Other gpio can set to TLS_GPIO_ATTR_PULLLOW,but can not set to TLS_GPIO_ATTR_PULLHIGH,the deault
* 		attribute is TLS_GPIO_ATTR_PULLLOW.
* 		all gpio can set to TLS_GPIO_ATTR_FLOATING.
**********************************************************************************************************/
void tls_gpio_cfg(u16 gpio_pin, u16 dir, u16 attr);


/**********************************************************************************************************
* Description: 	This function is used to read gpio status.
*
* Arguments  : 	gpio_pin		is gpio pin num,start from 0,end to 20.
*
* Returns    : 	0			if power level is low
* 		1			if power level is high
**********************************************************************************************************/
u16 tls_gpio_read(u16 gpio_pin);

/**********************************************************************************************************
* Description: 	This function is used to modify gpio status.
*
* Arguments  : 	gpio_pin		is gpio pin num,start from 0,end to 20.
* 		value			is power level,define as follow:
* 					value == 0	:low power level
* 					value == 1	:high power level
*
* Returns    : 	
**********************************************************************************************************/
void tls_gpio_write(u16 gpio_pin, u16 value);


/**********************************************************************************************************
* Description: 	This function is used to config gpio interrupt.
*
* Arguments  : 	gpio_pin		is gpio pin num,start from 0,end to 20.
* 		mode			is interrupt trigger type,define as follow:
* 					mode == TLS_GPIO_INT_TRIG_RISING_EDGE	:rising edge arises the interrupt.
* 					mode == TLS_GPIO_INT_TRIG_FALLING_EDGE	:rising edge arises the interrupt.
* 					mode == TLS_GPIO_INT_TRIG_DOUBLE_EDGE	:both rising edge and falling edge arise the interrupt.
* 					mode == TLS_GPIO_INT_TRIG_HIGH_LEVEL	:high power level arises the interrupt.
* 					mode == TLS_GPIO_INT_TRIG_LOW_LEVEL	:low power level arises the interrupt.
*
* Returns    : 	
**********************************************************************************************************/
void tls_gpio_int_enable(u16 gpio_pin, u16 mode);

/**********************************************************************************************************
* Description: 	This function is used to disable gpio interrupt.
*
* Arguments  : 	gpio_pin		is gpio pin num,start from 0,end to 20.
*
* Returns    : 	
**********************************************************************************************************/
void tls_gpio_int_disable(u16 gpio_pin);

/**********************************************************************************************************
* Description: 	This function is used to get gpio interrupt status.
*
* Arguments  : 	gpio_pin		is gpio pin num,start from 0,end to 20.
*
* Returns    : 	return the interrupt status
* 		0			:no interrupt happened
* 		1			:interrupt happened
**********************************************************************************************************/
int tls_get_gpio_int_flag(u16 gpio_pin);

/**********************************************************************************************************
* Description: 	This function is used to clear gpio interrupt flag.
*
* Arguments  : 	gpio_pin		is gpio pin num,start from 0,end to 20.
*
* Returns    : 	
**********************************************************************************************************/
void tls_clr_gpio_int_flag(u16 gpio_pin);


/**********************************************************************************************************
* Description: 	This function is used to get gpio interrupt if is enabled.
*
* Arguments  : 	gpio_pin		is gpio pin num,start from 0,end to 20.
*
* Returns    : 	
**********************************************************************************************************/
int tls_gpio_int_if_enable(int gpio_pin);


/**********************************************************************************************************
* Description: 	This function is used to register gpio interrupt.
*
* Arguments  : 	callback		is the gpio interrupt call back function.
* 		context			is the callback function argument.
*
* Returns    : 
* Note	     :	The user can register several gpio interrupt,but all the gpio interrupt must use one call back function.
**********************************************************************************************************/
void tls_gpio_isr_register(void (*callback)(void *context), void *context);

#endif /* end of WM_GPIO_H */
