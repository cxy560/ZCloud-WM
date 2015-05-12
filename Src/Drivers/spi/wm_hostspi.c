/***************************************************************************** 
* 
* File Name : wm_hostspi.c 
* 
* Description: host spi Driver Module 
* 
* Copyright (c) 2014 Winner Microelectronics Co., Ltd. 
* All rights reserved. 
* 
* Author : dave
* 
* Date : 2014-6-6
*****************************************************************************/ 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "wm_regs.h"
#include "wm_irq.h"
#include "wm_hostspi.h"
#include "wm_gpio.h"
#include "wm_dma.h"
#include "wm_debug.h"
#include "wm_mem.h"
#include "wm_cpu.h"
#include "wm_spi_hal.h"
static struct tls_spi_port *spi_port = NULL;

#define MSG_QUEUE_SIZE      (8)
static void *msg_queue[MSG_QUEUE_SIZE];

#define SPI_SCHEDULER_STK_SIZE      (128)
static u32 spi_scheduler_stk[SPI_SCHEDULER_STK_SIZE];

#define SPI_SCHED_MSG_START_ENGINE      (1)
#define SPI_SCHED_MSG_TX_FIFO_READY      (2)
#define SPI_SCHED_MSG_RX_FIFO_READY      (3)
#define SPI_SCHED_MSG_TRANSFER_COMPLETE      (4)
#define SPI_SCHED_MSG_EXIT      (5)
#define SPI_SCHED_MSG_END      (6)
static void spi_start_transfer(u32 transfer_bytes);

int tls_spi_async(struct tls_spi_message *message);
int tls_spi_sync(struct tls_spi_message *message);

#ifdef SPI_USE_DMA
static void SpiMasterInit(u8 mode, u8 cs_active, u32 fclk)
{

	/* pin config */
	IOCTRL_PIN_CONFIG_REG3 = 0;//(IOCTRL_LSSPI_MS_DO_PIN | IOCTRL_LSSPI_MS_DI_PIN | IOCTRL_LSSPI_MS_CS_PIN | IOCTRL_LSSPI_MS_CLK_PIN);
	
	SPIM_CHCFG_REG = SPI_CLEAR_FIFOS;
	while(SPIM_CHCFG_REG & SPI_CLEAR_FIFOS);
	
	SPIM_CLKCFG_REG = SPI_SCLK_DIVIDER(SPI_GET_SCLK_DIVIDER(fclk));
	SPIM_SPICFG_REG = 0;
	SPIM_SPICFG_REG = SPI_FRAME_FORMAT_MOTO | SPI_SET_MASTER_SLAVE(SPI_MASTER) | mode;/* 0x0004 - FRAM FORMAT : motorola 00 | Master | Rising | IDL LOW */
	SPIM_INTEN_REG = 0xff;/* Disable INT */

	tlsDmaInit();
}

 int spiWaitIdle(void)
{	
	unsigned long regVal;	
	unsigned long timeout=0;	

	do
	{
		timeout ++;
		if(timeout > 0x4FFFFF)		//5s	
			return TLS_SPI_STATUS_EBUSY;
		regVal = SPIM_SPISTATUS_REG;
	}while(regVal & (1<<12));

	return TLS_SPI_STATUS_OK;
}

static int SpiDmaBlockWrite(u8 *data,u32 len,u8 ifusecmd,u32 cmd)
{
	unsigned char dmaCh = 0;
	TLS_DMA_DESCRIPTOR DmaDesc;
	u32 txlen,txlenbk;
	u32 i,blocknum,blocksize = 0;
	int ret = TLS_SPI_STATUS_OK;
	int txcmdover = 0;

	if(NULL == data)
	{
		return TLS_SPI_STATUS_EINVAL;
	}
	if(spiWaitIdle())
		return  TLS_SPI_STATUS_EBUSY;
 	SPIM_CHCFG_REG = SPI_CLEAR_FIFOS;
	while(SPIM_CHCFG_REG & SPI_CLEAR_FIFOS);
	
	if(ifusecmd)
		SPIM_TXDATA_REG = cmd;

	if(len%4)
	{
		//txlen = ((len+3)/4)<<2;
		txlen = len & 0xfffffffc;	//不够字的最后单独发
	}
	else
	{
		txlen = len;
	}
	//printf("\nblock write cmd=%x,len=%d,txlen=%d\n",cmd,len,txlen);
	txlenbk = txlen;
	if(txlen > 0){
	blocknum = txlen/SPI_DMA_MAX_TRANS_SIZE;

	/* Request DMA Channel */
	dmaCh = tlsDmaRequest(1, TLS_DMA_FLAGS_CHANNEL_SEL(TLS_DMA_SEL_LSSPI_TX) | TLS_DMA_FLAGS_HARD_MODE);
	for(i = 0;i <= blocknum; i ++)
	{
	//	printf("\ni=%d\n",i);
		DmaDesc.src_addr = (int)(data + i*SPI_DMA_MAX_TRANS_SIZE);
		DmaDesc.dest_addr = HR_SPI_TXDATA_REG;
		blocksize = (txlen > SPI_DMA_MAX_TRANS_SIZE)? SPI_DMA_MAX_TRANS_SIZE:txlen;
	//	printf("\nblocksize=%d\n",blocksize);
		if(0 == blocksize)
			break;
		DmaDesc.dma_ctrl = TLS_DMA_DESC_CTRL_SRC_ADD_INC | TLS_DMA_DESC_CTRL_DATA_SIZE_WORD | (blocksize << 5);
		DmaDesc.valid = TLS_DMA_DESC_VALID;
		DmaDesc.next = NULL;
		tlsDmaStart(dmaCh, &DmaDesc, 0);

		SPIM_MODECFG_REG = SPI_RX_TRIGGER_LEVEL(0) | SPI_TX_TRIGGER_LEVEL(0) | SPI_TX_DMA_ON;/* Enable SPI TX DMA */
		SPIM_SPITIMEOUT_REG = SPI_TIMER_EN |SPI_TIME_OUT((u32)0xffff);/* spi_timer_en | 0xffff */
		if(ifusecmd && 0 == i)
		{
			SPIM_CHCFG_REG = SPI_FORCE_SPI_CS_OUT | SPI_TX_CHANNEL_ON | SPI_CONTINUE_MODE | SPI_START | SPI_VALID_CLKS_NUM(((blocksize + 4) * 8));
			txcmdover = 1;
		}
		else
			SPIM_CHCFG_REG = SPI_FORCE_SPI_CS_OUT | SPI_TX_CHANNEL_ON | SPI_CONTINUE_MODE | SPI_START | SPI_VALID_CLKS_NUM((blocksize * 8));

		if(spiWaitIdle())
		{
			ret = TLS_SPI_STATUS_EBUSY;
			break;
		}
		/* Wait Dma Channel Complete and Free Dma channel */
		if(tlsDmaWaitComplt(dmaCh))
		{
			ret = TLS_SPI_STATUS_EBUSY;
			break;
		}
		txlen -= blocksize;
	//	printf("\ntxlen=%d\n",txlen);
	}
	tlsDmaFree(dmaCh);
	}
	//tx 不够一个整字的几个字节
	if(len > txlenbk)
	{
		u32 word32 = 0;
		int temp = 0;
		for(i = 0;i < (len - txlenbk);i ++)
		{
			word32 |= (data[txlenbk+i]<<(i*8));		
		}
		//printf("\nword32==%x\n",word32);
		SPIM_TXDATA_REG = word32;
		SPIM_MODECFG_REG = SPI_RX_TRIGGER_LEVEL(0) | SPI_TX_TRIGGER_LEVEL(0);
		SPIM_SPITIMEOUT_REG = SPI_TIMER_EN |SPI_TIME_OUT((u32)0xffff);/* spi_timer_en | 0xffff */
		if(ifusecmd && 0 == txcmdover)	//需要发送命令，但是命令还没有发送出去，发送的字节数需要增加4
			temp = 4;
		SPIM_CHCFG_REG = SPI_FORCE_SPI_CS_OUT | SPI_TX_CHANNEL_ON | SPI_CONTINUE_MODE | SPI_START | SPI_VALID_CLKS_NUM(((temp + len - txlenbk) * 8));
		if(spiWaitIdle())
		{
			ret = TLS_SPI_STATUS_EBUSY;
		}		
	}
	SPIM_CHCFG_REG = 0x00000000;
	SPIM_MODECFG_REG = 0x00000000;
	SPIM_SPITIMEOUT_REG = 0x00000000;
//	printf("\nwrite block over\n");
	return ret;
}

