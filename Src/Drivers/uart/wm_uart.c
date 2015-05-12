/***************************************************************************** 
* 
* File Name : wm_uart.c 
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
#include <string.h>
#include "wm_uart.h"
#include "wm_debug.h"
#include "wm_irq.h"
#include "wm_config.h"
#include "wm_mem.h"

#if TLS_CONFIG_UART
struct tls_uart_port uart_port[2];
static void tls_uart_tx_chars(struct tls_uart_port *port);

 
const struct tls_uart_baud_rate  baud_rates[] = { 
    {2000000, 0, 4},
    {1500000, 0, 11},
    {1250000, 1, 0},
    {1000000, 1, 8},
    {921600,  1, 11},
    {460800,  4, 7},
    {230400,  9, 14},
    {115200,  20, 11},
    {57600,   42, 6},
    {38400,   64, 2},
    {19200,   129, 3},
    {9600,    259, 7},
    {4800,    519, 13},
    {2400,    1040, 11},
    {1800,    1387, 14},
    {1200,    2082, 5},
    {600,     4165, 11},
};

static void tls_uart_tx_enable(struct tls_uart_port *port);
static void tls_uart_tx_chars(struct tls_uart_port *port);

void UartRegInit(int uart_no)
{
    	int ioctl;
		
    /* baud rate register value = apb_clk/(16*115200) */
    /* 如果APB时钟是40MHz， */
    /* 波特率寄存器的值设置为  115200  : 21 */
    /*                         9600bps : 260 */
	if(TLS_UART_0 == uart_no)
	{
	    tls_reg_write32(HR_UART0_BAUD_RATE_CTRL, 21);
	    /* Line control register : Normal,No parity,1 stop,8 bits, only use tx */
	    tls_reg_write32(HR_UART0_LINE_CTRL, ULCON_WL8|ULCON_TX_EN);

	    /* disable auto flow control */
	    tls_reg_write32(HR_UART0_FLOW_CTRL, 0);
	    /* disable dma */
	    tls_reg_write32(HR_UART0_DMA_CTRL, 0);
	    /* one byte tx */
	    tls_reg_write32(HR_UART0_FIFO_CTRL, 0);
	    /* disable interrupt */
	    tls_reg_write32(HR_UART0_INT_MASK, 0xFF);
	//uart0
            ioctl = tls_reg_read32(HR_IOCTL_GP_JTAG);
            ioctl &= 0xFFFF00FF;
            ioctl |= 0x00005500;
            tls_reg_write32(HR_IOCTL_GP_JTAG, ioctl);

	}

	if(TLS_UART_1 == uart_no)
	{
		/* 4 byte tx, 8 bytes rx */
		tls_reg_write32(HR_UART1_FIFO_CTRL, (0x01<<2)|(0x02<<4));
		/* enable rx timeout, disable rx dma, disable tx dma */ 
		tls_reg_write32(HR_UART1_DMA_CTRL, (8<<3)|(1<<2));
		/* enable rx/timeout interrupt */
		tls_reg_write32(HR_UART1_INT_MASK, ~(3<<2));

		//uart1
            ioctl = tls_reg_read32(HR_IOCTL_GP_SDIO_I2C);
            ioctl &= 0xFFFF00FF;
            ioctl |= 0x00005500;
            tls_reg_write32(HR_IOCTL_GP_SDIO_I2C, ioctl);
	}

}

int tls_uart_check_baudrate(u32 baudrate)
{
    int i;

    for (i = 0; i < sizeof(baud_rates)/sizeof(struct tls_uart_baud_rate); i++) {
        if (baudrate == baud_rates[i].baud_rate)
            return i;
    }
    /* not found match baudrate */
    return -1;
}

u32 tls_uart_get_baud_rate(struct tls_uart_port *port)
{
    if ((port != NULL) && (port->regs != NULL))
        return port->opts.baudrate;
    else
        return 0;
}

int tls_uart_set_baud_rate(struct tls_uart_port *port, 
        u32 baudrate)
{
    int index;
    u32 value;

    index = tls_uart_check_baudrate(baudrate);
    if (index < 0)
        return WM_FAILED;

    TLS_DBGPRT_INFO("ubdiv_frac = 0x%x\n", baud_rates[index].ubdiv_frac);
    value = baud_rates[index].ubdiv | (baud_rates[index].ubdiv_frac << 16);
    TLS_DBGPRT_INFO("value = 0x%x\n", value);
    port->regs->UR_BD = value;

    port->opts.baudrate = baudrate;
    TLS_DBGPRT_INFO("UR_BR = 0x%x\n", port->regs->UR_BD);

    return WM_SUCCESS;
}


