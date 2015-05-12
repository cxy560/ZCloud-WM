/***************************************************************************** 
* 
* File Name : wm_pwm.h 
* 
* Description: pwm module 
* 
* Copyright (c) 2014 Winner Micro Electronic Design Co., Ltd. 
* All rights reserved. 
* 
* Author : dave
* 
* Date : 2014-8-21 
*****************************************************************************/ 

#ifndef WM_PWM_H
#define WM_PWM_H

#include "wm_type_def.h"

#define PWM_CHANNEL_MAX_NUM	4
#define PWM_MAX_FREQ		400	//单路pwm最大频率，如果DEPTH调低，该频率可适当调高
#define PWM_DEPTH 250		//最大可调级别，可根据需要调整
#define PWM_1S 1000000
#define PWM_BASE_TIME_COUNT	(PWM_1S/PWM_MAX_FREQ/PWM_DEPTH)

typedef struct PWM_INFO{

	int freq;		//pwm 频率
	int duty;		//占空比	(1-255)
	int high_count;	//高电平计数器
	int low_count;	//低电平计数器
	int high_count_bk;	//计数器备份，避免再计算
	int low_count_bk;
}PWM_INFO_ST;

/**********************************************************************************************************
* Description: 	This function is used to initial pwm.
*
* Arguments  : 	channum		max pwm channel,range from 1 to 4
*				freq			is a pointer to frequency ,freq range from 1 to 400
*				duty			is a pointer to duty radio,duty range from 0 to 255
*
* Returns    : 		WM_SUCCESS		if initial success
* 				WM_FAILED		if initial failed
**********************************************************************************************************/
int tls_pwm_init(u16 channum,u16 *freq, u8 *duty);

/**********************************************************************************************************
* Description: 	This function is used to set duty radio.
*
* Arguments  : 	channel		pwm channel no,range form 0 to 3
*				duty			duty radio,range from 0 to 255
*
**********************************************************************************************************/
void tls_pwm_duty_set(u16 channel, u8 duty);

/**********************************************************************************************************
* Description: 	This function is used to set frequency.
*
* Arguments  : 	channel		pwm channel no,range form 0 to 3
*				freq			frequency,range from 1 to 400
*
**********************************************************************************************************/
void tls_pwm_freq_set(u16 channel, u16 freq);

/**********************************************************************************************************
* Description: 	This function is used to set frequency and duty radio at  the same time.
*
* Arguments  : 	channel		pwm channel no,range form 0 to 3
*				freq			frequency,range from 1 to 400
*				duty			duty radio,range from 0 to 255
**********************************************************************************************************/
void tls_pwm_freq_duty_set(u16 channel, u16 freq, u8 duty);

/**********************************************************************************************************
* Description: 	This function is used to stop pwm.
*
* Arguments  : 	
**********************************************************************************************************/
void tls_pwm_stop(void);

/**********************************************************************************************************
* Description: 	This function is used to set pwm flag,if flag is 1,cpu clk will switch up to 160MHZ.
*
* Arguments  : 	flag		0	off
						1 	on
**********************************************************************************************************/
void tls_set_pwm_flag(u8 flag);


/**********************************************************************************************************
* Description: 	This function is used to get pwm flag.
*
* Arguments  : 	
				
**********************************************************************************************************/
u8 tls_get_pwm_flag(void);


#endif /* WM_PWM_H */
