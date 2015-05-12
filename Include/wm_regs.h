#ifndef WM_REGS_H
#define WM_REGS_H


typedef volatile unsigned int TLS_REG;    /* Hardware register definition */

#define APB_CLK      (40000000) /* 40MHz */

/***************************************************************
 * SDIOģ��Ĵ�������
 ***************************************************************/
#define HR_SDIO_BASE_ADDR     0x0E000000
#define HR_SDIO_CIS0                (HR_SDIO_BASE_ADDR + 0x008)
#define HR_SDIO_CIS1                (HR_SDIO_BASE_ADDR + 0x00C)
#define HR_SDIO_CSA                 (HR_SDIO_BASE_ADDR + 0x010)
#define HR_SDIO_READ                (HR_SDIO_BASE_ADDR + 0x014)
#define HR_SDIO_WRITE               (HR_SDIO_BASE_ADDR + 0x018)
#define HR_SDIO_INTEN               (HR_SDIO_BASE_ADDR + 0x030)
#define HR_SDIO_OCR                 (HR_SDIO_BASE_ADDR + 0x034)
#define HR_SDIO_CIA                 (HR_SDIO_BASE_ADDR + 0x024)
#define HR_SDIO_PROG                (HR_SDIO_BASE_ADDR + 0x028)

/***************************************************************
 * SPIģ��Ĵ�������
 ***************************************************************/
#define HR_HSPI_BASE_ADDR       (0x0E000200)
#define HR_HSPI_CLEAR_FIFO          (HR_HSPI_BASE_ADDR)
#define HR_HSPI_SPI_CFG             (HR_HSPI_BASE_ADDR + 0x04)
#define HR_HSPI_MODE_CFG            (HR_HSPI_BASE_ADDR + 0x08)
#define HR_HSPI_INT_MASK            (HR_HSPI_BASE_ADDR + 0x0C)
#define HR_HSPI_INT_STTS            (HR_HSPI_BASE_ADDR + 0x10)
#define HR_HSPI_RXDAT_LEN           (HR_HSPI_BASE_ADDR + 0x18)

/***************************************************************
 * SDIO WRAPPER�Ĵ�������
 ***************************************************************/
#define HR_SDIO_WRAPPER_BASE_ADDR        (0x0E000300)
#define HR_SDIO_INT_SRC             (HR_SDIO_WRAPPER_BASE_ADDR + 0x000)
#define HR_SDIO_INT_MASK            (HR_SDIO_WRAPPER_BASE_ADDR + 0x004)
#define HR_SDIO_UPCMDVALID          (HR_SDIO_WRAPPER_BASE_ADDR + 0x008)
#define HR_SDIO_DOWNCMDVALID        (HR_SDIO_WRAPPER_BASE_ADDR + 0x00C)
#define HR_SDIO_TXBD_LINKEN         (HR_SDIO_WRAPPER_BASE_ADDR + 0x010)
#define HR_SDIO_TXBD_ADDR           (HR_SDIO_WRAPPER_BASE_ADDR + 0x014)
#define HR_SDIO_TXEN                (HR_SDIO_WRAPPER_BASE_ADDR + 0x018)
#define HR_SDIO_TX_STTS             (HR_SDIO_WRAPPER_BASE_ADDR + 0x01C)
#define HR_SDIO_RXBD_LINKEN         (HR_SDIO_WRAPPER_BASE_ADDR + 0x020)
#define HR_SDIO_RXBD_ADDR           (HR_SDIO_WRAPPER_BASE_ADDR + 0x024)
#define HR_SDIO_RXEN                (HR_SDIO_WRAPPER_BASE_ADDR + 0x028)
#define HR_SDIO_RX_STTS             (HR_SDIO_WRAPPER_BASE_ADDR + 0x02C)
#define HR_SDIO_CMD_ADDR            (HR_SDIO_WRAPPER_BASE_ADDR + 0x030)
#define HR_SDIO_CMD_SIZE            (HR_SDIO_WRAPPER_BASE_ADDR + 0x034)

/* SDIO interrupt bit definition */
#define SDIO_WP_INT_SRC_CMD_DOWN         (1UL<<3)
#define SDIO_WP_INT_SRC_CMD_UP           (1UL<<2)
#define SDIO_WP_INT_SRC_DATA_DOWN        (1UL<<1)
#define SDIO_WP_INT_SRC_DATA_UP          (1UL<<0)


/***************************************************************
 * DMAģ��Ĵ�������
 ***************************************************************/
