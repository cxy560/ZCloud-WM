/***************************************************************************** 
* 
* File Name : wm_uart.h
* 
* Description: uart Driver Module 
* 
* Copyright (c) 2014 Winner Microelectronics Co., Ltd. 
* All rights reserved. 
* 
* Author : dave
* 
* Date : 2014-6-5
*****************************************************************************/ 
#ifndef WM_UART_H
#define WM_UART_H

#include "wm_type_def.h"
#include "wm_regs.h"
#include "list.h"
#include "wm_osal.h"

#define TLS_UART_RX_BUF_SIZE   4096
#define TLS_UART_TX_BUF_SIZE   4096
#define WAKEUP_CHARS           256

#define MBOX_MSG_UART_RX       1
#define MBOX_MSG_UART_TX       2


/** baud rate definition */
#define UART_BAUDRATE_B600          600	
#define UART_BAUDRATE_B1200	        1200
#define UART_BAUDRATE_B1800         1800	
#define UART_BAUDRATE_B2400         2400	
#define UART_BAUDRATE_B4800         4800	
#define UART_BAUDRATE_B9600         9600	
#define UART_BAUDRATE_B19200        19200	
#define UART_BAUDRATE_B38400        38400	
#define UART_BAUDRATE_B57600        57600 
#define UART_BAUDRATE_B115200       115200
#define UART_BAUDRATE_B230400       230400 
#define UART_BAUDRATE_B460800       460800 
#define UART_BAUDRATE_B921600      921600 
#define UART_BAUDRATE_B1000000      1000000 
#define UART_BAUDRATE_B1250000      1250000 
#define UART_BAUDRATE_B1500000      1500000 
#define UART_BAUDRATE_B2000000      2000000 

#define UART_RX_INT_FLAG (UIS_RX_FIFO | UIS_RX_FIFO_TIMEOUT | UIS_BREAK |\
        UIS_OVERRUN | UIS_FRM_ERR | UIS_PARITY_ERR)
#define UART_RX_ERR_INT_FLAG (UIS_BREAK | UIS_FRM_ERR | \
        UIS_PARITY_ERR)

#define UART_TX_INT_FLAG (UIS_TX_FIFO | UIS_TX_FIFO_EMPTY)


#ifndef UART0_INT
#define UART0_INT                   (23UL)
#endif
#ifndef UART1_INT
#define UART1_INT                   (24UL)
#endif

/* Return count in buffer.  */
#define CIRC_CNT(head,tail,size) (((head) - (tail)) & ((size)-1))

/* Return space available, 0..size-1.  We always leave one free char
   as a completely full buffer has head == tail, which is the same as
   empty.  */
#define CIRC_SPACE(head,tail,size) CIRC_CNT((tail),((head)+1),(size))

/* Return count up to the end of the buffer.  Carefully avoid
   accessing head and tail more than once, so they can change
   underneath us without returning inconsistent results.  */
#define CIRC_CNT_TO_END(head,tail,size) \
	({int end = (size) - (tail); \
	  int n = ((head) + end) & ((size)-1); \
	  n < end ? n : end;})

/* Return space available up to the end of the buffer.  */
#define CIRC_SPACE_TO_END(head,tail,size) \
	({int end = (size) - 1 - (head); \
	  int n = (end + (tail)) & ((size)-1); \
	  n <= end ? n : end+1;})

#define CIRC_SPACE_TO_END_FULL(head,tail,size) \
	({int end = (size) - 1 - (head); \
	  int n = (end + (tail)) & ((size)-1); \
	  n < end ? n : end+1;})

#define uart_circ_empty(circ)		((circ)->head == (circ)->tail)
#define uart_circ_chars_pending(circ)	\
	(CIRC_CNT((circ)->head, (circ)->tail, TLS_UART_TX_BUF_SIZE))

struct tls_uart_baud_rate {
    u32 baud_rate;
    u16 ubdiv;
    u16 ubdiv_frac;
};


/** uart number enum */
enum {
    TLS_UART_0 = 0,
    TLS_UART_1 = 1,
};


