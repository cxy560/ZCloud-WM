/***************************************************************************** 
* 
* File Name : wm_irq.h 
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

#ifndef WM_IRQ_H
#define WM_IRQ_H

#include "wm_type_def.h"

/* Number of interrupts. */
#define INTR_CNT 32 

#define SDIO_UP_DATA_INT            (0UL)
#define SDIO_DOWN_DATA_INT          (1UL)
#define SDIO_UP_CMD_INT             (2UL)
#define SDIO_DOWN_CMD_INT           (3UL)
#define MAC_INT                     (4UL)

#define RX_INT                      (6UL)
#define TX_MGMT_COMPLETE_INT        (7UL)
#define TX_DATA_COMPLETE_INT        (8UL)
#define PM_RTC_INT                  (9UL)
#define PM_SLEEP_PERIOD_TIMER_INT   (10UL)
#define GPIO_WAKEUP_INT             (11UL)
#define SDIO_CMD_WAKEUP_INT         (12UL)
#define DMA0_INT                    (13UL)
#define DMA1_INT                    (14UL)
#define DMA2_INT                    (15UL)
#define DMA3_INT                    (16UL)
#define DMA4_INT                    (17UL)
#define DMA_BURST_INT               (18UL)
#define I2C_INT                     (19UL)
#define ADC_INT                     (20UL)
#define SPI0_INT                    (21UL)
#define SPI1_INT                    (22UL)
#define UART0_INT                   (23UL)
#define UART1_INT                   (24UL)
#define GPIO_INT                    (25UL)
#define TIMER0_INT                  (26UL)
#define TIMER1_INT                  (27UL)
#define TIMER2_INT                  (28UL)
#define WATCHDOG_INT                (29UL)
#define SPI_RF_INT                  (30UL)

typedef void (*intr_handler_func)(void *);

typedef struct tls_irq_handler {
    void        (*handler)(void *);
    void        *data;
	const char	*name;
    u32         counter;
} tls_irq_handler_t;


/**********************************************************************************************************
* Description: 	This function is used to initial system interrupt.
*
* Arguments  : 	
*
* Returns    : 
*
**********************************************************************************************************/
void tls_irq_init(void);

/**********************************************************************************************************
* Description: 	This function is used to register interrupt.
*
* Arguments  : 	vec_no		interrupt no
*				handler		interrupt callback function
*				data			argument of callback function 
*
* Returns    : 
*
**********************************************************************************************************/
void tls_irq_register_handler (u8 vec_no, intr_handler_func handler, void *data);


/**********************************************************************************************************
* Description: 	This function is used to enable interrupt.
*
* Arguments  : 	vec_no		interrupt no
*
* Returns    : 
*
**********************************************************************************************************/
void tls_irq_enable(u8 vec_no);

/**********************************************************************************************************
* Description: 	This function is used to disable interrupt.
*
* Arguments  : 	vec_no		interrupt no
*
* Returns    : 
*
**********************************************************************************************************/
void tls_irq_disable(u8 vec_no);


u8 tls_get_isr_count(void);


#endif /* WM_IRQ_H */