static int SpiDmaBlockRead(u8 *data,u32 len, u8 *txdata, u8 txlen)
{
	unsigned char dmaCh = 0;
	TLS_DMA_DESCRIPTOR DmaDesc;
	u32 word32 = 0;
	u32 i;
	u32 rxlen,rxlenbk;
	u8 blocknum;
	u32 blocksize = 0;
	int ret = TLS_SPI_STATUS_OK;
	
	//printf("\nentry SpiDmaBlockRead\n");
	if( NULL == data )
	{
		return TLS_SPI_STATUS_EINVAL;
	}
	if(spiWaitIdle())
	{
		return  TLS_SPI_STATUS_EBUSY;
	}

	SPIM_CHCFG_REG = SPI_CLEAR_FIFOS;
	while(SPIM_CHCFG_REG & SPI_CLEAR_FIFOS);
	//printf("\nblock read len = %d,txlen=%d,txdata=%x\n",len,txlen,*(u32 *)txdata);

	if(len%4)
	{
		//rxlen = ((len+3)/4)<<2;
		rxlen = len&0xfffffffc;	//不够字的最后单独取
	}
	else
	{
		rxlen = len;
	}
	rxlenbk = rxlen;
	blocknum = rxlen/SPI_DMA_MAX_TRANS_SIZE;
//	printf("\nlen = %d,rxlen=%d,blocknum=%d\n",len,rxlen,blocknum);
	if(txlen > 0 && txlen <= 32)
	{
		for(i = 0;i < txlen;i ++)
		{
			if((i > 0) && (0 == i%4))
			{
				SPIM_TXDATA_REG = word32;
				word32 = 0;
			}
			word32 |= (txdata[i]<<((i%4)*8));
		}
		SPIM_TXDATA_REG = word32;
	}

	/* Request DMA Channel */
	dmaCh = tlsDmaRequest(1, TLS_DMA_FLAGS_CHANNEL_SEL(TLS_DMA_SEL_LSSPI_RX) | TLS_DMA_FLAGS_HARD_MODE);
	DmaDesc.src_addr = HR_SPI_RXDATA_REG;
	for(i = 0;i <= blocknum; i ++)
	{
		if(rxlenbk > 0)
		{	//说明接收的数据大于4
		//	printf("\ni =%d\n",i);
			DmaDesc.dest_addr = (int)(data + i*SPI_DMA_MAX_TRANS_SIZE);
			blocksize = (rxlen > SPI_DMA_MAX_TRANS_SIZE) ? SPI_DMA_MAX_TRANS_SIZE:rxlen;
			if(0 == blocksize)
				break;
		//	printf("\nblocksize= %d\n",blocksize);
			DmaDesc.dma_ctrl = TLS_DMA_DESC_CTRL_DEST_ADD_INC | TLS_DMA_DESC_CTRL_BURST_SIZE1 | TLS_DMA_DESC_CTRL_DATA_SIZE_WORD | TLS_DMA_DESC_CTRL_TOTAL_BYTES(blocksize);
			//	word32 = DmaDesc.dma_ctrl;
			//	printf("\ndma ctrl = %x\n",DmaDesc.dma_ctrl);
			DmaDesc.valid = TLS_DMA_DESC_VALID;
			DmaDesc.next = NULL;
			tlsDmaStart(dmaCh, &DmaDesc, 0);
			SPIM_MODECFG_REG = SPI_RX_TRIGGER_LEVEL(0) | SPI_TX_TRIGGER_LEVEL(0) | SPI_RX_DMA_ON;/* Enable SPI RX DMA */
		}else
		{
			SPIM_MODECFG_REG = SPI_RX_TRIGGER_LEVEL(0) | SPI_TX_TRIGGER_LEVEL(0) ;//rx数据少于4个byte，不用开DMA
		}
		SPIM_SPITIMEOUT_REG = SPI_TIMER_EN |SPI_TIME_OUT((u32)0xffff);/* spi_timer_en | 0xffff */
		if(0 == blocknum)
		{
			SPIM_CHCFG_REG = SPI_FORCE_SPI_CS_OUT | SPI_RX_CHANNEL_ON | SPI_TX_CHANNEL_ON | SPI_CONTINUE_MODE | SPI_START | SPI_VALID_CLKS_NUM(((len + txlen) * 8)) | SPI_RX_INVALID_BITS(txlen*8);
		}
		else
		{
			if(0 == i)		//第一次需要打开TX
			{
				SPIM_CHCFG_REG = SPI_FORCE_SPI_CS_OUT | SPI_RX_CHANNEL_ON | SPI_TX_CHANNEL_ON | SPI_CONTINUE_MODE | SPI_START | SPI_VALID_CLKS_NUM(((blocksize + txlen) * 8)) | SPI_RX_INVALID_BITS(txlen*8);
			}
			else
				if(i == blocknum)
				{
					SPIM_CHCFG_REG = SPI_FORCE_SPI_CS_OUT | SPI_RX_CHANNEL_ON | SPI_CONTINUE_MODE | SPI_START | SPI_VALID_CLKS_NUM((blocksize + len - rxlenbk)* 8);
				}
				else
				{
					SPIM_CHCFG_REG = SPI_FORCE_SPI_CS_OUT | SPI_RX_CHANNEL_ON | SPI_CONTINUE_MODE | SPI_START | SPI_VALID_CLKS_NUM(blocksize* 8);
				}
		}
		if(spiWaitIdle())
		{
			ret = TLS_SPI_STATUS_EBUSY;
			break;
		}

		/* Wait Dma Channel Complete and Free Dma channel */
		if(tlsDmaWaitComplt(dmaCh))
		{
			ret = TLS_SPI_STATUS_EBUSY;
			break;
		}
		rxlen -= blocksize;
	//	printf("\nrxlen = %d\n",rxlen);
	}
	tlsDmaFree(dmaCh);
	//printf("\nret===%x\n",ret);

	if(len > rxlenbk)		//取最后的不够一个字的几个byte
	{
		word32 = SPIM_RXDATA_REG;
		//printf("\nword32==%x\n",word32);
		*((int *)data + rxlenbk/4) = word32;
	}
	SPIM_CHCFG_REG = 0x00000000;
	SPIM_MODECFG_REG = 0x00000000;
	SPIM_SPITIMEOUT_REG = 0x00000000;
	return ret;
}
#endif
/**********************************************************************************************************
* Description: 	This function is used to set SPI transfer mode.
*
* Arguments  : 	type					is the transfer type.
*				type == SPI_BYTE_TRANSFER	byte transfer
*				type == SPI_WORD_TRANSFER	word transfer
*				type == SPI_DMA_TRANSFER	DMA transfer
* Returns    :
* Notes	    :		This is an empty function.the system use DMA transfer default.
**********************************************************************************************************/
void tls_spi_trans_type(u8 type)
{

	spi_port->transtype = type;
	if(SPI_WORD_TRANSFER == type)
	{
		spi_set_endian(0);
	}
	else if(SPI_DMA_TRANSFER == type)
	{
#ifdef SPI_USE_DMA	
		SpiMasterInit(spi_port->mode, TLS_SPI_CS_LOW, spi_port->speed_hz);
#endif
	}
}
static void spi_message_init(struct tls_spi_message *m)
{
	memset(m, 0, sizeof(*m));
	dl_list_init(&m->transfers);
}


