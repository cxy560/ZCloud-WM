/***************************************************************************** 
* 
* File Name : wm_hostspi.h 
* 
* Description: host spi Driver Module 
* 
* Copyright (c) 2014 Winner Micro Electronic Design Co., Ltd. 
* All rights reserved. 
* 
* Author : dave 
* 
* Date : 2014-6-6
*****************************************************************************/ 
#ifndef WM_HOST_SPI_H
#define WM_HOST_SPI_H

#include "wm_type_def.h"
#include "list.h"
//#include "wm_os.h"
#include "wm_osal.h"

#define SPI_USE_DMA	

#define SPI_TX_DMA_MASK	0x01
#define SPI_RX_DMA_MASK	0x02

#define SPI_DMA_CMD_ADDR 	0X60000		//DMA访问地址必须在0x60000之后
#define SPI_DMA_BUF_ADDR	0x60020
#define SPI_DMA_CMD_MAX_SIZE (SPI_DMA_BUF_ADDR - SPI_DMA_CMD_ADDR)
#define SPI_DMA_BUF_MAX_SIZE	(0x61800 - SPI_DMA_BUF_ADDR)	/*61800之后的地址用户高速spi从接口使用*/

#define SPI_DMA_MAX_TRANS_SIZE	4092	//DMA一次最大搬移size

/**
 *  error code.
 */
#define TLS_SPI_STATUS_OK		(0)
#define TLS_SPI_STATUS_EINVAL		(-1)
#define TLS_SPI_STATUS_ENOMEM		(-2)
#define TLS_SPI_STATUS_EBUSY		(-3)
#define TLS_SPI_STATUS_ESHUTDOWN	(-4)
#define TLS_SPI_STATUS_EPERM		(-5)
#define TLS_SPI_STATUS_ECLKNOSUPPORT	(-6)
#define TLS_SPI_STATUS_EMODENOSUPPORT	(-7)

#define SPI_MASTER_FIFO_SIZE		(32)

/**
 *  the SPI master controller's configuration data.
 */
 /* configuration data. */
#define SPI_CPHA			(0x01) /* clock phase. */
#define SPI_CPOL			(0x02) /* clock polarity. */
#define TLS_SPI_MODE_0			(0|0) /* motorola mode. */
#define TLS_SPI_MODE_1			(0|SPI_CPHA)
#define TLS_SPI_MODE_2			(SPI_CPOL|0)
#define TLS_SPI_MODE_3      (SPI_CPOL|SPI_CPHA)
#define TLS_SPI_CS_LOW      0x00 /* chipselect active low. */
#define TLS_SPI_CS_HIGH      0x01 /* chipselect active high. */
#define TLS_SPI_FCLK_MIN      (1000) /* minimum work clock rate(Hz). */
#define TLS_SPI_FCLK_MAX      (APB_CLK/2) /* maximum work clock rate(Hz). */


/* default configuration data. */
#define SPI_DEFAULT_SPEED      (2000000) /* default clock rate is 2MHz. */
#define SPI_DEFAULT_MODE      (TLS_SPI_MODE_0) /* default mode MODE_0. */
#define SPI_CS_ACTIVE_MODE      (TLS_SPI_CS_LOW) /* default chipselect mode is active low. */
#define SPI_CS_INACTIVE_MODE      (TLS_SPI_CS_HIGH)

/* SPI transaction message status. */
#define SPI_MESSAGE_STATUS_IDLE      (0)
#define SPI_MESSAGE_STATUS_INPROGRESS      (1)
#define SPI_MESSAGE_STATUS_DONE      (2)

/*slave type*/
#define SPI_SLAVE_FLASH		0		/*本地flash*/
#define SPI_SLAVE_CARD		1		/*外扩SD卡*/
#define SPI_SLAVE_CONTROL_PIN		0
/*transfer type*/
#define SPI_BYTE_TRANSFER			0	/*不加速*/
#define SPI_WORD_TRANSFER			1	/*字传输加速*/
#define SPI_DMA_TRANSFER			2	/*DMA传输*/

#define SPI_CS_PIN					18
#define SPI_CS_AS_GPIO				1	//cs作为gpio使用
/**
 *  struct tls_spi_transfer - a read/write buffer pair
 *
 *  SPI transfers always write the same number of bytes as they read.
 *  If the transmit buffer is null, zeroes will be shifted out while 
 *  filling @rx_buf. If the receive buffer is null, the data shifted in 
 *  will be discarded.
 */
struct tls_spi_transfer {
	struct dl_list transfer_list; /* transfers are sequenced through @tls_spi_message.transfers. */
	
	const void *tx_buf; /* data to be written, or NULL. */
	void *rx_buf; /* data to be read, or NULL. */
	u32 len; /* size of rx and tx buffers (in bytes). */
	u32 delay_usecs; /* microseconds to delay after this transfer. */
};

/**
 *  struct tls_spi_message - one multi-segment SPI transaction
 *
 *  A struct tls_spi_message is used to execute an atomic sequence of data 
 *  transfers, each represented by a struct tls_spi_transfer.  The sequence 
 *  is "atomic" in the sense that no other spi_message may use that SPI bus 
 *  until that sequence completes. 
 */
struct tls_spi_message {
	struct dl_list queue; /* transaction messages are sequenced through @tls_spi_port.wait_queue. */
	
	struct dl_list transfers; /* list of transfer segments in this transaction. */
	void (*complete)(void *); /* called to report transaction completions. */
	void *context; /* the argument to complete() when it's called. */
	u32 status; /* transaction message status. */
};

