#ifndef __WM_DMA_H_
#define __WM_DMA_H_

#define TLS_DMA_SEL_UART0_RX      0
#define TLS_DMA_SEL_UART0_TX      1
#define TLS_DMA_SEL_UART1_RX      2
#define TLS_DMA_SEL_UART1_TX      3
#define TLS_DMA_SEL_RSV1      4
#define TLS_DMA_SEL_RSV2      5
#define TLS_DMA_SEL_LSSPI_RX      6
#define TLS_DMA_SEL_LSSPI_TX      7
#define TLS_DMA_SEL_SDADC_CH0      8
#define TLS_DMA_SEL_SDADC_CH1      9
#define TLS_DMA_SEL_SDADC_CH2      10
#define TLS_DMA_SEL_SDADC_CH3      11

#define TLS_DMA_FLAGS_HARD_MODE      (1<<0)
#define TLS_DMA_FLAGS_CHAIN_MODE      (1<<1)
#define TLS_DMA_FLAGS_CHANNEL_SEL(n)      ((n)<<2)
#define TLS_DMA_FLAGS_CHAIN_LINK_EN      (1<<6)

#define TLS_DMA_FLAGS_CHANNEL_VALID      (1<<7)

typedef struct __TLS_DMA_CHANNELS {
	unsigned char	channels[8];	/* list of channels */
}TLS_DMA_CHANNELS, *PTLS_DMA_CHANNELS;

#define TLS_DMA_DESC_VALID      (1U<<31)

#define TLS_DMA_DESC_CTRL_SRC_ADD_INC      (1<<0)
#define TLS_DMA_DESC_CTRL_DEST_ADD_INC      (1<<1)

#define TLS_DMA_DESC_CTRL_DATA_SIZE_BYTE      (0<<2)
#define TLS_DMA_DESC_CTRL_DATA_SIZE_SHORT      (1<<2)
#define TLS_DMA_DESC_CTRL_DATA_SIZE_WORD      (2<<2)

#define TLS_DMA_DESC_CTRL_BURST_SIZE1      (0<<4)
#define TLS_DMA_DESC_CTRL_BURST_SIZE4      (1<<4)

#define TLS_DMA_DESC_CTRL_TOTAL_BYTES(n)      ((n)<<5)
typedef struct __TLS_DMA_DESCRIPTOR
{
	unsigned int valid;
	unsigned int dma_ctrl;
	unsigned int src_addr;
	unsigned int dest_addr;
	struct __TLS_DMA_DESCRIPTOR * next;
}TLS_DMA_DESCRIPTOR, *PTLS_DMA_DESCRIPTOR;

int tlsDmaWaitComplt(unsigned ch);
unsigned char tlsDmaStart(unsigned char ch, PTLS_DMA_DESCRIPTOR pDmaDesc, unsigned char autoReload);
unsigned char tlsDmaStop(unsigned char ch);
unsigned char tlsDmaRequest(unsigned char ch, unsigned char flags);
void tlsDmaFree(unsigned char ch);
void tlsDmaInit(void);

#endif /* __TLS_DMA_H_151606__ */
 