static void spi_complete(void *arg)
{
	tls_os_sem_t *sem;

	sem = (tls_os_sem_t *)arg;
	tls_os_sem_release(sem);
}

static u32 spi_fill_txfifo(struct tls_spi_transfer *current_transfer, u32 current_remaining_bytes)
{
	u8 fifo_level;
	u16 rw_words;
	u16 rw_bytes;
	u8 data8;
	u8 i;
	u32 data32 = 0;
	u32 tx_remaining_bytes;
	if ((current_transfer == NULL) || (current_remaining_bytes == 0)) return 0;

	tx_remaining_bytes =  current_remaining_bytes;

	//printf("ready to write to fifo size - %d.\n", tx_remaining_bytes);
	spi_get_status(NULL,	NULL, &fifo_level);
	
	//TLS_DBGPRT_SPI("\nfifo_level 0= %d\n",fifo_level);
	rw_words = ((fifo_level > tx_remaining_bytes) ? tx_remaining_bytes : fifo_level)/4;
	rw_bytes = ((fifo_level > tx_remaining_bytes) ? tx_remaining_bytes : fifo_level)%4;

	//TLS_DBGPRT_SPI("write to spi fifo words - %d, bytes - %d.\n", rw_words, rw_bytes);

//下面代码17us
	for (i = 0; i < rw_words; i++) {
		if (current_transfer->tx_buf) {
			if(SPI_BYTE_TRANSFER == spi_port->transtype)
			{
				data32 = 0;
				data32 |= ((u8 *)current_transfer->tx_buf + (current_transfer->len - tx_remaining_bytes))[0] << 24;
				data32 |= ((u8 *)current_transfer->tx_buf + (current_transfer->len - tx_remaining_bytes))[1] << 16;
				data32 |= ((u8 *)current_transfer->tx_buf + (current_transfer->len - tx_remaining_bytes))[2] << 8;
				data32 |= ((u8 *)current_transfer->tx_buf + (current_transfer->len - tx_remaining_bytes))[3] << 0;
			}
			else if(SPI_WORD_TRANSFER == spi_port->transtype)
			{
				//TLS_DBGPRT_SPI("\n[%x][%x]\n",*((u8 *)current_transfer->tx_buf),*((u8 *)(current_transfer->tx_buf+1)));
				data32 = *((u32*)((u8 *)current_transfer->tx_buf + current_transfer->len - tx_remaining_bytes));
			}
		}
		else {data32 = 0xffffffff;}
		//TLS_DBGPRT_SPI("write to spi fifo word[%d]: 0x%x.\n", i, data32);
		spi_data_put(data32);
		tx_remaining_bytes -= 4;
	}

	if (rw_bytes) {
		data32 = 0;
		for (i = 0; i < rw_bytes; i++) {
			if (current_transfer->tx_buf) {data8 = ((u8 *)current_transfer->tx_buf)[current_transfer->len - tx_remaining_bytes];}
			else {data8 = 0xff;}
			if(SPI_BYTE_TRANSFER == spi_port->transtype)
				data32 |= data8<<((3 - i)*8);
			else if(SPI_WORD_TRANSFER == spi_port->transtype)
				{
				data32 |= data8<<(i*8);
				}
			tx_remaining_bytes -= 1;
		}
		//	TLS_DBGPRT_SPI("write to spi fifo word: 0x%x.\n", data32);
			spi_data_put(data32);
	}

	return (current_remaining_bytes - tx_remaining_bytes);
}

