/***************************************************************************** 
* 
* File Name : wm_timer.h 
* 
* Description: Timer Driver Module 
* 
* Copyright (c) 2014 Winner Microelectronics Co., Ltd. 
* All rights reserved. 
* 
* Author : dave 
* 
* Date : 2014-6-2 
*****************************************************************************/ 
#ifndef WM_TIMER_H
#define WM_TIMER_H



/**********************************************************************************************************
* Description: 	This function is used to register timer1 interrupt.
*
* Arguments  : 	
*
* Returns    : 	
**********************************************************************************************************/
void tls_timer_irq_register(void (*callback)(void));

/**********************************************************************************************************
* Description: 	This function is used to set the timer and start the timer, the unit is us.
*
* Arguments  : 	timeout 		Timer number
*
* Returns    : 	
**********************************************************************************************************/
void tls_timer_start(int timeout);

/**********************************************************************************************************
* Description: 	This function is used to stop the timer.
*
* Arguments  : 	
*
* Returns    : 	
**********************************************************************************************************/
void tls_timer_stop(void);

/**********************************************************************************************************
* Description: 	This function is used to clear timer irq.
*
* Arguments  : 	
*
* Returns    : 	
**********************************************************************************************************/
void tls_timer_clear_irq(void);


#endif /* WM_TIMER_H */