#define HR_DMA_BASE_ADDR			(0x0E000400)
#define HR_DMA_INT_MASK			(HR_DMA_BASE_ADDR + 0x0)
#define HR_DMA_INT_SRC				(HR_DMA_BASE_ADDR + 0x4)
#define HR_DMA_CHNL0_SRC_ADDR	(HR_DMA_BASE_ADDR + 0x10)
#define HR_DMA_CHNL0_DEST_ADDR	(HR_DMA_BASE_ADDR + 0x14)
#define HR_DMA_CHNL0_CHNL_CTRL	(HR_DMA_BASE_ADDR + 0x18)
#define HR_DMA_CHNL0_DMA_MODE	(HR_DMA_BASE_ADDR + 0x1c)
#define HR_DMA_CHNL0_DMA_CTRL	(HR_DMA_BASE_ADDR + 0x20)
#define HR_DMA_CHNL0_DMA_STATUS	(HR_DMA_BASE_ADDR + 0x24)
#define HR_DMA_CHNL0_LINK_DEST_ADDR	(HR_DMA_BASE_ADDR + 0x28)
#define HR_DMA_CHNL0_CURRENT_DEST_ADDR	(HR_DMA_BASE_ADDR + 0x2C)


#define DMA_STARTADDRESS        			(0x0E000400)
#define DMA_INTMASK_REG      	  		(*(volatile unsigned int*)(DMA_STARTADDRESS+0x000))
#define DMA_INTSRC_REG      	  			(*(volatile unsigned int*)(DMA_STARTADDRESS+0x004))

#define DMA_CHNL_REG_BASE                      (DMA_STARTADDRESS+0x010)
#define DMA_SRCADDR_REG(ch)      	  	(*(volatile unsigned int*)(DMA_CHNL_REG_BASE + 0x20 * (ch /*- 1*/) +0x00))
#define DMA_DESTADDR_REG(ch)      	  	(*(volatile unsigned int*)(DMA_CHNL_REG_BASE + 0x20 * (ch /*- 1*/) +0x04))
#define DMA_CHNLCTRL_REG(ch)      	  	(*(volatile unsigned int*)(DMA_CHNL_REG_BASE + 0x20 * (ch /*- 1*/) +0x08))
#define DMA_MODE_REG(ch)      	  		(*(volatile unsigned int*)(DMA_CHNL_REG_BASE + 0x20 * (ch /*- 1*/) +0x0C))
#define DMA_CTRL_REG(ch)      	  		(*(volatile unsigned int*)(DMA_CHNL_REG_BASE + 0x20 * (ch /*- 1*/) +0x10))
#define DMA_STATUS_REG(ch)      	  		(*(volatile unsigned int*)(DMA_CHNL_REG_BASE + 0x20 * (ch /*- 1*/) +0x14))
#define DMA_CURRSRCADDR_REG(ch)      	(*(volatile unsigned int*)(DMA_CHNL_REG_BASE + 0x20 * (ch /*- 1*/) +0x18))
#define DMA_CURRDESTADDR_REG(ch)      	(*(volatile unsigned int*)(DMA_CHNL_REG_BASE + 0x20 * (ch /*- 1*/) +0x1C))

#define DMA_CHNL_CTRL_CHNL_ON               (1<<0)
#define DMA_CHNL_CTRL_CHNL_OFF             (1<<1)

#define DMA_MODE_HARD_MODE                   (1<<0)
#define DMA_MODE_CHAIN_MODE                  (1<<1)
#define DMA_MODE_SEL_UART0_RX               (0<<2)
#define DMA_MODE_SEL_UART0_TX               (1<<2)
#define DMA_MODE_SEL_UART1_RX               (2<<2)
#define DMA_MODE_SEL_UART1_TX               (3<<2)
#define DMA_MODE_SEL_RSV1                       (4<<2)
#define DMA_MODE_SEL_RSV2                       (5<<2)
#define DMA_MODE_SEL_LSSPI_RX                (6<<2)
#define DMA_MODE_SEL_LSSPI_TX                (7<<2)
#define DMA_MODE_SEL_SDADC_CH0            (8<<2)
#define DMA_MODE_SEL_SDADC_CH1            (9<<2)
#define DMA_MODE_SEL_SDADC_CH2            (10<<2)
#define DMA_MODE_SEL_SDADC_CH3            (11<<2)
#define DMA_MODE_CHAIN_LINK_EN             (1<<6)

#define DMA_CTRL_AUTO_RELOAD                 (1<<0)
#define DMA_CTRL_SRC_ADDR_INC                (1<<1)
#define DMA_CTRL_DEST_ADDR_INC              (1<<2)
#define DMA_CTRL_DATA_SIZE_BYTE            (0<<3)
#define DMA_CTRL_DATA_SIZE_SHORT          (1<<3)
#define DMA_CTRL_DATA_SIZE_WORD           (2<<3)
#define DMA_CTRL_BURST_SIZE1                   (0<<5)
#define DMA_CTRL_BURST_SIZE4                   (4<<5)
#define DMA_CTRL_TOTLAL_SIZE(n)               (n<<6)


/***************************************************************
 * ����ģ��Ĵ�������
 ***************************************************************/
#define HR_PMU_BASE_ADDR            0x0E000600
#define HR_PMU_WLAN_SLEEP_EN        (HR_PMU_BASE_ADDR + 0x0)
#define HR_PMU_WLAN_POWER_DOWN      (HR_PMU_BASE_ADDR + 0x4)
#define HR_PMU_TIMER0_STANDBY       (HR_PMU_BASE_ADDR + 0x1C)

