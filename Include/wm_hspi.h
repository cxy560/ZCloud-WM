/***************************************************************************** 
* 
* File Name : wm_hspi.h 
* 
* Description: High speed spi slave Module 
* 
* Copyright (c) 2014 Winner Micro Electronic Design Co., Ltd. 
* All rights reserved. 
* 
* Author : dave
* 
* Date : 2014-6-2 
*****************************************************************************/ 
#ifndef WM_HSPI_H
#define WM_HSPI_H

#include "wm_type_def.h"


#define HSPI_INTERFACE_SPI		2		//接口用于spi
#define HSPI_INTERFACE_SDIO	3		//接口用于sdio

/*rx message，用于通知其他线程处理数据*/
#define HSPI_RX_CMD_MSG     1			
#define HSPI_RX_DATA_MSG    2

//spi/sdio buffer定义，wraper控制器只能访问0x60000之后的地址
#define HSPI_TXBUF_NUM               6 
#define HSPI_TX_DESC_NUM            HSPI_TXBUF_NUM
#define HSPI_RXBUF_NUM              10 
#define HSPI_RX_DESC_NUM            HSPI_RXBUF_NUM
#define HSPI_TXBUF_SIZE             1500 
#define HSPI_RXBUF_SIZE             1500

#define HSPI_TX_DESC_SIZE          sizeof(struct tls_hspi_tx_desc)
#define HSPI_RX_DESC_SIZE          sizeof(struct tls_hspi_rx_desc)

/*****************************************************************************
 * sdio/hspi sram partition
 * total size : 0x61800 - 0x67FFFF   (26KB)
 ******************************************************************************/
/* HSPI txbuf zone */
#define HSPI_TXBUF_BASE_ADDR        ((u32)(0x00061800))	//前面的地址用于master spi dma
#define HSPI_TXBUF_TOTAL_SIZE        (HSPI_TXBUF_SIZE * HSPI_TXBUF_NUM) 
/* HSPI tx desc zone */
#define HSPI_TX_DESC_BASE_ADDR      ((u32)(HSPI_TXBUF_BASE_ADDR + HSPI_TXBUF_TOTAL_SIZE))
#define HSPI_TX_DESC_TOTAL_SIZE     (HSPI_TX_DESC_SIZE * HSPI_TX_DESC_NUM)
/* HSPI rxbuf zone */
#define HSPI_RXBUF_BASE_ADDR        ((u32)(HSPI_TX_DESC_BASE_ADDR + HSPI_TX_DESC_TOTAL_SIZE))
#define HSPI_RXBUF_TOTAL_SIZE       (HSPI_RXBUF_NUM * HSPI_RXBUF_SIZE)
/* HSPI rx desc zone */
#define HSPI_RX_DESC_BASE_ADDR      ((u32)(HSPI_RXBUF_BASE_ADDR + HSPI_RXBUF_TOTAL_SIZE))
#define HSPI_RX_DESC_TOTAL_SIZE     (HSPI_RX_DESC_SIZE * HSPI_RX_DESC_NUM)

#define SDIO_CMD_RXBUF_SIZE          (0x100)
#define SDIO_CMD_RXBUF_ADDR          (0x00068000 -  SDIO_CMD_RXBUF_SIZE)
#define SDIO_CIS1_ADDR              (SDIO_CMD_RXBUF_ADDR - 0x80)
#define SDIO_CIS0_ADDR              (SDIO_CIS1_ADDR - 0x80)

#define CIS_FUN0_ADDR	SDIO_CIS0_ADDR
#define CIS_FUN1_ADDR	SDIO_CIS1_ADDR

#define FN0_TPL_FUNCID				(CIS_FUN0_ADDR + 0x00)
#define FN0_TPL_FUNCE				(CIS_FUN0_ADDR + 0x04)
#define FN0_TPL_FUNCE_MAXBLK		(CIS_FUN0_ADDR + 0x08)
#define FN0_TPL_MANFID_MID		    (CIS_FUN0_ADDR + 0x0C)
#define FN0_TPL_END					(CIS_FUN0_ADDR + 0x10)

#define FN1_TPL_FUNCID				(CIS_FUN1_ADDR + 0x00)
#define FN1_TPL_FUNCE				(CIS_FUN1_ADDR + 0x04)
#define FN1_TPL_FUNCE_VER			(CIS_FUN1_ADDR + 0x08)
#define FN1_TPL_FUNCE_NSN			(CIS_FUN1_ADDR + 0x0C)
#define FN1_TPL_FUNCE_CSASIZE		(CIS_FUN1_ADDR + 0x10)
#define FN1_TPL_FUNCE_OCR			(CIS_FUN1_ADDR + 0x14)
#define FN1_TPL_FUNCE_MINPWR		(CIS_FUN1_ADDR + 0x18)
#define FN1_TPL_FUNCE_STANDBY		(CIS_FUN1_ADDR + 0x1C)
#define FN1_TPL_FUNCE_OPTBW		    (CIS_FUN1_ADDR + 0x20)
#define FN1_TPL_FUNCE_NTIMEOUT	    (CIS_FUN1_ADDR + 0x24)
#define FN1_TPL_FUNCE_AVGPWR		(CIS_FUN1_ADDR + 0x28)
#define FN1_TPL_END					(CIS_FUN1_ADDR + 0x30)