int tls_uart_set_parity(struct tls_uart_port *port, 
        TLS_UART_PMODE_T paritytype)
{
    if (port == NULL)
        return WM_FAILED;

    port->opts.paritytype = paritytype;

    if (paritytype == TLS_UART_PMODE_DISABLED)
        port->regs->UR_LC &= ~ULCON_PMD_EN;
    else if (paritytype == TLS_UART_PMODE_EVEN) {
        port->regs->UR_LC &= ~ULCON_PMD_MASK;
        port->regs->UR_LC |= ULCON_PMD_EVEN;
    } else if (paritytype == TLS_UART_PMODE_ODD) {
        port->regs->UR_LC &= ~ULCON_PMD_MASK;
        port->regs->UR_LC |= ULCON_PMD_ODD;
    } else
        return WM_FAILED;

    return WM_SUCCESS;

}

TLS_UART_PMODE_T tls_uart_get_parity(struct tls_uart_port *port)
{
    return port->opts.paritytype;
}

int tls_uart_set_data_bits(struct tls_uart_port *port, 
        TLS_UART_CHSIZE_T charlength)
{
    if (!port)
        return WM_FAILED;

    port->opts.charlength = charlength;

    port->regs->UR_LC &= ~ULCON_WL_MASK;

    if (charlength == TLS_UART_CHSIZE_5BIT)
        port->regs->UR_LC |= ULCON_WL5;
    else if (charlength == TLS_UART_CHSIZE_6BIT)
        port->regs->UR_LC |= ULCON_WL6;
    else if (charlength == TLS_UART_CHSIZE_7BIT)
        port->regs->UR_LC |= ULCON_WL7;
    else if (charlength == TLS_UART_CHSIZE_8BIT)
        port->regs->UR_LC |= ULCON_WL8;
    else 
        return WM_FAILED;

    return WM_SUCCESS;
}

TLS_UART_CHSIZE_T tls_uart_get_data_bits(struct tls_uart_port *port)
{
    return port->opts.charlength;
}

int tls_uart_set_stop_bits(struct tls_uart_port *port, 
        TLS_UART_STOPBITS_T stopbits)
{
    if (!port)
        return WM_FAILED;

    port->opts.stopbits = stopbits;

    if (stopbits == TLS_UART_TWO_STOPBITS)
        port->regs->UR_LC |= ULCON_STOP_2;
    else 
        port->regs->UR_LC &= ~ULCON_STOP_2;

    return WM_SUCCESS;
}

TLS_UART_STOPBITS_T tls_uart_get_stop_bits(struct tls_uart_port *port)
{
    return port->opts.stopbits;
}

TLS_UART_STATUS_T tls_uart_set_flow_ctrl(struct tls_uart_port *port,
        TLS_UART_FLOW_CTRL_MODE_T flow_ctrl)
{
    TLS_UART_STATUS_T status = TLS_UART_STATUS_OK;
	
    if (!port)
        return TLS_UART_STATUS_ERROR;

  //  port->opts.flow_ctrl = flow_ctrl;		//不能在这里修改，为了配合透传和AT指令，软件会自己修改flowctrl配置，但是参数还是固定不变的
//printf("\nport %d flow ctrl==%d\n",port->uart_no,flow_ctrl);
    switch (flow_ctrl) {
        case TLS_UART_FLOW_CTRL_NONE:
            port->regs->UR_FC = 0;
            break;
        case TLS_UART_FLOW_CTRL_HARDWARE:
		if(TLS_UART_FLOW_CTRL_HARDWARE == port->opts.flow_ctrl)
		{
            		port->regs->UR_FC = (1UL<<0) | (6UL<<2);
		}
            break;
        default:
            return TLS_UART_STATUS_ERROR;
    }

    return status; 
}

void tls_uart_set_fc_status(int uart_no,TLS_UART_FLOW_CTRL_MODE_T status)
{
	struct tls_uart_port *port;
	
	if(TLS_UART_0 == uart_no)
	{
		port = &uart_port[0];
	}
	else
	{
		port = &uart_port[1];	
	}

	port->fcStatus = status;
	//printf("\nset fc status=%d\n",status);
	tls_uart_set_flow_ctrl(port, status);
	if(TLS_UART_FLOW_CTRL_HARDWARE == port->opts.flow_ctrl && 0 == status && port->hw_stopped)	//准备关闭流控时，发现tx已经停止，需要再打开tx
	{
		tls_uart_tx_enable(port);
		tls_uart_tx_chars(port);
		port->hw_stopped = 0;
	}
}