#define HR_PMU_TIMER0_SLEEP         (HR_PMU_BASE_ADDR + 0x20)
#define HR_PMU_PRETBTT_PERIOD       (HR_PMU_BASE_ADDR + 0x24)
#define HR_PMU_STANDBY_WAKE_EN      (HR_PMU_BASE_ADDR + 0x2C)
#define HR_PMU_PRETBTT_OFFSET       (HR_PMU_BASE_ADDR + 0x28)
#define HR_PMU_WAKEUP_ENABLE        (HR_PMU_BASE_ADDR + 0x30)
#define HR_PMU_RTC_CTRL1        (HR_PMU_BASE_ADDR + 0x34)
#define HR_PMU_RTC_CTRL2        (HR_PMU_BASE_ADDR + 0x38)
#define HR_PMU_INTERRUPT_SRC        (HR_PMU_BASE_ADDR + 0x3C)
#define HR_PMU_INTERRUPT_MASK       (HR_PMU_BASE_ADDR + 0x40)
#define HR_PMU_BYPASS_CTRL          (HR_PMU_BASE_ADDR + 0x48)
#define HR_PMU_WLAN_TX_RX_IDLE      (HR_PMU_BASE_ADDR + 0x54)
/* ���ܼĴ���λ���� */
#define PMU_WLAN_SLEEP_EN_BIT           (1UL << 0)

#define PMU_PRETBTT_PERIOD_RSV          (0xE0000000)
#define PMU_TBTT_TIMER_MASK             (0x3FFFF)
#define PMU_TBTT_TIMER_SHIFT            (11)
#define PMU_SLEEP_INTERVAL_MASK         (0x7FF)
#define PMU_SLEEP_INTERVAL_SHIFT        (0)

#define PMU_TBTT_OFFSET_MASK            (0x7FFF)
#define PMU_TBTT_OFFSET_SHIFT           (17)
#define PMU_PRETBTT_TIMER_MASK          (0x1FFFF)
#define PMU_PRETBTT_TIMER_SHIFT         (0)
#define PMU_PRETBTT_OFFSET_DEFAULT      (((30UL) << 17) | (300UL))

#define PMU_TIMER0_WAKEUP_EN            (1UL << 3)
#define PMU_TIMER1_WAKEUP_EN            (1UL << 2)
#define PMU_GPIO_WAKEUP_EN              (1UL << 1)
#define PMU_SDIO_WAKEUP_EN              (1UL << 0)

#define PMU_TIMER0_NTERRUPT_SRC         (1UL << 0)
#define PMU_TIMER1_NTERRUPT_SRC         (1UL << 1)
#define PMU_GPIO_NTERRUPT_SRC           (1UL << 2)
#define PMU_SDIO_NTERRUPT_SRC           (1UL << 3)

#define PMU_TIMER0_NTERRUPT_MASK        (1UL << 0)
#define PMU_TIMER1_NTERRUPT_MASK        (1UL << 1)
#define PMU_GPIO_NTERRUPT_MASK          (1UL << 2)
#define PMU_SDIO_NTERRUPT_MASK          (1UL << 3)

#define PMU_WLAN_RX_IDLE_BIT            (1UL << 1)
#define PMU_WLAN_TX_IDLE_BIT            (1UL << 0)

#define PMU_LDO_DAC28_BYPASS      (1UL<<0)
#define PMU_LDO_SW_BYPASS      (1UL<<1)
#define PMU_DLDO_BYPASS      (1UL<<2)
#define PMU_LDO_PRE_BYPASS      (1UL<<3)
#define PMU_LDO_ADDA_D_BYPASS      (1UL<<4)
#define PMU_LDO_ADDA_A_BYPASS      (1UL<<5)
#define PMU_LDO_UM_PA_BYPASS      (1UL<<6)
#define PMU_LDO_CHP_BYPASS      (1UL<<7)
#define PMU_LDO_DM_IF_BYPASS      (1UL<<8)
#define PMU_LDO_LNA_BYPASS      (1UL<<9)
#define PMU_LDO_VCO_BYPASS      (1UL<<10)
#define PMU_LDO_LO_BYPASS      (1UL<<11)
#define PMU_LDO_PLL_BYPASS      (1UL<<15)
#define PMU_LDO_WLAN_BYPASS      (1UL<<19)
#define PMU_LDO_BGR_BYPASS      (1UL<<20)
#define PMU_LDO_BB_BYPASS      (1UL<<21)

/***************************************************************
 * system clock ��bus clock�Ĵ�������
 ***************************************************************/
#define HR_CLK_BASE_ADDR        (0x0E000700)
#define HR_CLK_RST_CTL          (HR_CLK_BASE_ADDR + 0x0c)
#define HR_CLK_DIV_CTL			(HR_CLK_BASE_ADDR + 0x10)
/***************************************************************
 * �ڴ�ģ��Ĵ�������
 ***************************************************************/