static u32 spi_get_rxfifo(struct tls_spi_transfer *current_transfer, u32 current_remaining_bytes)
{
	u8 fifo_level;
	u8 rw_words;
	u8 rw_bytes;
	u8 data8 = 0;
	u8 i;
	u32 data32;
	u32 rx_remaining_bytes;

	if ((current_transfer == NULL) || (current_remaining_bytes == 0)) return 0;

	rx_remaining_bytes = current_remaining_bytes;
	spi_get_status(NULL, &fifo_level, NULL);
//	TLS_DBGPRT_SPI("rx fifo level: %d.\n", fifo_level);

	rw_words = fifo_level/4;
	rw_bytes = fifo_level%4;

	//TLS_DBGPRT_SPI("rx data: %d words, %d bytes.\n", rw_words, rw_bytes);

//下面代码大概10us	
	for (i = 0; i < rw_words; i++) {
		
		data32 = spi_data_get();
		
		//TLS_DBGPRT_SPI("rx data[%d](w): 0x%08x.\n", i, data32);
		if (current_transfer->rx_buf) {

			if(SPI_BYTE_TRANSFER == spi_port->transtype)
			{
			data32 = swap_32(data32);
			(((u8 *)current_transfer->rx_buf + (current_transfer->len - rx_remaining_bytes)))[0] = (u8)data32;
			(((u8 *)current_transfer->rx_buf + (current_transfer->len - rx_remaining_bytes)))[1] = (u8)(data32>>8);
			(((u8 *)current_transfer->rx_buf + (current_transfer->len - rx_remaining_bytes)))[2] = (u8)(data32>>16);
			(((u8 *)current_transfer->rx_buf + (current_transfer->len - rx_remaining_bytes)))[3] = (u8)(data32>>24);
			}
			else if(SPI_WORD_TRANSFER == spi_port->transtype)
			{
			*((u32 *)((u8 *)current_transfer->rx_buf + current_transfer->len - rx_remaining_bytes)) = data32;
			}
		}
		rx_remaining_bytes -= 4;
	
	}
	
	if (rw_bytes) {
		data32 = spi_data_get();
		//TLS_DBGPRT_SPI("\nrx data=%x\n",data32);
		if (current_transfer->rx_buf) {
			for (i = 0; i < rw_bytes; i++) {
				if(SPI_BYTE_TRANSFER == spi_port->transtype)
					data8 = (u8)(data32>>((3 - i)*8));
				else if(SPI_WORD_TRANSFER == spi_port->transtype)
					data8 = (u8)(data32>>(i*8));
	
	//			TLS_DBGPRT_SPI("rx data[%d](b): 0x%02x.\n", i, data8);
				((u8 *)current_transfer->rx_buf)[current_transfer->len - rx_remaining_bytes] = data8;
				rx_remaining_bytes -= 1;
			}
		} else {rx_remaining_bytes -= rw_bytes;}
	}

	return (current_remaining_bytes - rx_remaining_bytes);
}

static struct tls_spi_transfer * spi_next_transfer(struct tls_spi_message *current_message)
{
	if (current_message == NULL) {return NULL;}

	return dl_list_first(&current_message->transfers, struct tls_spi_transfer, transfer_list);
}

static struct tls_spi_message * spi_next_message(void)
{
	struct tls_spi_message *current_message;

	current_message = dl_list_first(&spi_port->wait_queue, struct tls_spi_message, queue);
	if (current_message == NULL) {return NULL;}

	spi_port->current_transfer = spi_next_transfer(current_message);
	current_message->status = SPI_MESSAGE_STATUS_INPROGRESS;

	return current_message;
}

static void spi_start_transfer(u32 transfer_bytes)
{
	if (spi_port->reconfig) {
		TLS_DBGPRT_SPI("reconfig the spi master controller.\n");
		spi_set_mode(spi_port->mode);
		spi_set_chipselect_mode(spi_port->cs_active);
		spi_set_sclk(spi_port->speed_hz);

		spi_port->reconfig = 0;
	}

	spi_set_sclk_length(transfer_bytes * 8, 0);

	spi_set_chipselect_mode(SPI_CS_ACTIVE_MODE);
	spi_sclk_start();
}
static void spi_continue_transfer(void)
{
	struct tls_spi_message *current_message;
	struct tls_spi_transfer *current_transfer;
	u32 transfer_bytes;
	
	current_message = spi_port->current_message;
	current_transfer = spi_port->current_transfer;
	if((current_message == NULL) || (current_transfer == NULL)) 
		{return;}
	transfer_bytes = spi_get_rxfifo(current_transfer, spi_port->current_remaining_bytes);

	spi_port->current_remaining_bytes -= transfer_bytes;
	if (spi_port->current_remaining_bytes == 0) {
		tls_os_sem_acquire(spi_port->lock, 0);
				
		dl_list_del(&current_transfer->transfer_list);
		spi_port->current_transfer = spi_next_transfer(spi_port->current_message);
		if (spi_port->current_transfer == NULL) {
			tls_sys_clk_set(0);
			//TLS_DBGPRT_SPI("current spi transaction message finish and switch to the next transaction message.\n");
			spi_set_chipselect_mode(SPI_CS_INACTIVE_MODE);
			current_message->status = SPI_MESSAGE_STATUS_DONE;
			dl_list_del(&current_message->queue);
			spi_port->current_message = spi_next_message();
		}
						
		tls_os_sem_release(spi_port->lock);

		//TLS_DBGPRT_SPI("get the next spi transfer pair.\n");
		current_transfer = spi_port->current_transfer;
		if(current_transfer != NULL) {spi_port->current_remaining_bytes = current_transfer->len;}
	}

	transfer_bytes = spi_fill_txfifo(current_transfer, spi_port->current_remaining_bytes);

	if (transfer_bytes) {
		spi_start_transfer(transfer_bytes);
	}

	if (current_message->status == SPI_MESSAGE_STATUS_DONE) {
		//TLS_DBGPRT_SPI("current spi transaction finish and notify the submitter.\n");
		current_message->complete(current_message->context);
	}
}