void tls_uart_rx_disable(struct tls_uart_port *port)
{
    u32 ucon;

    ucon = port->regs->UR_LC;
    ucon &= ~ULCON_RX_EN;
    port->regs->UR_LC = ucon;
}

void tls_uart_rx_enable(struct tls_uart_port *port)
{
    port->regs->UR_LC |= ULCON_RX_EN; 
}

static void tls_uart_tx_enable(struct tls_uart_port *port)
{
    u32 ucon;

//	printf("tx e\n");
    ucon = port->regs->UR_LC;
    ucon |= ULCON_TX_EN;
    port->regs->UR_LC = ucon;
}

static void tls_uart_tx_disable(struct tls_uart_port *port)
{
    u32 ucon;

//	printf("tx d\n");
    ucon = port->regs->UR_LC;
    ucon &= ~ULCON_TX_EN;
    port->regs->UR_LC = ucon;
}


int tls_uart_config(struct tls_uart_port *port, 
        struct tls_uart_options *opts)
{
	if(NULL == port || NULL == opts)
		return WM_FAILED;
    /* set the baud rate */
    tls_uart_set_baud_rate(port, opts->baudrate);
    /* set the parity */
    tls_uart_set_parity(port, opts->paritytype);
    /* set the number of data bits */
    tls_uart_set_data_bits(port, opts->charlength);
    /* set the number of stop bits */
    tls_uart_set_stop_bits(port, opts->stopbits);
    /* Set flow control */
    port->opts.flow_ctrl = opts->flow_ctrl;
    tls_uart_set_flow_ctrl(port, opts->flow_ctrl);

    /* config uart interrupt register */
/*    if (port->uart_mode == TLS_UART_MODE_INT) *///默认使用中断的方式
	{
        /* clear interrupt */
        port->regs->UR_INTS = 0xFFFFFFFF;
        /* enable interupt */
        port->regs->UR_INTM = 0x0;
        port->regs->UR_DMAC = (4UL << UDMA_RX_FIFO_TIMEOUT_SHIFT) |UDMA_RX_FIFO_TIMEOUT ;
    	}

    /* config FIFO control */
    port->regs->UR_FIFOC = UFC_TX_FIFO_LVL_16_BYTE | UFC_RX_FIFO_LVL_16_BYTE | 
                           UFC_TX_FIFO_RESET | UFC_RX_FIFO_RESET;
    port->regs->UR_LC &= ~(ULCON_TX_EN|ULCON_RX_EN);
    port->regs->UR_LC |= ULCON_RX_EN|ULCON_TX_EN;

    return WM_SUCCESS;
}

/**
 *	uart_handle_cts_change - handle a change of clear-to-send state
 *	@port: uart_port structure for the open port
 *	@status: new clear to send status, nonzero if active
 */
static void uart_handle_cts_change(struct tls_uart_port *port, 
        unsigned int status)
{
//printf("\ncts change port->fcStatus=%d,port->opts.flow_ctrl=%d,status=%d\n",port->fcStatus,port->opts.flow_ctrl,status);
	if (((1 == port->fcStatus) && (port->opts.flow_ctrl == TLS_UART_FLOW_CTRL_HARDWARE)) && 
            (port->uart_no == TLS_UART_1)) {
		if (port->hw_stopped) {
			if (status) {
				port->hw_stopped = 0;
                tls_uart_tx_enable(port);
                tls_uart_tx_chars(port);
			}
		} else {
			if (!status) {
				port->hw_stopped = 1;
                tls_uart_tx_disable(port); 
			}
		}
	}
}

int tls_uart_poll_put_char(struct tls_uart_port *port, int ch)
{
    if (ch == '\n')  {
        while (port->regs->UR_FIFOS & 0x3F) /* wait THR is empty */
            ;
        port->regs->UR_TXW = '\r';         /* output CR */
    }
    while (port->regs->UR_FIFOS & 0x3F)	   /* wait THR is empty */
        ;
    port->regs->UR_TXW = (char)ch;

    return ch;
}

int tls_uart_poll_get_char(struct tls_uart_port *port)
{
    while ((port->regs->UR_FIFOS & UFS_RX_FIFO_CNT_MASK)>>6) 
        return (int)port->regs->UR_RXW;
    return 0;
}