#define HR_MEM_BASE_ADDR    0x0E000800
#define HR_MEM_TXBUF_BASE_ADDR      (HR_MEM_BASE_ADDR + 0x00)
#define HR_MEM_RXBUF_BASE_ADDR      (HR_MEM_BASE_ADDR + 0x04)
#define HR_MEM_BUF_NUM_CFG          (HR_MEM_BASE_ADDR + 0x08)
#define HR_MEM_BUF_SIZE_CFG         (HR_MEM_BASE_ADDR + 0x0C)
#define HR_MEM_AGGR_CFG             (HR_MEM_BASE_ADDR + 0x10)
#define HR_MEM_BUF_EN               (HR_MEM_BASE_ADDR + 0x14)


/***************************************************************
 * �ж�ģ��Ĵ�������
 ***************************************************************/
#define HR_VIC_BASE_ADDR   0xFFFFF000
#define HR_VIC_IRQ_STATUS           (HR_VIC_BASE_ADDR + 0x0)
#define HR_VIC_FIQ_STATUS           (HR_VIC_BASE_ADDR + 0x4)
#define HR_VIC_RAW_INTR             (HR_VIC_BASE_ADDR + 0x8)
#define HR_VIC_INT_SELECT           (HR_VIC_BASE_ADDR + 0xc)
#define HR_VIC_INT_ENABLE           (HR_VIC_BASE_ADDR + 0x10)
#define HR_VIC_INT_EN_CLR           (HR_VIC_BASE_ADDR + 0x14)
#define HR_VIC_SOFT_INT             (HR_VIC_BASE_ADDR + 0x18)
#define HR_VIC_SOFT_INT_CLR         (HR_VIC_BASE_ADDR + 0x1c)
#define HR_VIC_PROTECT              (HR_VIC_BASE_ADDR + 0x20)
#define HR_VIC_VECT_ADDR            (HR_VIC_BASE_ADDR + 0x30)
#define HR_VIC_DEF_VECT_ADDR        (HR_VIC_BASE_ADDR + 0x34)
#define HR_VIC_VECT_ENABLE          (HR_VIC_BASE_ADDR + 0x100)
#define HR_VIC_VECT_BASE_ADDR       (HR_VIC_BASE_ADDR + 0x104)
#define HR_VIC_INT_NUM              (HR_VIC_BASE_ADDR + 0x108)
#define HR_VIC_INT_PRIO0            (HR_VIC_BASE_ADDR + 0x10C)
#define HR_VIC_INT_PRIO1            (HR_VIC_BASE_ADDR + 0x110)
#define HR_VIC_INT_PRIO2            (HR_VIC_BASE_ADDR + 0x114)
#define HR_VIC_INT_PRIO3            (HR_VIC_BASE_ADDR + 0x118)


/* APB����ַ*/
#define HR_APB_BASE_ADDR    0x0F000000
/***************************************************************
 * I2Cģ��Ĵ�������
 ***************************************************************/
#define HR_I2C_BASE_ADDR    (HR_APB_BASE_ADDR)
#define HR_I2C_PRER_LO              (HR_I2C_BASE_ADDR + 0x0)
#define HR_I2C_PRER_HI              (HR_I2C_BASE_ADDR + 0x04)
#define HR_I2C_CTRL                 (HR_I2C_BASE_ADDR + 0x08)
#define HR_I2C_TX_RX                   (HR_I2C_BASE_ADDR + 0x0C)
#define HR_I2C_CR_SR                   (HR_I2C_BASE_ADDR + 0x10)
#define HR_I2C_TXR               (HR_I2C_BASE_ADDR + 0x14)
#define HR_I2C_CR               (HR_I2C_BASE_ADDR + 0x18)


#define I2C_CTRL_INT_DISABLE	(0x1UL<<6)
#define I2C_CTRL_ENABLE			(0x1UL<<7)

#define I2C_CR_STA				(0x1UL<<7)
#define I2C_CR_STO				(0x1UL<<6)
#define I2C_CR_RD				(0x1UL<<5)
#define I2C_CR_WR				(0x1UL<<4)
#define I2C_CR_NAK				(0x1UL<<3)
#define I2C_CR_IACK				(0x1UL)

#define I2C_SR_NAK				(0x1UL<<7)
#define I2C_SR_BUSY					(0x1UL<<6)
#define I2C_SR_TIP				(0x1UL<<1)
#define I2C_SR_IF				(0x1UL)


/***************************************************************
 * SD ADCģ��Ĵ�������
 ***************************************************************/
#define HR_SD_ADC_BASE_ADDR         (HR_APB_BASE_ADDR + 0x200)
#define HR_SD_ADC_RESULT_REG     	((HR_SD_ADC_BASE_ADDR+0x000))
#define HR_SD_ADC_CONFIG_REG     	((HR_SD_ADC_BASE_ADDR+0x004))
#define HR_SD_ADC_CTRL_REG	       ((HR_SD_ADC_BASE_ADDR+0x008))
#define HR_SD_ADC_INT_MASK_REG	((HR_SD_ADC_BASE_ADDR+0x00C))
#define HR_SD_ADC_INT_SOURCE_REG	((HR_SD_ADC_BASE_ADDR+0x010))