/**
 *  struct tls_spi_port - driver structure to SPI master controller
 *
 *  This data structure presents the SPI master controller's configuration 
 *  data. The device attached to this SPI master controller share the same 
 *  transfer mode, chipselect mode and clock rate. And this structure maintains 
 *  a queue of tls_spi_message transactions and uses this tls_spi_message transaction 
 *  to access to the SPI device. For each such message it queues, it calls the message's 
 *  completion function when the transaction completes.
 */
struct tls_spi_port {
	u32 speed_hz; /* clock rate to be used. */
	u8 cs_active; /* chipselect mode, maybe active low or active high. */
	u8 mode; /* SPI transfer mode: mode_0(CPHA=0, CHOL=0), mode_1(CPHA=0, CHOL=1), 
				mode_2(CPHA=1, CHOL=0), mode_3(CPHA=1, CHOL=1). */
	u8 reconfig;

	struct dl_list wait_queue; /* wait list of transaction messages. */
	tls_os_queue_t *lock;
	
	tls_os_queue_t *msg_queue; /* notify the schedule thread that there's transaction message queued. */
	struct tls_spi_message *current_message; /* current transaction message in-progressing. */
	u32 current_remaining_transfer; /* remaining transfer segments count in current transaction message. */

	struct tls_spi_transfer *current_transfer; /* current transfer segment in-progressing. */
	u32 current_remaining_bytes; /* remaining data length in current transfer segment. */

	u8 transtype;	/*加速方式，0不加速，1字传输，2 DMA字传输*/
};

/**********************************************************************************************************
* Description: 	This function is used to initialize the SPI master driver.
*
* Arguments  : 	
*
* Returns    : 	TLS_SPI_STATUS_OK			if initialize success
* 		TLS_SPI_STATUS_EBUSY			if SPI is already initialized
* 		TLS_SPI_STATUS_ENOMEM			if malloc SPI memory fail*		
**********************************************************************************************************/
int tls_spi_init(void);


/**********************************************************************************************************
* Description: 	This function is used to select SPI slave type.
*
* Arguments  : 	slave					is the slave type,defined as follow:
* 							slave == SPI_SLAVE_FLASH	:flash
* 							slave == SPI_SLAVE_CARD		: sd card
*
* Returns    : 	Before communicate with different SPI device, must call the function.
**********************************************************************************************************/
void tls_spi_slave_sel(u16 slave);


/**********************************************************************************************************
* Description: 	This function is used to setup the spi CPOL,CPHA,cs signal and clock.
*
* Arguments  : 	mode					is CPOL and CPHA type defined in TLS_SPI_MODE_0 to TLS_SPI_MODE_3
* 		cs_active				is cs mode, defined as follow:
* 							cs_active == TLS_SPI_CS_LOW 	:low is active
* 							cs_active == TLS_SPI_CS_HIGH 	:high is active
* 		fclk					is spi clock,the unit is HZ.
*
*
* Returns    : 	TLS_SPI_STATUS_OK			if initialize success
* 		TLS_SPI_STATUS_EBUSY			if SPI is already initialized
* 		TLS_SPI_STATUS_ENOMEM			if malloc SPI memory fail
**********************************************************************************************************/
int tls_spi_setup(u8 mode, u8 cs_active, u32 fclk);

/**********************************************************************************************************
* Description: 	This function is used to synchronous write data by SPI. 
*
* Arguments  : 	buf					is the user data.
* 		len					is the data length.
*
* Returns    : 	TLS_SPI_STATUS_OK			if write success.
* 		TLS_SPI_STATUS_EINVAL			if argument is invalid.
* 		TLS_SPI_STATUS_ENOMEM			if there is no enough memory.
* 		TLS_SPI_STATUS_ESHUTDOWN		if SPI driver does not installed.
**********************************************************************************************************/
int tls_spi_write(const u8 *buf, u32 len);

/**********************************************************************************************************
* Description: 	This function is used to synchronous read data by SPI. 
*
* Arguments  : 	buf					is the buffer for saving SPI data.
* 		len					is the data length.
*
* Returns    : 	TLS_SPI_STATUS_OK			if write success.
* 		TLS_SPI_STATUS_EINVAL			if argument is invalid.
* 		TLS_SPI_STATUS_ENOMEM			if there is no enough memory.
* 		TLS_SPI_STATUS_ESHUTDOWN		if SPI driver does not installed.
**********************************************************************************************************/
int tls_spi_read(u8 *buf, u32 len);

/**********************************************************************************************************
* Description: 	This function is used to synchronous write data then read data by SPI.
*
* Arguments  : 	txbuf					is the write data buffer.
* 		n_tx					is the write data length.
*		rxbuf					is the read data buffer.
*		n_rx					is the read data length.
* Returns    : 	TLS_SPI_STATUS_OK			if write success.
* 		TLS_SPI_STATUS_EINVAL			if argument is invalid.
* 		TLS_SPI_STATUS_ENOMEM			if there is no enough memory.
* 		TLS_SPI_STATUS_ESHUTDOWN		if SPI driver does not installed.
**********************************************************************************************************/
int tls_spi_write_then_read(const u8 *txbuf, u32 n_tx, u8 *rxbuf, u32 n_rx);

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
int tls_spi_write32_then_writen(const u32 *buf32, const u8 *txbuf, u32 n_tx);

/**********************************************************************************************************
* Description: 	This function is used to set SPI transfer mode.
*
* Arguments  : 	type					is the transfer type.
*				type == SPI_BYTE_TRANSFER	byte transfer
*				type == SPI_WORD_TRANSFER	word transfer
*				type == SPI_DMA_TRANSFER	DMA transfer
* Returns    :
* Notes	    :		if user want to speed up the spi speed,need call this function.
**********************************************************************************************************/
void tls_spi_trans_type(u8 type);

#endif /* WM_HOST_SPI_H */
 