/**********************************************************************************************************
* Description: 	This function is used to fill tx buffer.
*
* Arguments  : 	port			is the uart port
* 				buf			is the user buffer.
*				count		is the user data length
*
* Returns    : 	
**********************************************************************************************************/
int tls_uart_write(struct tls_uart_port *port, char *buf, u32 count)
{
    struct tls_uart_circ_buf *xmit;
    int c;
    int ret = 0;

    xmit = &port->xmit;

    if (!xmit->buf)
        return WM_FAILED;

    while (1) {
        c = CIRC_SPACE_TO_END(xmit->head, xmit->tail, TLS_UART_TX_BUF_SIZE);
        if (count < c)
            c = count;
        if (c <= 0)
            break;
        MEMCPY((char *)(xmit->buf + xmit->head), buf, c);
        xmit->head = (xmit->head + c) & (TLS_UART_TX_BUF_SIZE - 1);
        buf += c;
        count -= c;
        ret += c;
    }

    return ret;
}


/**********************************************************************************************************
* Description: 	This function is used to start transfer data.
*
* Arguments  : 	port			is the uart port
*
* Returns    : 	
**********************************************************************************************************/
void tls_uart_tx_chars_start(struct tls_uart_port *port)
{
    struct tls_uart_circ_buf *xmit = &port->xmit;
    int tx_count;
    u32 cpu_sr;
    
    /* send some chars */
    tx_count = 32;
    cpu_sr = tls_os_set_critical();
    while (!uart_circ_empty(xmit) && tx_count-- > 0) {
        /* 检查tx fifo是否已满 */
        if ((port->regs->UR_FIFOS & UFS_TX_FIFO_CNT_MASK) == 
                port->tx_fifofull)
            break;

        port->regs->UR_TXW = xmit->buf[xmit->tail];
        xmit->tail = (xmit->tail + 1) & (TLS_UART_TX_BUF_SIZE - 1);
        port->icount.tx++;
    }
    tls_os_release_critical(cpu_sr);
    return;
}

/**********************************************************************************************************
* Description: 	This function is used to continue transfer data.
*
* Arguments  : 	port			is the uart port
*
* Returns    : 	
**********************************************************************************************************/
static void tls_uart_tx_chars(struct tls_uart_port *port)
{
    struct tls_uart_circ_buf *xmit = &port->xmit;
    int tx_count = 64;
    int len;

    if (uart_circ_empty(xmit)) {
       // tls_uart_tx_disable(port);

	if(port->tx_sem)
		tls_os_sem_release(port->tx_sem);

        return;
    }

    while (!uart_circ_empty(xmit) && tx_count-- > 0) {
        /* 检查tx fifo是否已满 */
        if ((port->regs->UR_FIFOS & UFS_TX_FIFO_CNT_MASK) == 
                port->tx_fifofull)
            break;

        port->regs->UR_TXW = xmit->buf[xmit->tail];
        xmit->tail = (xmit->tail + 1) & (TLS_UART_TX_BUF_SIZE - 1);
        port->icount.tx++;
    }

    len = uart_circ_chars_pending(xmit);
    if (len < WAKEUP_CHARS) {
		if(port->tx_callback)
			port->tx_callback(port);
    }

#if 0
    if (uart_circ_empty(xmit))
        tls_uart_tx_disable(port);
#endif
}


static int tls_rx_len = 0;

int get_rx_len(void)
{
	return tls_rx_len;
}

void clear_rx_len(void)
{
	tls_rx_len = 0;
}

void tls_set_uart_rx_status(int uart_no,int status)
{
	u32  cpu_sr;
	struct tls_uart_port *port;
	
	if(TLS_UART_1 == uart_no)
	{
		port = &uart_port[1];
		//TLS_DBGPRT_INFO("port%d set rx status=%d,prev rxstatus=%d\n",uart_no,status,port->rxstatus);
		if((TLS_UART_RX_DISABLE == port->rxstatus &&TLS_UART_RX_DISABLE == status) 
			|| (TLS_UART_RX_ENABLE == port->rxstatus &&TLS_UART_RX_ENABLE == status))
			return;
			
		if(TLS_UART_RX_DISABLE == status)
		{
			//TLS_DBGPRT_INFO("\nopts flowctrl=%d,fcstatus=%d\n",port->opts.flow_ctrl,port->fcStatus);
			if((TLS_UART_FLOW_CTRL_HARDWARE == port->opts.flow_ctrl) && (TLS_UART_FLOW_CTRL_HARDWARE == port->fcStatus))
			{				
				cpu_sr = tls_os_set_critical();
				port->regs->UR_INTM |= ((0x1 << 2) | (0x01 <<8));	//关rxfifo trigger level interrupt和overrun error
				port->rxstatus = TLS_UART_RX_DISABLE;
				tls_os_release_critical(cpu_sr);				
				//TLS_DBGPRT_INFO(" rx int mask status=%d\n",port->rxstatus);
			}
		}
		else
		{			
			//TLS_DBGPRT_INFO("start  unmask\n");
			cpu_sr = tls_os_set_critical();
			uart_port[1].regs->UR_INTM &= ~((0x1 << 2) | (0x01 <<8));
			port->rxstatus = TLS_UART_RX_ENABLE;
			tls_os_release_critical(cpu_sr);			
			//TLS_DBGPRT_INFO("rx int unmask status=%d\n",port->rxstatus);
		}
	}
}


