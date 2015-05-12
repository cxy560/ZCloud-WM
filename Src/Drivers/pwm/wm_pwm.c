/***************************************************************************** 
* 
* File Name : wm_pwm.c 
* 
* Description: pwm Module 
* 
* Copyright (c) 2014 Winner Microelectronics Co., Ltd. 
* All rights reserved. 
* 
* Author : dave
* 
* Date : 2014-8-20 
*****************************************************************************/

#include "wm_debug.h"
#include "wm_regs.h"
#include "wm_irq.h"
#include "wm_pwm.h"
#include "wm_gpio.h"
#include "wm_timer.h"
#include "wm_cpu.h"


u8 pwm_io[PWM_CHANNEL_MAX_NUM] = {11, 12, 13,0};
u8 pwm_num = 0;
PWM_INFO_ST pwm_info[4];
u8 pwm_on = 0;

static void cal_count(int channel)
{
	int high,low;
	
	high =  (PWM_1S/pwm_info[channel].freq)*pwm_info[channel].duty/PWM_DEPTH/(PWM_BASE_TIME_COUNT);
	low =  ((PWM_1S/pwm_info[channel].freq/(PWM_BASE_TIME_COUNT)) - high);
	if(0 == high || 0 == low )	//如果有一个为零，需要把计数器取反
	{
		pwm_info[channel].high_count_bk = low;
		pwm_info[channel].low_count_bk = high;

	}
	else
	{
		pwm_info[channel].high_count_bk = high;
		pwm_info[channel].low_count_bk = low;
	}	
}

void pwm_timer_cb(void)
{
	int i = 0;
	int value;
	value = tls_reg_read32(HR_TIMER1_CSR);
	value |= 1<<4;
	tls_reg_write32(HR_TIMER1_CSR, value);
	for(i = 0;i < pwm_num;i ++)
	{
		if(pwm_info[i].high_count > 0)
		{
			pwm_info[i].high_count --;
			if(0 == pwm_info[i].high_count)
			{
				tls_gpio_write(pwm_io[i], 0);
			}
		}
		else if(pwm_info[i].low_count > 0)
		{
			pwm_info[i].low_count --;
			if(0 == pwm_info[i].low_count)
			{
				tls_gpio_write(pwm_io[i], 1);
			}			
		}
		if(0 == pwm_info[i].high_count && 0 == pwm_info[i].low_count)
		{
			pwm_info[i].high_count = pwm_info[i].high_count_bk;
			pwm_info[i].low_count = pwm_info[i].low_count_bk;
			if(0 == pwm_info[i].high_count_bk)
				tls_gpio_write(pwm_io[i], 1);
			else if(0 == pwm_info[i].low_count_bk)
				tls_gpio_write(pwm_io[i], 0);			
		}
	}
}

void tls_pwm_duty_set(u16 channel, u8 duty)
{
	if(channel > (PWM_CHANNEL_MAX_NUM - 1))
	{
		TLS_DBGPRT_ERR("duty param err\n");
		return;
	}

	if(duty >= PWM_DEPTH)
		pwm_info[channel].duty = PWM_DEPTH;
	else if(duty < 1)
		pwm_info[channel].duty = 0;
	else
		pwm_info[channel].duty = duty;
	cal_count(channel);
	TLS_DBGPRT_INFO("duty set time=%d,high=%d,low=%d\n",pwm_info[channel].duty,pwm_info[channel].high_count,pwm_info[channel].low_count);
}

void tls_pwm_freq_set(u16 channel, u16 freq)
{
	if(channel > (PWM_CHANNEL_MAX_NUM - 1))
	{
		TLS_DBGPRT_ERR("freq param err\n");
		return;
	}
	if(freq > PWM_MAX_FREQ)
		pwm_info[channel].freq = PWM_MAX_FREQ;
	else if(freq < 1)
		pwm_info[channel].freq = 1;
	else
		pwm_info[channel].freq = freq;
	cal_count(channel);
	TLS_DBGPRT_INFO("freq set time=%d,high=%d,low=%d\n",pwm_info[channel].duty,pwm_info[channel].high_count,pwm_info[channel].low_count);
}

void tls_pwm_freq_duty_set(u16 channel, u16 freq, u8 duty)
{
	if(channel > (PWM_CHANNEL_MAX_NUM - 1))
	{
		TLS_DBGPRT_ERR("freq param err\n");
		return;
	}
	if(freq > PWM_MAX_FREQ)
		pwm_info[channel].freq = PWM_MAX_FREQ;
	else if(freq < 1)
		pwm_info[channel].freq = 1;
	else
		pwm_info[channel].freq = freq;

	if(duty >= PWM_DEPTH)
		pwm_info[channel].duty = PWM_DEPTH;
	else if(duty < 1)
		pwm_info[channel].duty = 0;
	else
		pwm_info[channel].duty = duty;
	
	cal_count(channel);
	TLS_DBGPRT_INFO("time=%d,high=%d,low=%d\n",pwm_info[channel].duty,pwm_info[channel].high_count,pwm_info[channel].low_count);
}


int tls_pwm_init(u16 channum,u16 *freq, u8 *duty)
{
	int i;
	
	if(NULL == freq || NULL == duty)
		return WM_FAILED;
	
	if(channum > PWM_CHANNEL_MAX_NUM)
	{
		channum = PWM_CHANNEL_MAX_NUM;
	}

	for(i = 0;i < channum;i ++)
	{
		tls_gpio_cfg(pwm_io[i], TLS_GPIO_DIR_OUTPUT, TLS_GPIO_ATTR_FLOATING);
		tls_pwm_freq_duty_set(i, *(freq + i), *(duty + i));
	}
	pwm_num = channum;	
	tls_timer_irq_register(pwm_timer_cb);
	tls_timer_start(PWM_BASE_TIME_COUNT);	
	tls_set_pwm_flag(1);
	
	return 	WM_SUCCESS;
}

void tls_pwm_stop(void)
{
	tls_timer_stop();
	tls_set_pwm_flag(0);
}

//0 表示关闭，1表示打开了
//根据标志切换cpu速度
void tls_set_pwm_flag(u8 flag)
{
	if(1 == flag)
	{
		tls_sys_clk_set(CPU_CLK_160M);	
		pwm_on = 1;
	}
	else
	{
		pwm_on = 0;	
		tls_sys_clk_set(CPU_CLK_80M);	
	}
}

u8 tls_get_pwm_flag(void)
{
	return pwm_on;
}