static void spi_scheduler(void *data)
{
	u8 err;
	u32 msg;
	u32 transfer_bytes;
	struct tls_spi_transfer *current_transfer;

	while(1) {
		err = tls_os_queue_receive(spi_port->msg_queue, (void * *)&msg, 4, 0);
		if (err == TLS_OS_SUCCESS) {
			switch (msg) {
				case SPI_SCHED_MSG_START_ENGINE:
					tls_sys_clk_set(2);
					if (spi_port->current_message) {
						TLS_DBGPRT_WARNING("spi transaction scheduler is running now!\n");
						break;
					}

					TLS_DBGPRT_SPI("acquire the first transaction message in waiting queue.\n");
					tls_os_sem_acquire(spi_port->lock, 0);
					
					spi_port->current_message = spi_next_message();

					tls_os_sem_release(spi_port->lock);

					//TLS_DBGPRT_SPI("acquire the first transfer pair in the current transaction message.\n");				
					current_transfer = spi_port->current_transfer;
					if(current_transfer == NULL) {break;}
					spi_port->current_remaining_bytes = current_transfer->len;

					//TLS_DBGPRT_SPI("current transfer lenght - %d.\n", spi_port->current_remaining_bytes);

					//TLS_DBGPRT_SPI("fill the tx fifo.\n");
					
					transfer_bytes = spi_fill_txfifo(current_transfer, spi_port->current_remaining_bytes);
					
				//	printf("start the spi transfer - %d.\n", transfer_bytes);
					spi_start_transfer(transfer_bytes);

					break;
					
				case SPI_SCHED_MSG_TX_FIFO_READY:
					TLS_DBGPRT_SPI("process SPI_SCHED_MSG_TX_FIFO_READY.\n");
					break;

				case SPI_SCHED_MSG_RX_FIFO_READY:
					TLS_DBGPRT_SPI("process SPI_SCHED_MSG_RX_FIFO_READY.\n");
					break;
					
				case SPI_SCHED_MSG_TRANSFER_COMPLETE:
					spi_continue_transfer();
					break;
					
				case SPI_SCHED_MSG_EXIT:
					break;

				default:
					break;
			}
		}
	}
}

static void spi_interrupt_handler(void *arg)
{
	u32 int_status;
	u32 int_mask;
	arg = arg;
	//printf("\nspi int\n");
	int_status = spi_get_int_status();
	//printf("\nspi int sta=%x\n",int_status);
	spi_clear_int_status(int_status);

	int_mask = spi_int_mask();
	int_status &= ~int_mask;

	//printf("spi interrupt - 0x%x.\n", int_status);

	if (int_status & SPI_INT_TX_FIFO_RDY) {
		tls_os_queue_send(spi_port->msg_queue, (void *)SPI_SCHED_MSG_TX_FIFO_READY, 4);
	}

	if (int_status & SPI_INT_RX_FIFO_RDY) {
		tls_os_queue_send(spi_port->msg_queue, (void *)SPI_SCHED_MSG_RX_FIFO_READY, 4);
	}

	if (int_status & SPI_INT_TRANSFER_DONE) {
		if(SPI_WORD_TRANSFER == spi_port->transtype)
			spi_continue_transfer();
		else
			tls_os_queue_send(spi_port->msg_queue, (void *)SPI_SCHED_MSG_TRANSFER_COMPLETE, 4);
		
	}
}

/**
 * @brief 
 *
 * @param mode
 * @param cs_active
 * @param fclk
 *
 * @return 
 */
int tls_spi_setup(u8 mode, u8 cs_active, u32 fclk)
{
	if ((spi_port->mode == mode) && (spi_port->cs_active == cs_active) && (spi_port->speed_hz == fclk)) {
		TLS_DBGPRT_WARNING("@mode, @cs_activer, @fclk is the same as settings of the current spi master driver!\n");
		return TLS_SPI_STATUS_OK;
	}

	switch (mode) {
		case TLS_SPI_MODE_0:		
		case TLS_SPI_MODE_1:
		case TLS_SPI_MODE_2:	
		case TLS_SPI_MODE_3:
			spi_port->mode = mode;
			break;

		default:
			TLS_DBGPRT_ERR("@mode is invalid!\n");
			return TLS_SPI_STATUS_EMODENOSUPPORT;
	}

	if((cs_active != TLS_SPI_CS_HIGH) && (cs_active != TLS_SPI_CS_LOW)) {
		TLS_DBGPRT_ERR("@cs_active  is invalid!\n");
		return TLS_SPI_STATUS_EINVAL;
	} else {spi_port->cs_active = cs_active;}

	if((fclk < TLS_SPI_FCLK_MIN) || (fclk > TLS_SPI_FCLK_MAX)) {
		TLS_DBGPRT_ERR("@fclk  is invalid!\n");
		return TLS_SPI_STATUS_ECLKNOSUPPORT;
	} else 
	{
		spi_port->speed_hz = fclk;
	}

#ifdef SPI_USE_DMA
	if(SPI_DMA_TRANSFER == spi_port->transtype)
	{
		SpiMasterInit(mode, TLS_SPI_CS_LOW, fclk);
		return TLS_SPI_STATUS_OK;
	}
#endif

	spi_port->reconfig = 1;

	return TLS_SPI_STATUS_OK;
}