void tls_uart_isr(void *data)
{
	struct tls_uart_port *port = (struct tls_uart_port *)data;
	struct tls_uart_circ_buf *recv = &port->recv;
	u32 intr_src;
	u32 rx_fifocnt;
	u32 fifos;
	u8  ch;
	u8  escapefifocnt = 0;

	/* check interrupt status */
	intr_src = port->regs->UR_INTS;
	/* clear uart interrupt */
	port->regs->UR_INTS = intr_src;

	if ((intr_src & UART_RX_INT_FLAG) && (0 == (port->regs->UR_INTM &UIS_RX_FIFO) )) 
	{
		rx_fifocnt = (port->regs->UR_FIFOS >> 6) & 0x3F;
		tls_rx_len += rx_fifocnt;
		escapefifocnt = rx_fifocnt;
		while(rx_fifocnt-- > 0)
		{
			ch = (u8)port->regs->UR_RXW;
			/* break, stop bit error  parity error, not include overrun err */
	            	if(intr_src & UART_RX_ERR_INT_FLAG) 
	            	{
	                	port->regs->UR_INTS |= UART_RX_ERR_INT_FLAG;
				TLS_DBGPRT_INFO("\nrx err=%x,c=%d,ch=%x\n",intr_src,rx_fifocnt,ch);
	                	/*  not insert to buffer */
	                	continue;
			}
			//if (CIRC_SPACE(recv->head, recv->tail, TLS_UART_RX_BUF_SIZE) == 0) 
			if (CIRC_SPACE(recv->head, recv->tail, TLS_UART_RX_BUF_SIZE) <= 2) 
			{
                		port->icount.buf_overrun++;					
				TLS_DBGPRT_INFO("\nrx buf overrun int_src=%x\n",intr_src);
				if(TLS_UART_FLOW_CTRL_HARDWARE == port->fcStatus)
				{
					tls_set_uart_rx_status(port->uart_no,TLS_UART_RX_DISABLE);
					rx_fifocnt = 0;		//如果有硬件流控，关闭接收，把最后一个字符放进环形buffer中
				}
				else
					break;
            		}
			
	        	/* insert the character into the buffer */
			recv->buf[recv->head] = ch;
	        	recv->head = (recv->head + 1) & (TLS_UART_RX_BUF_SIZE - 1);
	        	port->icount.rx++;
	/* process escape char "+++" ，"+++"是一个比较特殊的字符串，用于系统中透传模式的退出*/
	           	if(ch == '+'){
		             port->plus_char_cnt++;
		             if(port->plus_char_cnt == 3) {if(escapefifocnt != 3)port->plus_char_cnt = 0;}
	             	}else{port->plus_char_cnt = 0;}
        	}
		if(port->ifusermode && 3 == port->plus_char_cnt)	//如果是user模式，收到+++退出该模式
		{
			tls_uart_disable_user_mode();
		}
		if(port->rx_callback != NULL)
				port->rx_callback((char *)(recv->buf + recv->tail), (u16)port->icount.rx);
		if(port->ifusermode)	//用户模式下把buffer内容传出去，每次从头开始，避免环形buffer tail>head
		{
			memset(recv->buf, 0, port->icount.rx);
			recv->tail = 0;
			recv->head = 0;
			port->icount.rx = 0;
		}
	}

	if (intr_src & UART_TX_INT_FLAG) 
	{
		tls_uart_tx_chars(port);
	}

	if(intr_src & UIS_CTS_CHNG) 
	{
		fifos = port->regs->UR_FIFOS;
		uart_handle_cts_change(port, fifos & UFS_CST_STS); 
	}
    	return;
}

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
int tls_uart_port_init(int uart_no, tls_uart_options_t *opts)
{
	struct tls_uart_port *port;
	int ret;
	char *bufrx,*buftx;
	tls_uart_options_t opt;

	UartRegInit(uart_no);
	
	if(TLS_UART_0 == uart_no)
	{
		memset(&uart_port[0], 0, sizeof(struct tls_uart_port));
		port = &uart_port[0];
		port->regs = (TLS_UART_REGS_T *)HR_UART0_BASE_ADDR;
	}
	else if(TLS_UART_1 == uart_no)
	{
		memset(&uart_port[1], 0, sizeof(struct tls_uart_port)); 
		port = &uart_port[1];
		port->regs = (TLS_UART_REGS_T *)HR_UART1_BASE_ADDR;
	}
	else
	{
		return WM_FAILED;
	}
	
 	port->uart_no = uart_no;

	if(NULL == opts)
	{
		opt.baudrate = UART_BAUDRATE_B115200;
		opt.charlength = TLS_UART_CHSIZE_8BIT;
		opt.flow_ctrl = TLS_UART_FLOW_CTRL_NONE;
		opt.paritytype = TLS_UART_PMODE_DISABLED;
		opt.stopbits = TLS_UART_ONE_STOPBITS;
		ret = tls_uart_config(port, &opt);
	}
	else
	{
		ret = tls_uart_config(port, opts);
	}
	
	if(ret != WM_SUCCESS)
		return WM_FAILED;
	port->rxstatus = TLS_UART_RX_ENABLE;
	port->uart_irq_no = (uart_no == TLS_UART_0) ? UART0_INT : UART1_INT;

	bufrx = tls_mem_alloc(TLS_UART_RX_BUF_SIZE);
	if (!bufrx)
            return WM_FAILED;
	memset(bufrx, 0, TLS_UART_RX_BUF_SIZE);
 	port->recv.buf = (u8 *)bufrx;
 	port->recv.head = 0;
 	port->recv.tail = 0;
 	buftx = tls_mem_alloc(TLS_UART_TX_BUF_SIZE);
	if (!buftx)
	{
		tls_mem_free(bufrx);
		return WM_FAILED;
	}
	memset(buftx, 0, TLS_UART_TX_BUF_SIZE);
	port->xmit.buf = (u8 *)buftx;
	port->xmit.head = 0;
	port->xmit.tail = 0;
	port->tx_fifofull = 16;

	/* request interrupt */
	tls_irq_register_handler(port->uart_irq_no, tls_uart_isr, port);
	/* enable uart interrupt */
	tls_irq_enable(port->uart_irq_no);

	return WM_SUCCESS;
}