/** Parity Mode */
typedef enum TLS_UART_PMODE
{
    TLS_UART_PMODE_DISABLED = 0,  /* No Parity */
    TLS_UART_PMODE_ODD      = 1,  /* Odd Parity */
    TLS_UART_PMODE_EVEN     = 2,  /* Even Parity */
    TLS_UART_PMODE_MARK     = 3,  /* The parity bit is always 1. */
    TLS_UART_PMODE_SPACE    = 4,  /* The parity bit is always 0. */
} TLS_UART_PMODE_T;

/** Character Size */
typedef enum TLS_UART_CHSIZE
{
    TLS_UART_CHSIZE_5BIT = (0x00<<0),  /* Character size: 5 bit */
    TLS_UART_CHSIZE_6BIT = (0x01<<0),  /* Character size: 6 bit */
    TLS_UART_CHSIZE_7BIT = (0x02<<0),  /* Character size: 7 bit */
    TLS_UART_CHSIZE_8BIT = (0x03<<0),  /* Character size: 8 bit */
} TLS_UART_CHSIZE_T;

/** flow control mode */
typedef enum TLS_UART_FLOW_CTRL_MODE {
    TLS_UART_FLOW_CTRL_NONE,
    TLS_UART_FLOW_CTRL_HARDWARE,
} TLS_UART_FLOW_CTRL_MODE_T;

/** flow control rx flag */
typedef enum TLS_UART_RX_FLOW_CTRL_FLAG {
    TLS_UART_RX_DISABLE,
    TLS_UART_RX_ENABLE,
} TLS_UART_RX_FLOW_CTRL_FLAG_T;

typedef enum TLS_UART_STOPBITS {
    TLS_UART_ONE_STOPBITS,
    TLS_UART_TWO_STOPBITS,
} TLS_UART_STOPBITS_T;


typedef enum TLS_UART_STATUS {
    TLS_UART_STATUS_OK,
    TLS_UART_STATUS_ERROR,
} TLS_UART_STATUS_T; 

/** operation mode */
typedef enum TLS_UART_MODE {
    TLS_UART_MODE_POLL,           /* uart operation mode: poll */
    TLS_UART_MODE_INT,            /* uart operation mode: interrupt mode */
} TLS_UART_MODE_T;

struct tls_uart_icount {
	u32	cts;
	u32	dsr;
	u32	rng;
	u32	dcd;
	u32	rx;
	u32	tx;
	u32	frame;
	u32	overrun;
	u32	parity;
	u32	brk;
	u32	buf_overrun;
};


/** uart parameters */
typedef struct tls_uart_options {
	/** Set baud rate of the UART  */
	u32 baudrate;

	/** Number of bits to transmit as a character (5 to 8). */
	TLS_UART_CHSIZE_T charlength;

	/** 
     * Parity type: TLS_UART_PMODE_DISABLED, 
     *              TLS_UART_PMODE_EVEN,
     *              TLS_UART_PMODE_ODD  
     */
	TLS_UART_PMODE_T paritytype;

    /**
     * Flow control type
     */
    TLS_UART_FLOW_CTRL_MODE_T flow_ctrl;

	/**
     * Number of stop bits between two characters:
	 * true: 2 stop bits
	 * false: 1 stop bit
     */
	TLS_UART_STOPBITS_T stopbits;

} tls_uart_options_t;


typedef struct tls_uart_circ_buf {
    u8 *buf;
    u32 head;
    u32 tail;
} tls_uart_circ_buf_t;

typedef struct TLS_UART_REGS {
    TLS_REG UR_LC;              // line control register
    TLS_REG UR_FC;              // flow control register
    TLS_REG UR_DMAC;            // dma control register
    TLS_REG UR_FIFOC;           // fifo control register
    TLS_REG UR_BD;              // baud rate register
    TLS_REG UR_INTM;            // interrupt mask register
    TLS_REG UR_INTS;            // interrupt source register
    TLS_REG UR_FIFOS;           // fifo status register
    TLS_REG UR_TXW;             // tx windows register
    TLS_REG UR_RES0;
    TLS_REG UR_RES1;
    TLS_REG UR_RES2;
    TLS_REG UR_RXW;             // rx windows register
} TLS_UART_REGS_T;


