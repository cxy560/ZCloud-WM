/***************************************************************************** 
* 
* File Name : wm_rtc.c 
* 
* Description: rtc Driver Module 
* 
* Copyright (c) 2014 Winner Microelectronics Co., Ltd. 
* All rights reserved. 
* 
* Author : dave
* 
* Date : 2014-10-23
*****************************************************************************/ 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "wm_regs.h"
#include "wm_rtc.h"

void tls_set_rtc(struct tm *tblock)
{
	int ctrl1 = 0;
	int ctrl2 = 0;

	ctrl2 = tls_reg_read32(HR_PMU_RTC_CTRL2);	//disable
	ctrl2 &= ~(1<<16);
	tls_reg_write32(HR_PMU_RTC_CTRL2, ctrl2);

	ctrl1 |= tblock->tm_sec;
	ctrl1 |= tblock->tm_min<<8;
	ctrl1 |= tblock->tm_hour<<16;
	ctrl1 |= tblock->tm_mday<<24;
	tls_reg_write32(HR_PMU_RTC_CTRL1, ctrl1);

	ctrl2 = 0;
	ctrl2 |= tblock->tm_mon + 1;
	ctrl2 |= (tblock->tm_year - 100)<<8;
	tls_reg_write32(HR_PMU_RTC_CTRL2, ctrl2);

	ctrl2 = tls_reg_read32(HR_PMU_RTC_CTRL2);		//enable
	ctrl2 |= (1<<16);
	tls_reg_write32(HR_PMU_RTC_CTRL2, ctrl2);
}

void tls_get_rtc(struct tm *tblock)
{
	int ctrl1 = 0;
	int ctrl2 = 0;

	ctrl1 = tls_reg_read32(HR_PMU_RTC_CTRL1);
	ctrl2 = tls_reg_read32(HR_PMU_RTC_CTRL2);
	tblock->tm_year = ((int)(((int)ctrl2&0x00003f00)>>8) + 100);
	tblock->tm_mon = (ctrl2&0x0000000f) - 1;
	tblock->tm_mday = (ctrl1&0x1f000000)>>24;
	tblock->tm_hour = (ctrl1&0x001f0000)>>16;
	tblock->tm_min = (ctrl1&0x00003f00)>>8;
	tblock->tm_sec = ctrl1&0x0000003f;
}