/**********************************************************************************************************
* Description: 	This function is used to register uart rx interrupt.
*
* Arguments  : 	callback		is the uart rx interrupt call back function.
*
* Returns    : 
*
**********************************************************************************************************/
void tls_uart_rx_register(u16 uart_no, s16 (*rx_callback)(char *buf, u16 len))
{
	if(TLS_UART_0 == uart_no)
		uart_port[0].rx_callback = rx_callback;
	else if(TLS_UART_1 == uart_no)
		uart_port[1].rx_callback = rx_callback;
}

/**********************************************************************************************************
* Description: 	This function is used to register uart tx interrupt.
*
* Arguments  : 	callback		is the uart tx interrupt call back function.
*
* Returns    : 
*
**********************************************************************************************************/
void tls_uart_tx_register(u16 uart_no, s16 (*tx_callback)(struct tls_uart_port *port))
{
	if(TLS_UART_0 == uart_no)
		uart_port[0].tx_callback = tx_callback;
	else if(TLS_UART_1 == uart_no)
		uart_port[1].tx_callback = tx_callback;
}

/**********************************************************************************************************
* Description: 	This function is used to copy circular buffer data to user buffer.
*
* Arguments  : 	buf		is the user buffer
*				bufsize	is the user buffer size
*				recv		is the uart rx circular buffer
*
* Returns    : 		copy data size
*
**********************************************************************************************************/
int tls_uart_copy_rxbuf(u8 *buf, int bufsize, struct tls_uart_circ_buf *recv)
{
	int data_cnt,buflen,bufcopylen;
	
	if(NULL == buf || NULL == recv)
		return WM_FAILED;

	data_cnt = CIRC_CNT(recv->head, recv->tail, TLS_UART_RX_BUF_SIZE);
	//TLS_DBGPRT_INFO("\ndata cnt=%d\n",data_cnt);
	if (data_cnt >= bufsize) {
		buflen = bufsize;
	}
	else
	{
    		buflen = data_cnt;
	}
	if ((recv->tail + buflen) > TLS_UART_RX_BUF_SIZE)
	{
		bufcopylen = (TLS_UART_RX_BUF_SIZE - recv->tail);
		MEMCPY(buf, recv->buf + recv->tail, bufcopylen);
		MEMCPY(buf + bufcopylen, recv->buf, buflen - bufcopylen);
	}
	else
	{
    		MEMCPY(buf, recv->buf + recv->tail, buflen);
	}
	recv->tail = (recv->tail + buflen) & (TLS_UART_RX_BUF_SIZE - 1);
	return buflen;
}