/***************************************************************
 * SPIģ��Ĵ�������
 ***************************************************************/
#define HR_SPI_BASE_ADDR            (HR_APB_BASE_ADDR + 0x400)
#define HR_SPI_CHCFG_REG		   		(HR_SPI_BASE_ADDR + 0x000)
#define HR_SPI_SPICFG_REG		   		(HR_SPI_BASE_ADDR + 0x004)
#define HR_SPI_CLKCFG_REG		   		(HR_SPI_BASE_ADDR + 0x008)
#define HR_SPI_MODECFG_REG		   	(HR_SPI_BASE_ADDR + 0x00C)
#define HR_SPI_INT_MASK_REG		   		(HR_SPI_BASE_ADDR + 0x010)
#define HR_SPI_INT_STATUS_REG		   		(HR_SPI_BASE_ADDR + 0x014)
#define HR_SPI_STATUS_REG		   	(HR_SPI_BASE_ADDR + 0x018)
#define HR_SPI_TIMEOUT_REG		   	(HR_SPI_BASE_ADDR + 0x01C)
#define HR_SPI_TXDATA_REG		   		(HR_SPI_BASE_ADDR + 0x020)
#define HR_SPI_TRANSLEN_REG		   	(HR_SPI_BASE_ADDR + 0x024)
#define HR_SPI_RXDATA_REG		   		(HR_SPI_BASE_ADDR + 0x030)


#define SPIM_STARTADDRESS 		  		(0x0F000400)
#define SPIM_CHCFG_REG		   			(*(volatile unsigned int*)(SPIM_STARTADDRESS+0x000))
#define SPIM_SPICFG_REG		   		(*(volatile unsigned int*)(SPIM_STARTADDRESS+0x004))
#define SPIM_CLKCFG_REG		   		(*(volatile unsigned int*)(SPIM_STARTADDRESS+0x008))
#define SPIM_MODECFG_REG		   		(*(volatile unsigned int*)(SPIM_STARTADDRESS+0x00C))
#define SPIM_INTEN_REG		   			(*(volatile unsigned int*)(SPIM_STARTADDRESS+0x010))
#define SPIM_INTSRC_REG		   		(*(volatile unsigned int*)(SPIM_STARTADDRESS+0x014))
#define SPIM_SPISTATUS_REG		   		(*(volatile unsigned int*)(SPIM_STARTADDRESS+0x018))
#define SPIM_SPITIMEOUT_REG		   	(*(volatile unsigned int*)(SPIM_STARTADDRESS+0x01C))
#define SPIM_TXDATA_REG		   		(*(volatile unsigned int*)(SPIM_STARTADDRESS+0x020))
#define SPIM_TRANSLEN_REG		   		(*(volatile unsigned int*)(SPIM_STARTADDRESS+0x024))
#define SPIM_RXDATA_REG		   		(*(volatile unsigned int*)(SPIM_STARTADDRESS+0x030))


/* Bits Definitions of "CH_CFG" */
#define SPI_RX_INVALID_BITS(n)      ((n) << 23)
#define SPI_CLEAR_FIFOS      (1U << 22)
#define SPI_CONTINUE_MODE      (1U << 21)
#define SPI_RX_CHANNEL_ON      (1U << 20)
#define SPI_RX_CHANNEL_OFF      (0U << 20)
#define SPI_TX_CHANNEL_ON      (1U << 19)
#define SPI_TX_CHANNEL_OFF      (0U << 19)
#define SPI_VALID_CLKS_NUM(n)      ((n) << 3)
#define SPI_CS_LOW      (0U << 2)
#define SPI_CS_HIGH      (1U << 2)
#define SPI_FORCE_SPI_CS_OUT      (1U << 1)
#define SPI_START      (1U << 0)

/* Bits Definitions of "SPI_CFG" */
#define SPI_FRAME_FORMAT_MOTO      (0U << 17)
#define SPI_FRAME_FORMAT_TI      (1U << 17)
#define SPI_FRAME_FORMAT_MICROWAIRE      (2U << 17)
#define SPI_TX_PIN_ALWS_DRIVER      (1U << 16)
#define SPI_CS_HOLD(n)      ((n) << 12)
#define CS_HOLD_1_CLKS     0
#define CS_HOLD_2_CLKS     1
#define CS_HOLD_4_CLKS     2
#define CS_HOLD_8_CLKS     3
#define CS_HOLD_16_CLKS     4
#define CS_HOLD_32_CLKS     5
#define CS_HOLD_64_CLKS     6
#define CS_HOLD_127_CLKS     7
#define SPI_CS_SETUP(n)      ((n) << 9)
#define CS_SETUP_1_CLKS     0
#define CS_SETUP_2_CLKS     1
#define CS_SETUP_4_CLKS     2
#define CS_SETUP_8_CLKS     3
#define CS_SETUP_16_CLKS     4
#define CS_SETUP_32_CLKS     5
#define CS_SETUP_64_CLKS     6
#define CS_SETUP_127_CLKS     7
#define SPI_DATA_OUT_DELAY(n)      ((n) << 7)
#define DATA_OUT_DELAY_1_CLKS      0
#define DATA_OUT_DELAY_2_CLKS      1
#define DATA_OUT_DELAY_3_CLKS      2
#define DATA_OUT_DELAY_0_CLKS      3
#define SPI_FRAME_DELAY(n)      ((n) << 4)
#define FRAME_DELAY_1_CLKS     0
#define FRAME_DELAY_2_CLKS     1
#define FRAME_DELAY_4_CLKS     2
#define FRAME_DELAY_8_CLKS     3
#define FRAME_DELAY_16_CLKS     4
#define FRAME_DELAY_32_CLKS     5
#define FRAME_DELAY_64_CLKS     6
#define FRAME_DELAY_127_CLKS     7
#define SPI_LITTLE_ENDIAN      (0 << 3)
#define SPI_BIG_ENDIAN      (1 << 3)
#define SPI_SET_MASTER_SLAVE(mode)      ((mode) << 2)
#define SPI_MASTER      1
#define SPI_SLAVE      0
#define SPI_SET_CPHA(n)      ((n) << 1)
#define SPI_SET_CPOL(n)      ((n) << 0)