typedef struct tls_uart_port {
    /** uart number: 0 or 1 */
    u32                         uart_no;

    /** uart interrupt number */
    u32                         uart_irq_no;

  //  u32                         cmd_mode;
	
    u32                         plus_char_cnt;
    /** uart work mode: interrupt mode or poll mode */
    TLS_UART_MODE_T             uart_mode;     

    /** uart config parameters */
    struct tls_uart_options     opts;    

    int fcStatus;	//flow ctrl status,0 流控状态是关闭的，1是打开的
    enum TLS_UART_RX_FLOW_CTRL_FLAG rxstatus;

    /** uart tx fifo trigger level */
    u32                         tx_fifofull;

    /** uart registers struct pointer */
    TLS_UART_REGS_T         volatile    *regs;

    /** uart statistics information */
	struct tls_uart_icount	    icount;			

    /** uart ring buffer */
    struct tls_uart_circ_buf    recv;
    struct tls_uart_circ_buf    xmit;

    u8                         hw_stopped;

	tls_os_sem_t *tx_sem;

	u8	ifusermode;	/*是否用户模式*/
	char	*buf_ptr;
	u16 buf_len;
	s16 (*rx_callback)(char *buf, u16 len);
	s16 (*rx_callback_bk)(char *buf, u16 len);
	
	s16 (*tx_callback)(struct tls_uart_port *port);	
	s16 (*tx_callback_bk)(struct tls_uart_port *port);	
} tls_uart_port_t;


/**********************************************************************************************************
* Description: 	This function is used to initial uart port.
*
* Arguments  : 	uart_no		is the uart number
*					uart_no	==	TLS_UART_0
*					uart_no	==	TLS_UART_1
*				opts			is the uart setting options,if this param is NULL,this function will use the default options.
* Returns    : 		WM_SUCCESS: init correctly;
*				WM_FAILED: error;			
* Notes      : 		When the system is initialized, the function has been called, so users can not call the function.
**********************************************************************************************************/
int tls_uart_port_init(int uart_no, tls_uart_options_t *opts);

/**********************************************************************************************************
* Description: 	This function is used to set flow ctrl.
*
* Arguments  : 	uart_no		is the uart number
*					uart_no	==	TLS_UART_0
*					uart_no	==	TLS_UART_1
*				flow_ctrl		is the flow ctrl mode.
*					flow_ctrl == TLS_UART_FLOW_CTRL_NONE
*					flow_ctrl == TLS_UART_FLOW_CTRL_HARDWARE
	
**********************************************************************************************************/
void tls_uart_set_fc_status(int uart_no,TLS_UART_FLOW_CTRL_MODE_T status);


/**********************************************************************************************************
* Description: 	This function is used to set uart rx status.
*
* Arguments  : 	uart_no		is the uart number
*					uart_no	==	TLS_UART_0
*					uart_no	==	TLS_UART_1
*				flag			
*					flag		==	TLS_UART_RX_DISABLE
*					flag 		==	TLS_UART_RX_ENABLE
*Notes:			if user disalbe the rx,uart will not receive data until user enable the rx.
**********************************************************************************************************/
void tls_set_uart_rx_status(int uart_no,int status);

/**********************************************************************************************************
* Description: 	This function is used to fill tx buffer.
*
* Arguments  : 	port			is the uart port
* 				buf			is the user buffer.
*				count		is the user data length
*
* Returns    : 		WM_FAILED: Write error
*				data length(Bytes) written into hw
**********************************************************************************************************/
int tls_uart_write(struct tls_uart_port *port, char *buf, u32 count);

/**********************************************************************************************************
* Description: 	This function is used to start transfer data.
*
* Arguments  : 	port			is the uart port
*
* Returns    : 	
**********************************************************************************************************/
void tls_uart_tx_chars_start(struct tls_uart_port *port);