u8 *testbuf;
u16 flag = 0;
u16 datalen = 0;

#ifdef TEST_UART
/**********************************************************************************************************
* Description: 	This function is used to receive uart data.
*
* Arguments  : 	buf			is a buf for saving received data. 
*
* Returns    : 	Return received data length.
* Notes	     : 	
**********************************************************************************************************/
s16 tls_uart_rx_test(char *buf, u16 len)
{

	if(datalen < (4096 - len))
	{
		MEMCPY(testbuf + datalen, buf, len);
		flag = 1;
		datalen += len;
	}
	else
	{
		TLS_DBGPRT_INFO("\rx buffer over\n");
	}
	return WM_SUCCESS;
}


void tls_uart_test(void)
{
	int len = 0;
	
	TLS_DBGPRT_INFO("\nentry tls_uart_test\n");
	testbuf = tls_mem_alloc(4096);
  	if (!testbuf)
     		return;
	 memset(testbuf, 0, 4096);
	tls_uart_cfg_user_mode();
	tls_user_uart_rx_register( tls_uart_rx_test);
#if 0	
	tls_user_uart_set_baud_rate(UART_BAUDRATE_B38400);
	tls_user_uart_set_stop_bits(TLS_UART_ONE_STOPBITS);
	tls_user_uart_set_parity(TLS_UART_PMODE_DISABLED);
#endif
	TLS_DBGPRT_INFO("\nentry while\n");
	while(1)
	{
		//TLS_DBGPRT_INFO("\n==============flag=%d\n",flag);
		
		OSTimeDly(0x100);
		if(flag)
		{
			TLS_DBGPRT_INFO("\n\rrx data len = %d\n%s\n\r",datalen,testbuf);
			//tls_uart_tx("\n\r",2);
			tls_uart_tx_sync(testbuf,datalen);
			len = tls_uart_tx_length();
			printf("\ntx len=%d\n",len);
			
			len = tls_uart_tx_length();
			printf("\ntx len 0=%d\n",len);
			//tls_uart_tx("\n\r",2);
			tls_uart_tx_sync("abcde",5);
			len = tls_uart_tx_length();
			printf("\ntx len 1=%d\n",len);
			flag = 0;
			datalen = 0;
		}
	//	len = tls_uart_tx_length();
	//	printf("\ntx len 2=%d\n",len);

	}
	
}

#endif
/**********************************************************************************************************
* Description: 	This function is used to config uart port for user mode.
*
* Arguments  : 	
*
* Returns    : 	
* Notes      : 	If user want to use uart function,must call this function.
**********************************************************************************************************/
void tls_uart_cfg_user_mode(void)
{
	struct tls_uart_port *port = &uart_port[1];

	port->ifusermode = 1;
	memset(port->recv.buf, 0, TLS_UART_RX_BUF_SIZE);
	port->recv.head = 0;
	port->recv.tail = 0;
	port->icount.rx = 0;
	
	port->rx_callback_bk = port->rx_callback;
	port->rx_callback = NULL;
	
	port->tx_callback_bk = port->tx_callback;
	port->tx_callback = NULL;
}

/**********************************************************************************************************
* Description: 	This function is used to disable uart user mode.
*
* Arguments  : 	
*
* Returns    : 	
* Notes      : 	If user want to resume uart old function for AT commond,must call this function.
**********************************************************************************************************/
void tls_uart_disable_user_mode(void)
{
	struct tls_uart_port *port = &uart_port[1];

	port->ifusermode = 0;
	memset(port->recv.buf, 0, TLS_UART_RX_BUF_SIZE);
	port->recv.head = 0;
	port->recv.tail = 0;

	port->rx_callback = port->rx_callback_bk;
	port->rx_callback_bk = NULL;
	
	port->tx_callback = port->tx_callback_bk;
	port->tx_callback_bk = NULL;
}

static s16 tls_uart_tx_cb(struct tls_uart_port *port)
{
	int ret_len = 0;
	
	if(NULL == port)
		return WM_FAILED;
	if(port->buf_len > 0)		//uart1,缓冲区数据传输完成之后，剩余的待传的数据再放入缓冲区
	{
	    	ret_len = tls_uart_write(port, port->buf_ptr, port->buf_len);
		//TLS_DBGPRT_INFO("\ntx cb write len=%d",ret_len);

		if(ret_len >= 0)
		{
			if(port->buf_len >= ret_len)
			{
				port->buf_len -= ret_len;	
				port->buf_ptr += ret_len;
			}
		}
	}
	
	return WM_SUCCESS;
}

