/***************************************************************************** 
* 
* File Name : wm_adc.c 
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "wm_irq.h"
#include "wm_regs.h"
#include "wm_adc.h"
#include "wm_dma.h"

ST_ADC gst_adc;

static void adc_cpu_isr(void)
{
	u16 adcvalue;
	
	tls_adc_clear_irq(ADC_INT_TYPE_ADC);
	adcvalue = tls_read_adc_result();
	if(gst_adc.adc_cb)
		gst_adc.adc_cb(&adcvalue,1);
	//printf("\ncpu adc result=%x\n",adcvalue);
}

static void adc_dma_isr(void)
{
	tls_adc_clear_irq(ADC_INT_TYPE_DMA);
	if(gst_adc.adc_dma_cb)
		gst_adc.adc_dma_cb((u16 *)(ADC_DEST_BUFFER_DMA + 24), gst_adc.valuelen);	//前面12个半字需要舍去

}


void tls_adc_init(u8 ifusedma,u8 dmachannel)
{
	tls_reg_write32(HR_SD_ADC_CONFIG_REG, 0);
	tls_irq_register_handler(ADC_INT,(intr_handler_func)adc_cpu_isr,NULL);
	tls_irq_enable(ADC_INT);

//注册中断和channel有关，所以需要先请求
	if(ifusedma)
	{
		gst_adc.dmachannel = tlsDmaRequest(dmachannel, NULL);	//请求dma，不要直接指定，因为请求的dma可能会被别的任务使用
		if(gst_adc.dmachannel <= 3)
		{
			tls_irq_register_handler(DMA0_INT + gst_adc.dmachannel,(intr_handler_func)adc_dma_isr,NULL);
			tls_irq_enable(DMA0_INT + gst_adc.dmachannel);
		}
		else
		{
			tls_irq_register_handler(DMA4_INT,(intr_handler_func)adc_dma_isr,NULL);
			tls_irq_enable(DMA4_INT);
		}
	}

//	printf("\ndma channel = %d\n",gst_adc.dmachannel);		
}

void tls_adc_clear_irq(int inttype)
{
	if(ADC_INT_TYPE_ADC == inttype)
	{
		tls_reg_write32(HR_SD_ADC_INT_SOURCE_REG, 0x01);
	}
	else if(ADC_INT_TYPE_DMA == inttype)
	{
		DMA_INTSRC_REG |= 0x03<<(gst_adc.dmachannel*2);
	}
}

void tls_adc_irq_register(int inttype, void (*callback)(u16 *buf, u16 len))
{
	if(ADC_INT_TYPE_ADC == inttype)
	{
		gst_adc.adc_cb = callback;
	}
	else if(ADC_INT_TYPE_DMA == inttype)
	{
		gst_adc.adc_dma_cb = callback;
	}
}


u16 tls_read_adc_result(void)
{
	u32 value;
	u16 ret;
	
	value = tls_reg_read32(HR_SD_ADC_RESULT_REG);
	ret = value&0xfff;
	
	return ret;
}

void tls_adc_start_with_cpu(int Channel, int Accuracy)
{
	u32 value;
	
	Channel &= CONFIG_ADC_CHL_MASK;
	Accuracy &= CONFIG_ADC_SDIV_MASK;

	/* Stop adc first */
	value = tls_reg_read32(HR_SD_ADC_CONFIG_REG);
	value &= ~CONFIG_ADC_START;
	tls_reg_write32(HR_SD_ADC_CONFIG_REG, value);
	tls_reg_write32(HR_SD_ADC_CONFIG_REG, (Accuracy<<3) | Channel);
	tls_reg_write32(HR_SD_ADC_CTRL_REG, 0x00500A00);

	tls_reg_write32(HR_SD_ADC_INT_MASK_REG, 0);
	
	value = tls_reg_read32(HR_SD_ADC_CONFIG_REG);
	value |= CONFIG_ADC_START;
	tls_reg_write32(HR_SD_ADC_CONFIG_REG, value);		/*start adc*/
}
#if 0
static void adc_delay(int time)
{
	while(time--);
}
#endif
void tls_adc_start_with_dma(int Channel, int Accuracy, int Length)
{
	u32 value;
	int len;

	if(Channel < 0 || Channel > 5)
		return;
	if(Length > ADC_DEST_BUFFER_SIZE)
		len = ADC_DEST_BUFFER_SIZE;
	else
		len = Length;

	gst_adc.valuelen = Length;

	Channel &= CONFIG_ADC_CHL_MASK;
	Accuracy &= CONFIG_ADC_SDIV_MASK;

	/* Stop adc first */
	value = tls_reg_read32(HR_SD_ADC_CONFIG_REG);
	value &= ~CONFIG_ADC_START;
	tls_reg_write32(HR_SD_ADC_CONFIG_REG, value);
	
	/* Stop dma if necessary */
	while(DMA_CHNLCTRL_REG(gst_adc.dmachannel) & 1)
	{
		DMA_CHNLCTRL_REG(gst_adc.dmachannel) = 2;
	}

	DMA_SRCADDR_REG(gst_adc.dmachannel) = HR_SD_ADC_RESULT_REG;
	DMA_DESTADDR_REG(gst_adc.dmachannel) = ADC_DEST_BUFFER_DMA;
	/* Hard, Normal, adc_req */
	if (Channel == 4){
		DMA_MODE_REG(gst_adc.dmachannel) = (0x01 | (0+8)<<2);
	}
	else if (Channel == 5){
		DMA_MODE_REG(gst_adc.dmachannel) = (0x01 | (2+8)<<2);
	}
	else{
		DMA_MODE_REG(gst_adc.dmachannel) = (0x01 | (Channel+8)<<2);
	}	
	
	/* Dest_add_inc, halfword,  */
	DMA_CTRL_REG(gst_adc.dmachannel) = (1<<2)|(1<<3)|((len*2 + 24)<<6);	//多采集12个半字，因为前面的12个半字由于adc不稳定，需要舍去
	DMA_INTMASK_REG &= ~(0x01 << (gst_adc.dmachannel *2 + 1));
	DMA_CHNLCTRL_REG(gst_adc.dmachannel) = 1;		/* Enable dma */

	tls_reg_write32(HR_SD_ADC_CONFIG_REG, (Accuracy<<3) | Channel);
	
	value = 0;
	value |= ((0x1<<1) | (0x3ff<<5) |(0x3ff<<16));
	tls_reg_write32(HR_SD_ADC_CTRL_REG, value);

	tls_reg_write32(HR_SD_ADC_INT_MASK_REG,1);

	value = tls_reg_read32(HR_SD_ADC_CONFIG_REG);
	value |= CONFIG_ADC_START;
	tls_reg_write32(HR_SD_ADC_CONFIG_REG, value);		/*start adc*/
}


void tls_adc_stop(int ifusedma)
{
	u32 value;

	value = tls_reg_read32(HR_SD_ADC_CONFIG_REG);
	value &= ~CONFIG_ADC_START;
	tls_reg_write32(HR_SD_ADC_CONFIG_REG, value);

	if(ifusedma)
		tlsDmaFree(gst_adc.dmachannel);
}