/* Bits Definitions of "CLK_CFG" */
#define SPI_GET_SCLK_DIVIDER(clk)      ((APB_CLK)/((clk) * 2) - 1)/*In HZ*/
#define SPI_SCLK_DIVIDER(n)      ((n) << 0)

/* Bits Definitions of "MODE_CFG" */
#define SPI_RX_TRIGGER_LEVEL(n)      ((n) << 6)
#define SPI_TX_TRIGGER_LEVEL(n)      ((n) << 2)
#define SPI_RX_DMA_ON      (1 << 1)
#define SPI_RX_DMA_OFF      (0 << 1)
#define SPI_TX_DMA_ON      (1 << 0)
#define SPI_TX_DMA_OFF      (0 << 0)

/* Bits Definitions of SPI interrupt register(SPI_INT_MASK and SPI_INT_SOURCE) */
#define SPI_INT_MASK_ALL      0xffU
#define SPI_INT_CLEAR_ALL      0xffU
#define SPI_INT_TIMEOUT      (1U << 7)
#define SPI_INT_TRANSFER_DONE      (1U << 6)
#define SPI_INT_RX_OVERRUN      (1U << 5)
#define SPI_INT_RX_UNDERRUN      (1U << 4)
#define SPI_INT_TX_OVERRUN      (1U << 3)
#define SPI_INT_TX_UNDERRUN      (1U << 2)
#define SPI_INT_RX_FIFO_RDY      (1U << 1)
#define SPI_INT_TX_FIFO_RDY      (1U << 0)

/* Bits Definitions of "SPI_STATUS" */
#define SPI_IS_BUSY(value)      (((value) & (1 << 12) != 0) ? 1 : 0)
#define SPI_GET_RX_FIFO_CNT(value)      (((value) & 0xFC0) >> 6)
#define SPI_GET_TX_FIFO_CNT(value)      ((value) & 0x3F)

/* Bits Definitions of "SPI_TIMEOUT" */
#define SPI_TIMER_EN      (1U << 31)
#define SPI_TIME_OUT(n)      (((((n) * (APB_CLK)) / 1000) & ~(0x01U << 31)) << 0)

/***************************************************************
 * UART�Ĵ�������
 ***************************************************************/
#define HR_UART0_BASE_ADDR          (HR_APB_BASE_ADDR + 0x800)
#define HR_UART1_BASE_ADDR          (HR_APB_BASE_ADDR + 0xA00)
#define HR_UART0_LINE_CTRL          (HR_UART0_BASE_ADDR + 0x0)
#define HR_UART0_FLOW_CTRL          (HR_UART0_BASE_ADDR + 0x04)
#define HR_UART0_DMA_CTRL           (HR_UART0_BASE_ADDR + 0x08)
#define HR_UART0_FIFO_CTRL          (HR_UART0_BASE_ADDR + 0x0C)
#define HR_UART0_BAUD_RATE_CTRL     (HR_UART0_BASE_ADDR + 0x10)
#define HR_UART0_INT_MASK           (HR_UART0_BASE_ADDR + 0x14)
#define HR_UART0_INT_SRC            (HR_UART0_BASE_ADDR + 0x18)
#define HR_UART0_FIFO_STATUS        (HR_UART0_BASE_ADDR + 0x1C)
#define HR_UART0_TX_WIN             (HR_UART0_BASE_ADDR + 0x20)
#define HR_UART0_RX_WIN             (HR_UART0_BASE_ADDR + 0x30)

