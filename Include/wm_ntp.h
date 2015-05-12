/***************************************************************************** 
* 
* File Name : wm_ntp.h 
* 
* Description: ntp module 
* 
* Copyright (c) 2014 Winner Micro Electronic Design Co., Ltd. 
* All rights reserved. 
* 
* Author : dave
* 
* Date : 2014-11-08 
*****************************************************************************/ 

#ifndef WM_NTP_H
#define WM_NTP_H

#include "wm_type_def.h"


/**********************************************************************************************************
* Description: 	This function is used to get network time.
*
* Arguments  : 	
*
* Returns    : 		is relative to the 1970 time, the unit is seconds.
**********************************************************************************************************/
unsigned int ntp_client(void);

/**********************************************************************************************************
* Description: 	This function is used to set ntp server ipaddr.
*
* Arguments  : 	ipaddr	max num is four,IP addr is separated by ';'
*				
*
* Returns    : 		WM_SUCCESS		set success
* 				WM_FAILED		set failed
**********************************************************************************************************/
void tls_set_ntp_server(char *ipaddr);


#endif /* WM_NTP_H */