/**
 * @brief 
 *
 * @param txbuf
 * @param n_tx
 * @param rxbuf
 * @param n_rx
 *
 * @return 
 */
int tls_spi_write_then_read(const u8 *txbuf, u32 n_tx, u8 *rxbuf, u32 n_rx)
{
	int status;
	struct tls_spi_message message;
	struct tls_spi_transfer x[2];

	if ((txbuf == NULL) || (n_tx == 0) || (rxbuf == NULL) || (n_rx == 0)) {return TLS_SPI_STATUS_EINVAL;}
	
#ifdef SPI_USE_DMA
	if(SPI_DMA_TRANSFER == spi_port->transtype)
	{
		if(n_rx > SPI_DMA_BUF_MAX_SIZE || n_tx > SPI_DMA_CMD_MAX_SIZE)
		{
			TLS_DBGPRT_ERR("\nread length too long\n");
			return TLS_SPI_STATUS_EINVAL;
		}
		tls_os_sem_acquire(spi_port->lock, 0);
		MEMCPY((u8 *)SPI_DMA_CMD_ADDR, txbuf, n_tx);	
		SpiDmaBlockRead((u8 *)SPI_DMA_BUF_ADDR, n_rx, (u8 *)SPI_DMA_CMD_ADDR, n_tx);
		MEMCPY(rxbuf, (u8 *)SPI_DMA_BUF_ADDR, n_rx);
		tls_os_sem_release(spi_port->lock);
		return TLS_SPI_STATUS_OK;
	}
#endif

	spi_message_init(&message);
	
	memset(x, 0, sizeof(x));
	if (n_tx) {
		x[0].len = n_tx;
		x[0].tx_buf = txbuf;
		dl_list_add_tail(&message.transfers, &x[0].transfer_list);
	}
	if (n_rx) {
		x[1].len = n_rx;
		x[1].rx_buf = rxbuf;
		dl_list_add_tail(&message.transfers, &x[1].transfer_list);
	}

	/* do the i/o. */
	status = tls_spi_sync(&message);

	return status;
}

/**
 * @brief 
 *
 * @param buf
 * @param len
 *
 * @return 
 */
int tls_spi_read(u8 *buf, u32 len)
{
	struct tls_spi_transfer t;
	struct tls_spi_message m;

	if ((buf == NULL) || (len == 0)) {return TLS_SPI_STATUS_EINVAL;}

#ifdef SPI_USE_DMA
	if(SPI_DMA_TRANSFER == spi_port->transtype)
	{
		u32 data32 = 0;
		u16 rxBitLen;
		u32 rdval1;
		u32 i;
		tls_os_sem_acquire(spi_port->lock, 0);
		if(len <= 4)		//直接传输，这样做的原因是DMA不能连续读取4个字节以内的数据,SPI FIFO读取单位为word
		{
			SPIM_CHCFG_REG = SPI_CLEAR_FIFOS;
			while(SPIM_CHCFG_REG & SPI_CLEAR_FIFOS);

			rxBitLen = 8*len;
			rdval1 = 0;
			rdval1 |= SPI_FORCE_SPI_CS_OUT |SPI_CS_LOW |SPI_TX_CHANNEL_ON |SPI_RX_CHANNEL_ON | SPI_CONTINUE_MODE | SPI_START | SPI_VALID_CLKS_NUM(rxBitLen);		
			SPIM_CHCFG_REG = rdval1;	
			spiWaitIdle();
			SPIM_CHCFG_REG |= SPI_CS_HIGH;
		
			data32 = SPIM_RXDATA_REG;

			for(i = 0;i < len;i ++)
			{
				*(buf + i) = (u8)(data32>>i*8);
			}
			SPIM_CHCFG_REG = 0x00000000;	
			SPIM_MODECFG_REG = 0x00000000;
		}
		else				//DMA传输
		{
			if(len > SPI_DMA_BUF_MAX_SIZE)
			{
				TLS_DBGPRT_ERR("\nread len too long\n");
				tls_os_sem_release(spi_port->lock);
				return TLS_SPI_STATUS_EINVAL;
			}
			SpiDmaBlockRead((u8 *)SPI_DMA_BUF_ADDR, len, NULL, 0);
			MEMCPY(buf, (u8 *)SPI_DMA_BUF_ADDR, len);
		}
		tls_os_sem_release(spi_port->lock);
		return TLS_SPI_STATUS_OK;
	}
#endif

	memset(&t, 0, sizeof(t));
	t.rx_buf = buf;
	t.len = len;
	
	spi_message_init(&m);
	
	dl_list_add_tail(&m.transfers, &t.transfer_list);
	
	return tls_spi_sync(&m);
}

/**
 * @brief 
 *
 * @param buf
 * @param len
 *
 * @return 
 */
int tls_spi_write(const u8 *buf, u32 len)
{
	struct tls_spi_transfer t;
	struct tls_spi_message m;

	if ((buf == NULL) || (len == 0)) {return TLS_SPI_STATUS_EINVAL;}

#ifdef SPI_USE_DMA
	if(SPI_DMA_TRANSFER == spi_port->transtype)
	{
		u32 data32 = 0;
		u16 txBitLen;
		u32 rdval1;
		u32 i;
		tls_os_sem_acquire(spi_port->lock, 0);
		if(len <= 4)		//直接传输，这样做的原因是DMA不能连续传输少于4个字节的数据，SPI FIFO按字操作
		{
			SPIM_CHCFG_REG = SPI_CLEAR_FIFOS;
			while(SPIM_CHCFG_REG & SPI_CLEAR_FIFOS);
			for(i = 0;i < len;i ++)
			{
				data32 |= (((u8)(buf[i]))<<(i*8));
			}
			SPIM_TXDATA_REG  =  data32;
			txBitLen = 8*len;
			rdval1 = 0;
			rdval1 |= SPI_FORCE_SPI_CS_OUT |SPI_CS_LOW| SPI_TX_CHANNEL_ON | SPI_RX_CHANNEL_ON |SPI_CONTINUE_MODE | SPI_START | SPI_VALID_CLKS_NUM(txBitLen);
			SPIM_CHCFG_REG = rdval1;
			spiWaitIdle();
			SPIM_CHCFG_REG |= SPI_CS_HIGH;
		
			data32 = SPIM_RXDATA_REG;
			//printf("\nw data32 = %x\n",data32);
			SPIM_CHCFG_REG = 0x00000000;
			SPIM_MODECFG_REG = 0x00000000;
		}
		else				//DMA传输
		{
			if(len > SPI_DMA_BUF_MAX_SIZE)
			{
				TLS_DBGPRT_ERR("\nwrite len too long\n");
				tls_os_sem_release(spi_port->lock);
				return TLS_SPI_STATUS_EINVAL;
			}
			MEMCPY((u8 *)SPI_DMA_BUF_ADDR, buf, len);
			SpiDmaBlockWrite((u8 *)SPI_DMA_BUF_ADDR, len,0,0);
		}
		tls_os_sem_release(spi_port->lock);
		return 	TLS_SPI_STATUS_OK;
	}
	
#endif
	memset(&t, 0, sizeof(t));
	t.tx_buf = buf;
	t.len = len;
	
	spi_message_init(&m);
	
	dl_list_add_tail(&m.transfers, &t.transfer_list);
	
	return tls_spi_sync(&m);
}