#define HR_UART1_LINE_CTRL          (HR_UART1_BASE_ADDR + 0x0)
#define HR_UART1_FLOW_CTRL          (HR_UART1_BASE_ADDR + 0x04)
#define HR_UART1_DMA_CTRL           (HR_UART1_BASE_ADDR + 0x08)
#define HR_UART1_FIFO_CTRL          (HR_UART1_BASE_ADDR + 0x0C)
#define HR_UART1_BAUD_RATE_CTRL     (HR_UART1_BASE_ADDR + 0x10)
#define HR_UART1_INT_MASK           (HR_UART1_BASE_ADDR + 0x14)
#define HR_UART1_INT_SRC            (HR_UART1_BASE_ADDR + 0x18)
#define HR_UART1_FIFO_STATUS        (HR_UART1_BASE_ADDR + 0x1C)
#define HR_UART1_TX_WIN             (HR_UART1_BASE_ADDR + 0x20)
#define HR_UART1_RX_WIN             (HR_UART1_BASE_ADDR + 0x30)
/*
 * Line Control Register bits definition
 * ULCON0 ULCON1 Register
 */
/* bit 0-1 : Word length */
#define ULCON_WL5               0x00
#define ULCON_WL6               0x01
#define ULCON_WL7               0x02
#define ULCON_WL8               0x03
#define ULCON_WL_MASK           0x03
/* bit 2 : Number of stop bits */
#define ULCON_STOP_2            0x04  /* 2 stop bit */
/* bit 3-4 : Parity mode */
#define ULCON_PMD_MASK          0x18
#define ULCON_PMD_EN            0x08  /* no parity */
#define ULCON_PMD_ODD           0x18  /* odd parity */
#define ULCON_PMD_EVEN          0x08  /* even parity */
/* bit 6 uart tx enable */
#define ULCON_TX_EN             0x40
#define ULCON_RX_EN             0x80
/* bit 6 : Infra-red mode */
#define ULCON_INFRA_RED         0x80

/* 
 * auto flow control register bits definition
 */
#define UFC_ENABLE                0x01
#define UFC_SW_RTS_SET            0x02
#define UFC_RTS_TRIGGER_LVL_4_BYTE  (0<<2)
#define UFC_RTS_TRIGGER_LVL_8_BYTE  (1<<2)
#define UFC_RTS_TRIGGER_LVL_12_BYTE  (2<<2)
#define UFC_RTS_TRIGGER_LVL_16_BYTE  (3<<2)
#define UFC_RTS_TRIGGER_LVL_20_BYTE  (4<<2)
#define UFC_RTS_TRIGGER_LVL_24_BYTE  (5<<2)
#define UFC_RTS_TRIGGER_LVL_28_BYTE  (6<<2)
#define UFC_RTS_TRIGGER_LVL_31_BYTE  (7<<2)

#define UFC_TX_FIFO_RESET       0x01
#define UFC_RX_FIFO_RESET       0x02
#define UFC_TX_FIFO_LVL_ZERO    (0<<2)
#define UFC_TX_FIFO_LVL_4_BYTE  (1<<2)
#define UFC_TX_FIFO_LVL_8_BYTE  (2<<2)
#define UFC_TX_FIFO_LVL_16_BYTE (3<<2)
#define UFC_RX_FIFO_LVL_ZERO    (0<<4)
#define UFC_RX_FIFO_LVL_4_BYTE  (1<<4)
#define UFC_RX_FIFO_LVL_8_BYTE  (2<<4)
#define UFC_RX_FIFO_LVL_16_BYTE (3<<4)

/* dma control */
#define UDMA_RX_FIFO_TIMEOUT    (1<<2)
#define UDMA_RX_FIFO_TIMEOUT_SHIFT  (3)

/* 
 * uart interrupt source register bits definition 
 */
#define UIS_TX_FIFO_EMPTY       (1UL<<0)
#define UIS_TX_FIFO             (1UL<<1)
#define UIS_RX_FIFO             (1UL<<2) 
#define UIS_RX_FIFO_TIMEOUT     (1UL<<3)
#define UIS_CTS_CHNG            (1UL<<4)
#define UIS_BREAK               (1UL<<5)
#define UIS_FRM_ERR             (1UL<<6)
#define UIS_PARITY_ERR          (1UL<<7)
#define UIS_OVERRUN             (1UL<<8)

/*
 * fifo status register bits definition
 */
/* bit 0-5 : tx fifo count */
#define UFS_TX_FIFO_CNT_MASK      0x3F
/* bit 6-11 : rx fifo count */
#define UFS_RX_FIFO_CNT_MASK      0x3C0
#define UFS_CST_STS               (1UL<<12)

/***************************************************************
 * GPIO�Ĵ�������
 ***************************************************************/
#define HR_GPIO_BASE_ADDR           (HR_APB_BASE_ADDR + 0x0C00)
#define HR_GPIO_DATA                (HR_GPIO_BASE_ADDR + 0x0)
#define HR_GPIO_DATA_EN             (HR_GPIO_BASE_ADDR + 0x04)
#define HR_GPIO_DIR                 (HR_GPIO_BASE_ADDR + 0x40)
#define HR_GPIO_IS                  (HR_GPIO_BASE_ADDR + 0x44)
#define HR_GPIO_IBE                 (HR_GPIO_BASE_ADDR + 0x48)
#define HR_GPIO_IEV                 (HR_GPIO_BASE_ADDR + 0x4C)
#define HR_GPIO_IE                  (HR_GPIO_BASE_ADDR + 0x50)
#define HR_GPIO_RIS                 (HR_GPIO_BASE_ADDR + 0x54)
#define HR_GPIO_MIS                 (HR_GPIO_BASE_ADDR + 0x58)
#define HR_GPIO_IC                  (HR_GPIO_BASE_ADDR + 0x5C)
#define HR_GPIO_AFSEL               (HR_GPIO_BASE_ADDR + 0x60)
#define HR_GPIO_ITCR                (HR_GPIO_BASE_ADDR + 0x64)



