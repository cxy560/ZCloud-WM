#ifndef USER_SPI_H
#define USER_SPI_H

#include <string.h>
#include "wm_irq.h"
#include "tls_sys.h"
#include "wm_gpio.h"
#include "wm_params.h"
#include "wm_hostspi.h"
#include "wm_debug.h"
#if (GCC_COMPILE==1)
#include "wm_cmdp_hostif_gcc.h"
#else
#include "wm_cmdp_hostif.h"
#endif
#include "wm_config.h"
#include "wm_osal.h"
#include "wm_adc.h"
#include "wm_sockets.h"

#define  SPI_IO_CS		3	//spi cs
#define  SPI_IO_DI		4 	//spi di
#define  SPI_IO_DO		5	//spi do
#define  SPI_IO_CK		12	//uart1 tx gpio12
#define  SPI_IO_PKT		7	//uart1 rx spi int
#define  SPI_IO_RST		2	//boot mode2

#define SPI_RST(x)		tls_gpio_write(SPI_IO_RST, x) 
#define SPI_CS(x)		tls_gpio_write(SPI_IO_CS, x) 
#define SPI_CK(x)		tls_gpio_write(SPI_IO_CK, x) 
#define SPI_DO(x)		tls_gpio_write(SPI_IO_DO, x) 
#define SPI_DI			tls_gpio_read(SPI_IO_DI) 
#define SPI_PKT			tls_gpio_read(SPI_IO_PKT) 


void UserSpiInit(void);
void UserSpiWriteByte(u8 out);
INT16U UserSpiRead16(void);


#endif
