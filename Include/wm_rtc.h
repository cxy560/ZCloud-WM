/***************************************************************************** 
* 
* File Name : wm_rtc.h 
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

#ifndef WM_RTC_H
#define WM_RTC_H

#include "wm_type_def.h"
#include <time.h>

void tls_set_rtc(struct tm *tblock);

void tls_get_rtc(struct tm *tblock);


#endif
