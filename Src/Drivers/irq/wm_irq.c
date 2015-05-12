/***************************************************************************** 
* 
* File Name : wm_irq.c
* 
* Description: interupt driver module 
* 
* Copyright (c) 2014 Winner Micro Electronic Design Co., Ltd. 
* All rights reserved. 
* 
* Author : dave
* 
* Date : 2014-6-12 
*****************************************************************************/ 
#include <stdio.h>
#include "wm_regs.h"
#include "wm_irq.h"
#include "wm_config.h"

void tls_irq_handler_dummy(void *data)
{
}


tls_irq_handler_t intr_handler_vect[INTR_CNT];

u8 intr_counter;

void tls_irq_init(void)
{
    int i;

    for (i=0; i<INTR_CNT; i++) {
        intr_handler_vect[i].handler = tls_irq_handler_dummy;
        intr_handler_vect[i].data = NULL;
        intr_handler_vect[i].name = "intr_dummy";
        intr_handler_vect[i].counter = 0;
    }

    intr_counter = 0;

    /* initailize VIC */
    tls_reg_write32(HR_VIC_INT_SELECT, 0);
    tls_reg_write32(HR_VIC_VECT_ENABLE, 0); /* disable vector interrupt */
    tls_reg_write32(HR_VIC_INT_EN_CLR, 0xFFFFFFFF);
}

void tls_irq_register_handler (u8 vec_no, intr_handler_func handler,
                   void *data) 
{
    //ASSERT (vec_no >= 0 && vec_no <= 0x1f);

    intr_handler_vect[vec_no].handler = handler;
    intr_handler_vect[vec_no].data = data;
    //intr_handler_vect[vec_no].name = NULL;
    intr_handler_vect[vec_no].counter = 0;
}

void tls_irq_enable(u8 vec_no)
{
    tls_reg_write32(HR_VIC_INT_ENABLE, 
            tls_reg_read32(HR_VIC_INT_ENABLE) | (1UL<<vec_no));
}

void tls_irq_disable(u8 vec_no)
{
    tls_reg_write32(HR_VIC_INT_ENABLE, 
            tls_reg_read32(HR_VIC_INT_ENABLE) & ~(1UL<<vec_no));
}

