/***************************************************************************** 
* 
* File Name : wm_cpu.c 
* 
* Description: cpu Module 
* 
* Copyright (c) 2014 Winner Microelectronics Co., Ltd. 
* All rights reserved. 
* 
* Author : kevin 
* 
* Date : 2014-6-3 
*****************************************************************************/

#include "wm_debug.h"
#include "wm_regs.h"
#include "wm_irq.h"
#include "wm_cpu.h"
#include "wm_pwm.h"

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
void tls_watchdog_clr(void)
{
	tls_reg_write32(HR_WDG_INT_CTRL, 0x01);
}

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
void tls_watchdog_isr(void *data)
{
	TLS_DBGPRT_INFO("### kevin debug watchdog reset...\r\n");
}

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
void tls_watchdog_init(void)
{
	TLS_DBGPRT_INFO("### kevin debug watchdog init.\r\n");
	tls_irq_register_handler(WATCHDOG_INT, tls_watchdog_isr, NULL); 
	tls_irq_enable(WATCHDOG_INT);
	tls_reg_write32(HR_WDG_LOAD_VALUE, 0xBEBC200);	// 5S
	tls_reg_write32(HR_WDG_CTRL, 0x3);				// enable irq & reset
}

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
void tls_sys_reset(void)
{
	tls_reg_write32(HR_WDG_LOAD_VALUE, 0x100);
	tls_reg_write32(HR_WDG_CTRL, 0x3);
}


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
void tls_sys_clk_set(u32 clk)
{
	u32 RegValue;

	if(1 == tls_get_pwm_flag())	//pwm运行的时候,cpu会被切换到160M，不允许降低,否则可能有就频偏
		return;

	RegValue = tls_reg_read32(HR_CLK_DIV_CTL);
	RegValue &= ~0xFF00FF;

	switch(clk)
	{
		case CPU_CLK_40M:		// 40M
			RegValue |= 0x8001000C;
			break;
			
		case CPU_CLK_160M:		// 160M
			RegValue |= 0x80040003;
			break;
			
		default:	// 80M
			RegValue |= 0x80020006;
			break;
	}
	tls_reg_write32(HR_CLK_DIV_CTL, RegValue);
	
}