/**********************************************************************************************************
* Description: 	This function is used to synchronous write 32bit command then write data by SPI.
*
* Arguments  : 	buf32					is the command data.
* 		txbuf					is the write data buffer.
*		n_tx					is the write data length.
* Returns    : 	TLS_SPI_STATUS_OK			if write success.
* 		TLS_SPI_STATUS_EINVAL			if argument is invalid.
* 		TLS_SPI_STATUS_ENOMEM			if there is no enough memory.
* 		TLS_SPI_STATUS_ESHUTDOWN		if SPI driver does not installed.
**********************************************************************************************************/
int tls_spi_write32_then_writen(const u32 *buf32, const u8 *txbuf, u32 n_tx)
{
	int status;
	struct tls_spi_message message;
	struct tls_spi_transfer x[2];

	if ((buf32 == NULL) || (txbuf == NULL) || (n_tx == 0)) {return TLS_SPI_STATUS_EINVAL;}

#ifdef SPI_USE_DMA
	if(SPI_DMA_TRANSFER == spi_port->transtype)
	{
		if(n_tx > SPI_DMA_BUF_MAX_SIZE)
		{
			TLS_DBGPRT_ERR("\nwriten len too long\n");
			return TLS_SPI_STATUS_EINVAL;
		}
		tls_os_sem_acquire(spi_port->lock, 0);
		MEMCPY((u8 *)SPI_DMA_CMD_ADDR, (u8 *)buf32, 4);
		MEMCPY((u8 *)SPI_DMA_BUF_ADDR, txbuf, n_tx);		
		SpiDmaBlockWrite((u8 *)SPI_DMA_BUF_ADDR, n_tx, 1, *(u32 *)SPI_DMA_CMD_ADDR);
		tls_os_sem_release(spi_port->lock);
		return TLS_SPI_STATUS_OK;
	}
#endif

	spi_message_init(&message);
	
	memset(x, 0, sizeof(x));

	x[0].len = 4;
	x[0].tx_buf = (const void *)buf32;
	dl_list_add_tail(&message.transfers, &x[0].transfer_list);

	x[1].len = n_tx;
	x[1].tx_buf = txbuf;
	dl_list_add_tail(&message.transfers, &x[1].transfer_list);


	/* do the i/o. */
	status = tls_spi_sync(&message);

	return status;
}

/**
 * @brief 
 *
 * @param message
 *
 * @return 
 */
int tls_spi_sync(struct tls_spi_message *message)
{
	int status;
	u8 err;
	tls_os_sem_t *sem;

	err = tls_os_sem_create(&sem, 0);
	if (err != TLS_OS_SUCCESS) {
		TLS_DBGPRT_ERR("create spi transaction synchronizing semaphore fail!\n");
		return TLS_SPI_STATUS_ENOMEM;
	}

	message->context = (void *)sem;
	message->complete = spi_complete;

	status = tls_spi_async(message);
	if (status == TLS_SPI_STATUS_OK) {
		TLS_DBGPRT_SPI("waiting spi transaction finishing!\n");
		tls_os_sem_acquire(sem, 0);
	}

	tls_os_sem_delete(sem);
	message->context = NULL;
	message->complete = NULL;

	return status;
}

/**
 * @brief 
 *
 * @param message
 *
 * @return 
 */
int tls_spi_async(struct tls_spi_message *message)
{
	u8 need_sched;
	struct tls_spi_transfer *transfer;
	
	if (spi_port == NULL) {
		TLS_DBGPRT_ERR("spi master driver module not beed installed!\n");
		return TLS_SPI_STATUS_ESHUTDOWN;
	}

	if ((message == NULL) || (dl_list_empty(&message->transfers))) {
		TLS_DBGPRT_ERR("@message is NULL or @message->transfers is empty!\n");
		return TLS_SPI_STATUS_EINVAL;
	}

	dl_list_for_each(transfer, &message->transfers, struct tls_spi_transfer, transfer_list) {
		if (transfer->len == 0) {
			TLS_DBGPRT_ERR("\"@transfer->len\" belong to @message is 0!\n");
			return TLS_SPI_STATUS_EINVAL;
		}
	}

	//TLS_DBGPRT_SPI("add @message to spi transaction message waiting queue.\n");
	tls_os_sem_acquire(spi_port->lock, 0);
	
	if (dl_list_empty(&spi_port->wait_queue)) {need_sched = 1;}
	else {need_sched = 0;}
	message->status = SPI_MESSAGE_STATUS_IDLE;
	dl_list_add_tail(&spi_port->wait_queue, &message->queue);

	tls_os_sem_release(spi_port->lock);

	if (need_sched == 1) {
		//TLS_DBGPRT_SPI("start spi transactions scheduler task.\n");
		tls_os_queue_send(spi_port->msg_queue, (void *)SPI_SCHED_MSG_START_ENGINE, 4);
	}
	
	return TLS_SPI_STATUS_OK;
}

