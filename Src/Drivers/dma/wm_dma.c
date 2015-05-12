/***************************************************************************** 
* 
* File Name : wm_dma.c 
* 
* Description: DMA Driver Module 
* 
* Copyright (c) 2014 Winner Microelectronics Co., Ltd. 
* All rights reserved. 
* 
* Author : dave
* 
* Date : 2014-6-4
*****************************************************************************/ 



#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "wm_mem.h"
#include "list.h"
#include "wm_debug.h"
#include "wm_flash.h"
#include "utils.h"
#include "wm_irq.h"
#include "wm_regs.h"
#include "wm_dma.h"

TLS_DMA_CHANNELS channels;


/*   tlsDmaWaitComplt   */
/*-------------------------------------------------------------------------
	Description:	
		Wait until DMA operation completes.
	Arguments:
		ch: DMA channel no.		
	Return Value:
		None.
	Note:	
-------------------------------------------------------------------------*/
int tlsDmaWaitComplt(unsigned ch)
{
	unsigned long timeout = 0;
	
	while(DMA_CHNLCTRL_REG(ch) & DMA_CHNL_CTRL_CHNL_ON) 
	{
		tls_os_time_delay(1);
		timeout ++;
		if(timeout > 500)
			return -1;
	}

	return 0;
}

/*   tlsDmaStart   */
/*-------------------------------------------------------------------------
	Description:	
		Start the DMA controller.
	Arguments:
		autoReload: Does restart when current transfer complete?	
		ch: Channel no.
		pDmaDesc: Pointer to DMA channel descriptor structure.	
	Return Value:
		Always STATUS_SUCCESS.
	Note:	
		DMA Descriptor:
		+--------------------------------------------------------------+
		|Vld[31] |                    RSV                              |
	 	+--------------------------------------------------------------+
	 	|                  RSV           |         Dma_Ctrl[16:0]      |
	 	+--------------------------------------------------------------+
	 	|                         Src_Addr[31:0]                       |
	 	+--------------------------------------------------------------+
	 	|                         Dest_Addr[31:0]                      |
	 	+--------------------------------------------------------------+
	 	|                       Next_Desc_Add[31:0]                    |
	 	+--------------------------------------------------------------+
-------------------------------------------------------------------------*/
unsigned char tlsDmaStart(unsigned char ch, PTLS_DMA_DESCRIPTOR pDmaDesc, unsigned char autoReload)
{
	if((ch > 7) && !pDmaDesc) return 0;

	DMA_SRCADDR_REG(ch) = pDmaDesc->src_addr;
	DMA_DESTADDR_REG(ch) = pDmaDesc->dest_addr;
	DMA_CTRL_REG(ch) = ((pDmaDesc->dma_ctrl & 0x1ffff) << 1) | (autoReload ? 0x1: 0x0);
	DMA_CHNLCTRL_REG(ch) |= DMA_CHNL_CTRL_CHNL_ON;

	return 1;
}

/*   tlsDmaStop   */
/*-------------------------------------------------------------------------
	Description:	
		To stop current DMA channel transfer.
	Arguments:
		ch: Channel no. to be stopped.	
	Return Value:
		Always STATUS_SUCCESS.
	Note:	
		If channel stop, DMA_CHNL_CTRL_CHNL_ON bit in DMA_CHNLCTRL_REG is cleared.
-------------------------------------------------------------------------*/
unsigned char tlsDmaStop(unsigned char ch)
{
	if(ch > 7) return 0;
	if(DMA_CHNLCTRL_REG(ch) & DMA_CHNL_CTRL_CHNL_ON)
	{
		DMA_CHNLCTRL_REG(ch) |= DMA_CHNL_CTRL_CHNL_OFF;

		while(DMA_CHNLCTRL_REG(ch) & DMA_CHNL_CTRL_CHNL_ON);
	}

	return 1;
}

/*   tlsDmaRequest   */
/*-------------------------------------------------------------------------
	Description:	
		Request a free dma channel.
		If ch is 0, the function will select a random free channel,
		else return the selected channel no. if free.
	Arguments:
		ch: Channel no.
		flags	: Flags setted to selected channel.
	Return Value:
		Channel no. that is free now.
	Note:	
-------------------------------------------------------------------------*/
unsigned char tlsDmaRequest(unsigned char ch, unsigned char flags)
{

	unsigned char freeCh = 0;

#if 0
 	int i = 0;
	if(ch == 0)
	{
		for(i = 0; i < 8; i++)
			if(!(channels.channels[i] & TLS_DMA_FLAGS_CHANNEL_VALID)) {freeCh = i + 1;}
	}
	else if((ch >0) && (ch < 8))
#endif	
	if(ch < 8)
	{
		if(!(channels.channels[ch] & TLS_DMA_FLAGS_CHANNEL_VALID)) {freeCh = ch;}
	}
	else
	{
		printf("!!!there is no  free DMA channel.!!!\n");
		freeCh = 0;
	}

	//if(freeCh != 0)
	{
		channels.channels[freeCh] = flags | TLS_DMA_FLAGS_CHANNEL_VALID;
		DMA_MODE_REG(freeCh) = flags;
	}

	return freeCh;
}

/*   tlsDmaFree   */
/*-------------------------------------------------------------------------
	Description:	
		Free the DMA channel when not use.
	Arguments:
		ch: Channel no. that is ready to free.
	Return Value:
		None
	Note:	
-------------------------------------------------------------------------*/
void tlsDmaFree(unsigned char ch)
{
	if(ch < 8)
	{
		tlsDmaStop(ch);
		
		DMA_SRCADDR_REG(ch) = 0;
		DMA_DESTADDR_REG(ch) = 0;
		DMA_MODE_REG(ch) = 0;
		DMA_CTRL_REG(ch) = 0;
//		DMA_INTSRC_REG = 0xffff;
		DMA_INTSRC_REG |= 0x03<<(ch*2);
		
		channels.channels[ch] = 0x00;
	}
}

/*   tlsDmaInit   */
/*-------------------------------------------------------------------------
	Description:	
		Initialize DMA Control.
		Disable all interrupt.
	Arguments:
		None.
	Return Value:
		None.
	Note:	
-------------------------------------------------------------------------*/
void tlsDmaInit(void)
{
	DMA_INTMASK_REG = 0xffff;
	DMA_INTSRC_REG = 0xffff;
}