/*系统中对于sdio中断的定义*/
#define SDIO_TX_DATA_INT            (0UL)
#define SDIO_RX_DATA_INT          (1UL)
#define SDIO_TX_CMD_INT             (2UL)
#define SDIO_RX_CMD_INT             (3UL)

/*IO控制寄存器*/
#define HR_IOCTL_GP_SDIO        (0xf001200 + 0x04)


/* SDIO interrupt bit definition */
#define SDIO_WP_INT_SRC_CMD_DOWN         (1UL<<3)
#define SDIO_WP_INT_SRC_CMD_UP           (1UL<<2)
#define SDIO_WP_INT_SRC_DATA_DOWN        (1UL<<1)
#define SDIO_WP_INT_SRC_DATA_UP          (1UL<<0)

/**
 * struct tls_hspi_tx_desc是由软件和硬件共同维护用于从硬件上传数据的
 * 接收描述符数据结构定义
 */
struct tls_hspi_tx_desc {
    u32 valid_ctrl;
    u32 buf_info;
    u32 buf_addr[3];
    u32 next_desc_addr;
};

struct tls_hspi_rx_desc {
    u32 valid_ctrl;
    u32 buf_addr;
    u32 next_desc_addr;
};


struct tls_slave_hspi {
    u8 ifusermode;	//如果设置成用户模式，系统中通过hspi的精简指令不能使用了
    u16 (*rx_cmd_callback)(char *buf);
    u16 (*rx_data_callback)(char *buf);
    u16 (*tx_data_callback)(char *buf);
	
    /* 上行数据管理 */
    struct tls_hspi_tx_desc *curr_tx_desc;

    /* 下行数据管理 */
    struct tls_hspi_rx_desc   *curr_rx_desc;
}; 


/**********************************************************************************************************
* Description: 	This function is used to initial HSPI register.
*
* Arguments  : 	portmode == HSPI_INTERFACE_SPI	SPI interface
*				portmode == HSPI_INTERFACE_SDIO	sdio interface

* Returns    :
* Notes	    :		When the system is initialized, the function has been called, so users can not call the function.
**********************************************************************************************************/
int tls_slave_spi_init(int portmode);

/**********************************************************************************************************
* Description: 	This function is used to enable or disable user mode.
*
* Arguments  : 	TRUE or FALSE

* Returns    :
* Notes	    :		If the user enables the user mode, RICM instruction in the system will not be used by SPI.
*		If the user wants to use the SPI interface as other use, need to enable the user mode.
*		This function must be called before the register function.
**********************************************************************************************************/
void tls_set_hspi_user_mode(u8 ifenable);


/**********************************************************************************************************
* Description: 	This function is used to set HSPI interface type.
*
* Arguments  : 	type					is the interface type.
*				type == HSPI_INTERFACE_SPI	SPI interface
*				type == HSPI_INTERFACE_SDIO	sdio interface

* Returns    :
**********************************************************************************************************/
void tls_set_hspi_interface_type(int type);


/**********************************************************************************************************
* Description: 	This function is used to register hspi rx command interrupt.
*
* Arguments  : 	rx_cmd_callback		is the hspi rx interrupt call back function.
*
* Returns    : 
*
**********************************************************************************************************/
void tls_hspi_rx_cmd_register(s16 (*rx_cmd_callback)(char *buf));

/**********************************************************************************************************
* Description: 	This function is used to register hspi rx data interrupt.
*
* Arguments  : 	rx_data_callback		is the hspi rx interrupt call back function.
*
* Returns    : 
*
**********************************************************************************************************/
void tls_hspi_rx_data_register(s16 (*rx_data_callback)(char *buf));

/**********************************************************************************************************
* Description: 	This function is used to register hspi tx data interrupt.
*
* Arguments  : 	tx_data_callback		is the hspi tx interrupt call back function.
*
* Returns    : 
*
**********************************************************************************************************/
void tls_hspi_tx_data_register(s16 (*tx_data_callback)(char *buf));


/**********************************************************************************************************
* Description: 	This function is used to transfer data.
*
* Arguments  : 	buf			is a buf for saving user data. 
* 				len			is the data length.
*
* Returns    : 	
**********************************************************************************************************/
void tls_hspi_tx_data(char *txbuf, int len);

#endif /* WM_HSPI_H */