/**********************************************************************************************************
* Description: 	This function is used to initialize the SPI master driver.
*
* Arguments  : 	
*
* Returns    : 	TLS_SPI_STATUS_OK			if initialize success
* 		TLS_SPI_STATUS_EBUSY			if SPI is already initialized
* 		TLS_SPI_STATUS_ENOMEM			if malloc SPI memory fail*		
**********************************************************************************************************/
 int tls_spi_init(void)
{
	u8 err;
	struct tls_spi_port *port;
	
	if(spi_port != NULL) {
		TLS_DBGPRT_ERR("spi driver module has been installed!\n");
		return TLS_SPI_STATUS_EBUSY;
	}

	TLS_DBGPRT_SPI("initialize spi master driver module.\n");

	port = (struct tls_spi_port *)tls_mem_alloc(sizeof(struct tls_spi_port));
	if (port == NULL) {
		TLS_DBGPRT_ERR("allocate \"struct tls_spi_port\" fail!\n");
		return TLS_SPI_STATUS_ENOMEM;
	}

	err = tls_os_sem_create(&port->lock, 1);
	if (err != TLS_OS_SUCCESS) {
		TLS_DBGPRT_ERR("create semaphore @spi_port->lock fail!\n");
		tls_mem_free(port);
		return TLS_SPI_STATUS_ENOMEM;
	}

	err = tls_os_queue_create(&port->msg_queue, (void *)&msg_queue[0], MSG_QUEUE_SIZE, 4);
	if (err != TLS_OS_SUCCESS) {
		TLS_DBGPRT_ERR("create message queue @spi_port->msg_queue fail!\n");
		tls_os_sem_delete(port->lock);
		tls_mem_free(port);
		return TLS_SPI_STATUS_ENOMEM;
	}

	err = tls_os_task_create(NULL, NULL,
					spi_scheduler,
					(void *)spi_port,
					(void *)&spi_scheduler_stk,
					SPI_SCHEDULER_STK_SIZE * sizeof(u32),
					TLS_SPI_SCHEDULER_TASK_PRIO,
					0);
	if (err != TLS_OS_SUCCESS) {
		TLS_DBGPRT_ERR("create spi master driver scheduler task fail!\n");
		tls_os_sem_delete(port->lock);
		tls_os_queue_delete(port->msg_queue);
		tls_mem_free(port);
		return TLS_SPI_STATUS_ENOMEM;
	}

	port->speed_hz = SPI_DEFAULT_SPEED;	/*默认2M*/
	port->cs_active = SPI_CS_ACTIVE_MODE;
	port->mode = SPI_DEFAULT_MODE;	/*CPHA = 0,CPOL = 0*/
	port->reconfig = 0;

	dl_list_init(&port->wait_queue);

	port->current_message = NULL;
	port->current_remaining_transfer = 0;
	port->current_transfer = NULL;
	port->current_remaining_bytes = 0;

	spi_port = port;
#if SPI_CS_AS_GPIO
	tls_gpio_cfg(SPI_CS_PIN, TLS_GPIO_DIR_OUTPUT, TLS_GPIO_ATTR_FLOATING);	//片选作为gpio使用
	tls_gpio_write(SPI_CS_PIN, 1);
#endif	
	tls_spi_trans_type(SPI_BYTE_TRANSFER);
	TLS_DBGPRT_SPI("initialize spi master controller.\n");

	{
		tls_reg_write32(HR_IOCTL_GP_SPI, 0);		/*设置复用IO为SPI模式*/
	}

	spi_clear_fifo();
	spi_set_endian(1);
	spi_set_mode(spi_port->mode);
	spi_set_chipselect_mode(SPI_CS_INACTIVE_MODE);		/*cs=1 ,片选无效*/
	spi_force_cs_out(1);		/*片选由软件控制*/
	spi_set_sclk(spi_port->speed_hz);

	spi_set_tx_trigger_level(0);
	spi_set_rx_trigger_level(7);

	spi_set_rx_channel(1);
	spi_set_tx_channel(1);
	spi_unmask_int(SPI_INT_TRANSFER_DONE /*| SPI_INT_RX_FIFO_RDY | SPI_INT_TX_FIFO_RDY*/);


	TLS_DBGPRT_SPI("register spi master interrupt handler.\n");
	tls_irq_register_handler(SPI0_INT, spi_interrupt_handler, (void *)spi_port);
	tls_irq_enable(SPI0_INT);


	TLS_DBGPRT_SPI("spi master driver module initialization finish.\n");

	return TLS_SPI_STATUS_OK;
}

/**
 * @brief 
 *
 * @return 
 */
int tls_spi_exit(void)
{
	TLS_DBGPRT_SPI("Not support spi master driver module uninstalled!\n");
	return TLS_SPI_STATUS_EPERM;
}



/**********************************************************************************************************
* Description: 	This function is used to select SPI slave type.
*
* Arguments  : 	slave					is the slave type,defined as follow:
* 							slave == SPI_SLAVE_FLASH	:flash
* 							slave == SPI_SLAVE_CARD		: sd card
*
* Returns    : 	Before communicate with different SPI device, must call the function.
**********************************************************************************************************/
void tls_spi_slave_sel(u16 slave)
{
	//u16 ret;
/*gpio0控制cs信号*/
	tls_gpio_cfg(SPI_SLAVE_CONTROL_PIN, TLS_GPIO_DIR_OUTPUT, TLS_GPIO_ATTR_FLOATING);
	if(SPI_SLAVE_FLASH == slave)
	{
		tls_gpio_write(SPI_SLAVE_CONTROL_PIN, 0);
	//	ret = tls_gpio_read(SPI_SLAVE_CONTROL_PIN);
	//	printf("\nflash gpio 0 ===%d\n",ret);
	}
	else if(SPI_SLAVE_CARD == slave)
	{
		tls_gpio_write(SPI_SLAVE_CONTROL_PIN, 1);
	//	ret = tls_gpio_read(SPI_SLAVE_CONTROL_PIN);
	//	printf("\ncard gpio 0 ===%d\n",ret);
	}
}