/**********************************************************************************************************
* Description: 	This function is used to transfer data asynchronous.
*
* Arguments  : 	buf			is a buf for saving user data. 
* 				len			is the data length.
*
* Returns    : 		
* Notes		:	The function only start transmission, fill buffer in the callback function.
**********************************************************************************************************/
int tls_uart_tx(char *buf, u16 len)
{
	struct tls_uart_port *port = &uart_port[1];
	int ret_len;

	if(NULL == buf || len < 1)
		return WM_FAILED;

	port->icount.tx = 0;
	tls_uart_tx_register(TLS_UART_1, tls_uart_tx_cb);
	port->buf_ptr = buf;
	port->buf_len= len;
	
	ret_len = tls_uart_write(port, port->buf_ptr, port->buf_len);
	//TLS_DBGPRT_INFO("\ntx write len=%d",ret_len);
	if(ret_len >= 0)
	{
		if(port->buf_len >= ret_len)
		{
			port->buf_ptr += ret_len;
			port->buf_len -= ret_len;
			//TLS_DBGPRT_INFO("\start uart1 tx\n");
			tls_uart_tx_chars_start(port);
		}
	}

	return WM_SUCCESS;
}

//获取tx的数据长度
int tls_uart_tx_length(void)
{
	struct tls_uart_port *port = &uart_port[1];

	return port->icount.tx;
}

/**********************************************************************************************************
* Description: 	This function is used to transfer data synchronous.
*
* Arguments  : 	buf			is a buf for saving user data. 
* 				len			is the data length.
*
* Returns    : 		
* Notes		:	
**********************************************************************************************************/
int tls_uart_tx_sync(char *buf, u16 len)
{
	struct tls_uart_port *port = &uart_port[1];
	u8 err;	

	err = tls_os_sem_create(&port->tx_sem, 0);
	if (err != TLS_OS_SUCCESS) 
	{
		TLS_DBGPRT_ERR("\ntx sem create fail\n");
		return WM_FAILED;
	}

	tls_uart_tx(buf, len);

	tls_os_sem_acquire(port->tx_sem, 0);
	tls_os_sem_delete(port->tx_sem);
	port->tx_sem = NULL;
	
	return WM_SUCCESS;
}


/**********************************************************************************************************
* Description: 	This function is used to register uart rx interrupt.
*
* Arguments  : 	callback		is the uart rx interrupt call back function.
*
* Returns    : 
*
**********************************************************************************************************/
void tls_user_uart_rx_register(s16 (*rx_callback)(char *buf, u16 len))
{
	tls_uart_rx_register(TLS_UART_1, rx_callback);
}

/**********************************************************************************************************
* Description: 	This function is used to register uart tx interrupt.
*
* Arguments  : 	callback		is the uart tx interrupt call back function.
*
* Returns    : 
*
**********************************************************************************************************/
void tls_user_uart_tx_register(s16 (*tx_callback)(struct tls_uart_port *port))
{
	tls_uart_tx_register(TLS_UART_1, tx_callback);
}

/**********************************************************************************************************
* Description: 	This function is used to set uart parity.
*
* Arguments  : 	paritytype		is a parity type defined in TLS_UART_PMODE_T
*
* Returns    :  WM_SUCCESS	if setting success
* 			WM_FAILED	if setting fail
**********************************************************************************************************/
int tls_user_uart_set_parity(TLS_UART_PMODE_T paritytype)
{
	return tls_uart_set_parity(&uart_port[1], paritytype);
}

/**********************************************************************************************************
* Description: 	This function is used to set uart baudrate.
*
* Arguments  : 	baudrate		is the baudrate user want used,the unit is HZ.
*
* Returns    :  WM_SUCCESS	if setting success
* 			WM_FAILED	if setting fail
**********************************************************************************************************/
int tls_user_uart_set_baud_rate(u32 baudrate)
{
	return tls_uart_set_baud_rate(&uart_port[1], baudrate);
}

/**********************************************************************************************************
* Description: 	This function is used to set uart stop bits.
*
* Arguments  : 	baudrate		is a stop bit type defined in TLS_UART_STOPBITS_T
*
* Returns    :  WM_SUCCESS	if setting success
* 			WM_FAILED	if setting fail
**********************************************************************************************************/
int tls_user_uart_set_stop_bits(TLS_UART_STOPBITS_T stopbits)
{
	return tls_uart_set_stop_bits(&uart_port[1], stopbits);
}

#endif 
//TLS_CONFIG_UART