/***************************************************************
 * TIMER�Ĵ�������
 ***************************************************************/
#define HR_TIMER_BASE_ADDR          (HR_APB_BASE_ADDR + 0x0E00)
#define HR_TIMER_CFG                (HR_TIMER_BASE_ADDR + 0x0)
#define HR_TIMER0_CSR               (HR_TIMER_BASE_ADDR + 0x04)
#define HR_TIMER1_CSR               (HR_TIMER_BASE_ADDR + 0x08)
#define HR_TIMER2_CSR               (HR_TIMER_BASE_ADDR + 0x0C)
/* timer csr bits defintion */
#define TLS_TIMER_VALUE_S               ((unsigned int) 0x8)
#define TLS_TIMER_MS_UNIT               (1UL<<0)
#define TLS_TIMER_ONE_TIME              (1UL<<1)
#define TLS_TIMER_EN                    (1UL<<2)
#define TLS_TIMER_INT_EN                (1UL<<3)
#define TLS_TIMER_INT_CLR               (1UL<<4)

/***************************************************************
 * WATCH DOG�Ĵ�������
 ***************************************************************/
#define HR_WDG_BASE_ADDR            (HR_APB_BASE_ADDR + 0x1000)
#define HR_WDG_LOAD_VALUE           (HR_WDG_BASE_ADDR + 0x00)
#define HR_WDG_CUR_VALUE            (HR_WDG_BASE_ADDR + 0x04)
#define HR_WDG_CTRL                 (HR_WDG_BASE_ADDR + 0x08)
#define HR_WDG_INT_CTRL             (HR_WDG_BASE_ADDR + 0x0C)
#define HR_WDG_INT_SRC              (HR_WDG_BASE_ADDR + 0x10)

/***************************************************************
 * IOCTL�Ĵ�������
 ***************************************************************/
#define HR_IOCTL_BASE_ADDR          (HR_APB_BASE_ADDR + 0x1200)
#define HR_IOCTL_DL_MODE            (HR_IOCTL_BASE_ADDR + 0x0)
#define HR_IOCTL_GP_SDIO_I2C        (HR_IOCTL_BASE_ADDR + 0x04)
#define HR_IOCTL_GP_JTAG            (HR_IOCTL_BASE_ADDR + 0x08)
#define HR_IOCTL_GP_SPI             (HR_IOCTL_BASE_ADDR + 0x0C)
#define HR_IOCTL_PE                 (HR_IOCTL_BASE_ADDR + 0x10)


#define IOCTRL_STARTADDRESS	(0x0F001200)
#define IOCTRL_DOWNLOAD_MODE_REG	(*(volatile unsigned *)(IOCTRL_STARTADDRESS+0x000))
#define IOCTRL_PIN_CONFIG_REG1      (*(volatile unsigned *)(IOCTRL_STARTADDRESS+0x004))
#define IOCTRL_PIN_CONFIG_REG2      (*(volatile unsigned *)(IOCTRL_STARTADDRESS+0x008))
#define IOCTRL_PIN_CONFIG_REG3      (*(volatile unsigned *)(IOCTRL_STARTADDRESS+0x00C))

/*  EFUSE Register */
#define HR_EFUSE_REG_BASE      (0x0f001600)
#define HR_EFUSE_WDATA_PORT      (HR_EFUSE_REG_BASE+0x000)
#define HR_EFUSE_PGM_CMD      (HR_EFUSE_REG_BASE+0x004)
#define HR_EFUSE_PGM_STTS      (HR_EFUSE_REG_BASE+0x008)
#define HR_EFUSE_CLEAR      (HR_EFUSE_REG_BASE+0x00C)
#define HR_EFUSE_VDDQ_SETUP      (HR_EFUSE_REG_BASE+0x010)

/* PGM_CMD define */
#define EFUSE_PGM_START      (1UL)
#define EFUSE1_SELECT      (1UL << 1)
#define EFUSE2_SELECT      (1UL << 2)

/* PGM STTS define */
#define EFUSE_PGM_DONE	      (1UL)
#define EFUSE_FIFO_FREE      (1UL << 1)

/* CLEAR_EFUSE */
#define EFUSE_RESET      (1UL)

static __inline void tls_reg_write32(unsigned int reg, unsigned int val)
{
    *(TLS_REG *)reg = val;
}


static __inline unsigned int tls_reg_read32(unsigned int reg)
{
    unsigned int val = *(TLS_REG *)reg;
    return val;
}

#endif /* WM_REGS_H */
