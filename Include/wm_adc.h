/***************************************************************************** 
* 
* File Name : wm_adc.h 
* 
* Description: adc Driver Module 
* 
* Copyright (c) 2014 Winner Microelectronics Co., Ltd. 
* All rights reserved. 
* 
* Author : dave
* 
* Date : 2014-8-15
*****************************************************************************/ 

#ifndef WM_ADC_H
#define WM_ADC_H

#include "wm_type_def.h"

//每次启动dma之后，需要一段稳定时间，所以采集到的数据前面的12个byte不稳定，要舍去
#define ADC_DEST_BUFFER_DMA	0x61100ul		//dma搬移只能搬移到sdram地址中0x60000之后，这段地址和hostspi的dma地址复用
#define ADC_DEST_BUFFER_SIZE	800			//以半字为单位	//0x61800之后的buffer用于高速spi从接口通信，如果不使用高速spi，这个size可以改大

#define CONFIG_ADC_START  (1<<5)
#define CONFIG_ADC_SDIV_64    (0x0<<3)
#define CONFIG_ADC_SDIV_128  (0x1<<3)
#define CONFIG_ADC_SDIV_256  (0x2<<3)
#define CONFIG_ADC_SDIV_512  (0x3<<3)
#define CONFIG_ADC_SDIV_MASK  (0x3)
#define CONFIG_ADC_CHL_MASK (0x7)

#define ADC_SAMPLE_ACCURACY_7Bits    0
#define ADC_SAMPLE_ACCURACY_9Bits    1
#define ADC_SAMPLE_ACCURACY_10Bits    2
#define ADC_SAMPLE_ACCURACY_12Bits    3

#define ADC_INT_TYPE_ADC	0
#define ADC_INT_TYPE_DMA	1

typedef struct adc_st{
	u8 dmachannel;
	void (*adc_cb)(u16 *buf, u16 len);
	void (*adc_dma_cb)(u16 *buf,u16 len);
	u16 valuelen;		/*dma 采样数据长度*/
}ST_ADC;

/**********************************************************************************************************
* Description: 	This function is used to init adc.
*
* Arguments  : 	ifusedma		if use dma
*				dmachannel	dma channel
*
* Returns    : 
* 		
* Notes:			If the requested dma channel is already used by other task, system will auto use other dma channel.
**********************************************************************************************************/
void tls_adc_init(u8 ifusedma,u8 dmachannel);


/**********************************************************************************************************
* Description: 	This function is used to register interrupt callback function.
*
* Arguments  : 	inttype			interrupt type
*				ADC_INT_TYPE_ADC		adc interrupt,user get adc result from the callback function.
*				ADC_INT_TYPE_DMA		dma interrupt,dma transfer the adc result to the user's buffer.
*				callback			interrupt callback function.
* Returns    : 
* 		
**********************************************************************************************************/
void tls_adc_irq_register(int inttype, void (*callback)(u16 *buf, u16 len));

/**********************************************************************************************************
* Description: 	This function is used to clear the interrupt source.
*
* Arguments  : 	inttype			interrupt type
*
* Returns    : 
* 		
**********************************************************************************************************/
void tls_adc_clear_irq(int inttype);

/**********************************************************************************************************
* Description: 	This function is used to start adc,use dma for transfer data.
*
* Arguments  : 	channel		adc channel,from 0 to 3 is single input;4 and 5 is differential input.
*				accuracy		Sampling precision
*							ADC_SAMPLE_ACCURACY_7Bits
*							ADC_SAMPLE_ACCURACY_9Bits
*							ADC_SAMPLE_ACCURACY_10Bits
*							ADC_SAMPLE_ACCURACY_12Bits
*				length		byte data length,is an integer multiple of half word,need <= 0x500
* Returns    : 
* 		
**********************************************************************************************************/
void tls_adc_start_with_dma(int Channel, int Accuracy, int Length);

/**********************************************************************************************************
* Description: 	This function is used to start adc.
*
* Arguments  : 	channel		adc channel,from 0 to 3 is single input;4 and 5 is differential input.
*				accuracy		Sampling precision
*							ADC_SAMPLE_ACCURACY_7Bits
*							ADC_SAMPLE_ACCURACY_9Bits
*							ADC_SAMPLE_ACCURACY_10Bits
*							ADC_SAMPLE_ACCURACY_12Bits
* Returns    : 
**********************************************************************************************************/
void tls_adc_start_with_cpu(int Channel, int Accuracy);

/**********************************************************************************************************
* Description: 	This function is used to read adc result.
*
* Arguments  : 	
* Returns    : 
**********************************************************************************************************/
u16 tls_read_adc_result(void);

/**********************************************************************************************************
* Description: 	This function is used to stop the adc.
*
* Arguments  : 	ifusedma		if use dma
* Returns    : 
**********************************************************************************************************/
void tls_adc_stop(int ifusedma);

#endif
