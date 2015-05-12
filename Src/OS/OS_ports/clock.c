/***************************************************************************** 
* 
* File Name : clock.c
* 
* Description: os timer module
* 
* Copyright (c) 2014 Winner Micro Electronic Design Co., Ltd. 
* All rights reserved. 
* 
* Author : dave
* 
* Date : 2014-6-12 
*****************************************************************************/ 

#include "wm_regs.h"
#include "wm_timer.h"
#include "wm_osal.h"
#include "wm_irq.h"

void tls_os_timer_isr(void *data)
{
    u32 value;
#if 	TLS_OS_FREERTOS
	 extern void freeRtosTickIrqHandler( void );
#endif
#if TLS_OS_UCOS
    OSTimeTick();                     /* Call uC/OS-II's OSTimeTick()        */
#elif TLS_OS_FREERTOS
	//printf("\n-\n");
	freeRtosTickIrqHandler();
#endif

 	/* clear timer0 interrupt */
    value = tls_reg_read32(HR_TIMER0_CSR);
	tls_reg_write32(HR_TIMER0_CSR, value);
}

void tls_os_timer_init(void)
{
    u32 timer_csr;
    u32 cpu_sr; /* Allocate storage for CPU status register   */

    cpu_sr = tls_os_set_critical();

    /** 
     * set up timer, 定时器的us基准值：时钟频率(bus2频率)-1，
     * 如果时钟频率(bus2)是40MHz，则设置为40-1=39 
     */
    tls_reg_write32(HR_TIMER_CFG, 39);			
#if TLS_OS_UCOS
    timer_csr = ((1000/OS_TICKS_PER_SEC) << TLS_TIMER_VALUE_S) | TLS_TIMER_MS_UNIT | TLS_TIMER_EN;
#elif TLS_OS_FREERTOS
    timer_csr = ((1000/configTICK_RATE_HZ) << TLS_TIMER_VALUE_S) | TLS_TIMER_MS_UNIT | TLS_TIMER_EN;
#endif
    tls_reg_write32(HR_TIMER0_CSR, timer_csr);

    tls_irq_register_handler(TIMER0_INT, tls_os_timer_isr, NULL);

    /* Enable timer0 interrupts */
    /* 1. timer0 config register interrupt bit enable */
    tls_reg_write32(HR_TIMER0_CSR, tls_reg_read32(HR_TIMER0_CSR) | TLS_TIMER_INT_EN);
    /* 2. enable time0 interrupt bit in vic enable register */
    tls_irq_enable(TIMER0_INT);

    tls_os_release_critical(cpu_sr);
}