/**********************************************************************************************************
* Description: 	This function is used to register uart rx interrupt.
*
* Arguments  : 	uart_no		TLS_UART_0 or TLS_UART_1
*				callback		is the uart rx interrupt call back function.
*
* Returns    : 
*
**********************************************************************************************************/
void tls_uart_rx_register(u16 uart_no, s16(*rx_callback)(char *buf, u16 len));

/**********************************************************************************************************
* Description: 	This function is used to register uart tx interrupt.
*
* Arguments  : 	callback		is the uart tx interrupt call back function.
*
* Returns    : 
*
**********************************************************************************************************/
void tls_uart_tx_register(u16 uart_no, s16 (*tx_callback)(struct tls_uart_port *port));


/**********************************************************************************************************
* Description: 	This function is used to transfer data asynchronous.
*
* Arguments  : 	buf			is a buf for saving user data. 
* 		len			is the data length.
*
* Returns    : 		WM_SUCCESS: TX ok;
*				WM_FAILED:	  error
* Notes		:	The function only start transmission, fill buffer in the callback function.
**********************************************************************************************************/
int tls_uart_tx(char *buf, u16 len);

/**********************************************************************************************************
* Description: 	This function is used to get tx transfer data length.
*
* Arguments  :
*
* Returns    : 		length
**********************************************************************************************************/
int tls_uart_tx_length(void);

/**********************************************************************************************************
* Description: 	This function is used to transfer data synchronous.
*
* Arguments  : 	buf			is a buf for saving user data. 
* 				len			is the data length.
*
* Returns    : 		WM_SUCCESS 	tx success
*				WM_FAILED		tx failed  	
* Notes		:	
**********************************************************************************************************/
int tls_uart_tx_sync(char *buf, u16 len);

/**********************************************************************************************************
* Description: 	This function is used to config uart port for user mode.
*
* Arguments  : 	
*
* Returns    : 	
* Notes      : 	If user want to use uart function,must call this function.
*			This function must be called before the register function.
**********************************************************************************************************/
void tls_uart_cfg_user_mode(void);


/**********************************************************************************************************
* Description: 	This function is used to disable uart user mode.
*
* Arguments  : 	
*
* Returns    : 	
* Notes      : 	If user want to resume uart old function for AT commond,must call this function.
**********************************************************************************************************/
void tls_uart_disable_user_mode(void);


/**********************************************************************************************************
* Description: 	This function is used to register uart rx interrupt.
*
* Arguments  : 	callback		is the uart rx interrupt call back function.
*
* Returns    : 
*
**********************************************************************************************************/
void tls_user_uart_rx_register(s16 (*rx_callback)(char *buf, u16 len));

/**********************************************************************************************************
* Description: 	This function is used to register uart tx interrupt.
*
* Arguments  : 	callback		is the uart tx interrupt call back function.
*
* Returns    : 
*
**********************************************************************************************************/
void tls_user_uart_tx_register(s16 (*tx_callback)(struct tls_uart_port *port));


/**********************************************************************************************************
* Description: 	This function is used to set uart parity.
*
* Arguments  : 	paritytype		is a parity type defined in TLS_UART_PMODE_T
*
* Returns    :  WM_SUCCESS	if setting success
* 			WM_FAILED	 	if setting fail
**********************************************************************************************************/
int tls_user_uart_set_parity(TLS_UART_PMODE_T paritytype);

/**********************************************************************************************************
* Description: 	This function is used to set uart baudrate.
*
* Arguments  : 	baudrate		is the baudrate user want used,the unit is HZ.
*
* Returns    :  WM_SUCCESS	if setting success
* 		WM_FAILED 	if setting fail
**********************************************************************************************************/
int tls_user_uart_set_baud_rate(u32 baudrate);

/**********************************************************************************************************
* Description: 	This function is used to set uart stop bits.
*
* Arguments  : 	baudrate		is a stop bit type defined in TLS_UART_STOPBITS_T
*
* Returns    :  WM_SUCCESS	if setting success
* 		WM_FAILED 	if setting fail
**********************************************************************************************************/
int tls_user_uart_set_stop_bits(TLS_UART_STOPBITS_T stopbits);


//==============================================================================================================


#endif /* WM_UART_H */