#if TLS_OS_UCOS
void freeRtos_irq_isr_handler(void)
{}
void OS_CPU_IRQ_ISR_Handler(void)
#elif TLS_OS_FREERTOS
void OS_CPU_IRQ_ISR_Handler(void)
{}
void freeRtos_irq_isr_handler(void)
#endif
{
    u32 irq_status = tls_reg_read32(HR_VIC_IRQ_STATUS);

	intr_counter++;

    if (irq_status & (1UL<<SDIO_DOWN_DATA_INT)) {
        intr_handler_vect[SDIO_DOWN_DATA_INT].handler(
                (void *)intr_handler_vect[SDIO_DOWN_DATA_INT].data);
        intr_handler_vect[SDIO_DOWN_DATA_INT].counter++;
    }

    if (irq_status & (1UL<<SDIO_DOWN_CMD_INT)) {
        intr_handler_vect[SDIO_DOWN_CMD_INT].handler(
                (void *)intr_handler_vect[SDIO_DOWN_CMD_INT].data);
        intr_handler_vect[SDIO_DOWN_CMD_INT].counter++;
    }

    if (irq_status & (1UL<<SDIO_UP_DATA_INT)) {
        intr_handler_vect[SDIO_UP_DATA_INT].handler(
                (void *)intr_handler_vect[SDIO_UP_DATA_INT].data);
        intr_handler_vect[SDIO_UP_DATA_INT].counter++;
    }

    if (irq_status & (1UL<<TIMER0_INT)) {
        intr_handler_vect[TIMER0_INT].handler(
                (void *)intr_handler_vect[TIMER0_INT].data);
        intr_handler_vect[TIMER0_INT].counter++;
    }
	
    if (irq_status & (1UL<<TIMER1_INT)) {
        intr_handler_vect[TIMER1_INT].handler(
                (void *)intr_handler_vect[TIMER1_INT].data);
        intr_handler_vect[TIMER1_INT].counter++;
    } 
	
    if (irq_status & (1UL<<TIMER2_INT)) {
        intr_handler_vect[TIMER2_INT].handler(
                (void *)intr_handler_vect[TIMER2_INT].data);
        intr_handler_vect[TIMER2_INT].counter++;
    }    

    if(irq_status & (1UL<<MAC_INT)) {
        intr_handler_vect[MAC_INT].handler(
                (void *)intr_handler_vect[MAC_INT].data);
        intr_handler_vect[MAC_INT].counter++;
    }

    if (irq_status & (1UL<<TX_MGMT_COMPLETE_INT)) {
        intr_handler_vect[TX_MGMT_COMPLETE_INT].handler(
                (void *)intr_handler_vect[TX_MGMT_COMPLETE_INT].data);
        intr_handler_vect[TX_MGMT_COMPLETE_INT].counter++;
    }

    if (irq_status & (1UL<<TX_DATA_COMPLETE_INT)) {
        intr_handler_vect[TX_DATA_COMPLETE_INT].handler(
                (void *)intr_handler_vect[TX_DATA_COMPLETE_INT].data);
        intr_handler_vect[TX_DATA_COMPLETE_INT].counter++;
    }

    if (irq_status & (1UL<<RX_INT)) {
        intr_handler_vect[RX_INT].handler((void *)intr_handler_vect[RX_INT].data);
        intr_handler_vect[RX_INT].counter++;
    }

    if (irq_status & (1UL<<UART0_INT)) {
        intr_handler_vect[UART0_INT].handler((void *)intr_handler_vect[UART0_INT].data);
        intr_handler_vect[UART0_INT].counter++;
    }

    if (irq_status & (1UL<<UART1_INT)) {
        intr_handler_vect[UART1_INT].handler((void *)intr_handler_vect[UART1_INT].data);
        intr_handler_vect[UART1_INT].counter++;
    }

    if (irq_status & (1UL<<SPI0_INT)) {
        intr_handler_vect[SPI0_INT].handler((void *)intr_handler_vect[SPI0_INT].data);
        intr_handler_vect[SPI0_INT].counter++;
    }

    if (irq_status & (1UL<<PM_RTC_INT)) {
        intr_handler_vect[PM_RTC_INT].handler((void *)intr_handler_vect[PM_RTC_INT].data);
        intr_handler_vect[PM_RTC_INT].counter++;
    }

    if (irq_status & (1UL<<PM_SLEEP_PERIOD_TIMER_INT)) {
        intr_handler_vect[PM_SLEEP_PERIOD_TIMER_INT].handler((void *)intr_handler_vect[PM_SLEEP_PERIOD_TIMER_INT].data);
        intr_handler_vect[PM_SLEEP_PERIOD_TIMER_INT].counter++;
    }

    if (irq_status & (1UL<<GPIO_WAKEUP_INT)) {
        intr_handler_vect[GPIO_WAKEUP_INT].handler((void *)intr_handler_vect[GPIO_WAKEUP_INT].data);
        intr_handler_vect[GPIO_WAKEUP_INT].counter++;
    }

	if (irq_status & (1UL<<GPIO_INT)) {
        intr_handler_vect[GPIO_INT].handler((void *)intr_handler_vect[GPIO_INT].data);
        intr_handler_vect[GPIO_INT].counter++;
    }
	
	if (irq_status & (1UL<<DMA0_INT)) {
        intr_handler_vect[DMA0_INT].handler((void *)intr_handler_vect[DMA0_INT].data);
        intr_handler_vect[DMA0_INT].counter++;
    }
	if (irq_status & (1UL<<DMA1_INT)) {
        intr_handler_vect[DMA1_INT].handler((void *)intr_handler_vect[DMA1_INT].data);
        intr_handler_vect[DMA1_INT].counter++;
    }
	if (irq_status & (1UL<<DMA2_INT)) {
        intr_handler_vect[DMA2_INT].handler((void *)intr_handler_vect[DMA2_INT].data);
        intr_handler_vect[DMA2_INT].counter++;
    }
	if (irq_status & (1UL<<DMA3_INT)) {
        intr_handler_vect[DMA3_INT].handler((void *)intr_handler_vect[DMA3_INT].data);
        intr_handler_vect[DMA3_INT].counter++;
    }
	if (irq_status & (1UL<<DMA4_INT)) {
        intr_handler_vect[DMA4_INT].handler((void *)intr_handler_vect[DMA4_INT].data);
        intr_handler_vect[DMA4_INT].counter++;
    }

	if (irq_status & (1UL<<ADC_INT)) {
        intr_handler_vect[ADC_INT].handler((void *)intr_handler_vect[ADC_INT].data);
        intr_handler_vect[ADC_INT].counter++;
    }

	intr_counter --;
    /* clear interrupt */
    tls_reg_write32(HR_VIC_VECT_ADDR, 0);
}

u8 tls_get_isr_count(void)
{
	return intr_counter;
}
