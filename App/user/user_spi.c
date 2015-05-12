#include "user_spi.h"

void UserSpiInit(void)
{
	INT16U temp;
	
	tls_gpio_cfg(SPI_IO_RST, TLS_GPIO_DIR_OUTPUT, TLS_GPIO_ATTR_FLOATING);
	tls_gpio_cfg(SPI_IO_CK, TLS_GPIO_DIR_OUTPUT, TLS_GPIO_ATTR_FLOATING);
	tls_gpio_cfg(SPI_IO_CS, TLS_GPIO_DIR_OUTPUT, TLS_GPIO_ATTR_PULLHIGH);
	tls_gpio_cfg(SPI_IO_DO, TLS_GPIO_DIR_OUTPUT, TLS_GPIO_ATTR_PULLHIGH);
	
	tls_gpio_cfg(SPI_IO_DI, TLS_GPIO_DIR_INPUT, TLS_GPIO_ATTR_FLOATING);
	tls_gpio_cfg(SPI_IO_PKT, TLS_GPIO_DIR_INPUT, TLS_GPIO_ATTR_PULLHIGH);

	SPI_RST(1);
	SPI_CK(1);
	SPI_CS(1);
}
static void UserDelay(void)
{
	volatile register int i;
	
	for(i = 0; i < 0x0f; i++)
	{
	}
}
void UserSpiWriteByte(u8 out)
{	
	unsigned char i = 0;
	
	for (i = 0; i < 8; i++)
	{		
		SPI_CK(0); 
		UserDelay();
		if (out & 0x80) 
		{
			SPI_DO(1);
		}
		else	
		{
			SPI_DO(0);
		}
		out <<= 1;
		SPI_CK(1);
		UserDelay();
	}
}

INT16U UserSpiRead16(void)
{	
	unsigned char i = 0;
	INT16U value = 0;
	
	for (i = 0; i < 16; i++)
	{		
		SPI_CK(0);
		UserDelay();
		SPI_CK(1);
		UserDelay();
		value |= SPI_DI;
		value <<= 1;
	}	
	return value;
}

