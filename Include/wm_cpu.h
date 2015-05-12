/***************************************************************************** 
* 
* File Name : wm_cpu.h 
* 
* Description: cpu module 
* 
* Copyright (c) 2014 Winner Micro Electronic Design Co., Ltd. 
* All rights reserved. 
* 
* Author : dave
* 
* Date : 2014-6-12 
*****************************************************************************/ 

#ifndef WM_CPU_H
#define WM_CPU_H


#define CPU_CLK_80M		0
#define CPU_CLK_40M		1
#define CPU_CLK_160M	2

/*************************************************************************** 
* Function: tls_watchdog_clr 
*
* Description: This function is used to clear watchdog. 
* 
* Input: None 
* 
* Output: None 
* 
* Return: None 
* 
* Date : 2014-6-3 
****************************************************************************/ 
void tls_watchdog_clr(void);

/*************************************************************************** 
* Function: tls_watchdog_isr 
*
* Description:  This function is used to report system reset.  
* 
* Input: None 
* 
* Output: None 
* 
* Return: None 
* 
* Date : 2014-6-3 
****************************************************************************/ 
void tls_watchdog_isr(void *data);

/*************************************************************************** 
* Function: tls_watchdog_init
*
* Description: This function is used to init watchdog. 
* 
* Input: None 
* 
* Output: None 
* 
* Return: None 
* 
* Date : 2014-6-3 
****************************************************************************/ 
void tls_watchdog_init(void);

/*************************************************************************** 
* Function: tls_sys_reset
*
* Description: This function is used to reset system. 
* 
* Input: None 
* 
* Output: None 
* 
* Return: None 
* 
* Date : 2014-6-12
****************************************************************************/ 
void tls_sys_reset(void);

/*************************************************************************** 
* Function: tls_sys_clk_set
*
* Description: This function is used to set cpu clock. 
* 
* Input: 		clk		select cpu clock
				clk == CPU_CLK_80M	80M
*				clk == CPU_CLK_40M	40M
*				clk == CPU_CLK_160M	160M
* 
* Output: None 
* 
* Return: None 
* 
* Date : 2014-6-12
****************************************************************************/ 
void tls_sys_clk_set(u32 clk);


#endif /* WM_CPU_H */
