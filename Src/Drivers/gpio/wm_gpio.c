/***************************************************************************** 
* 
* File Name : wm_gpio.c 
* 
* Description: GPIO Driver Module 
* 
* Copyright (c) 2014 Winner Microelectronics Co., Ltd. 
* All rights reserved. 
* 
* Author : dave 
* 
* Date : 2014-6-4
*****************************************************************************/ 

#include "wm_gpio.h"
#include "wm_regs.h"
#include "wm_irq.h"
#include "wm_osal.h"

/**********************************************************************************************************
* Description: 	This function is used to config gpio function.
*
* Arguments  : 	gpio_pin		is gpio pin num,start from 0,end to 20.
* 		dir			is gpio dir,define as follow:
* 					dir == TLS_GPIO_DIR_OUTPUT	output
* 					dir == TLS_GPIO_DIR_INPUT	input
* 		attr			is gpio attribute,define as follow:
* 					attr == TLS_GPIO_ATTR_FLOATING	:floating status
* 					atrr == TLS_GPIO_ATTR_PULLHIGH	:pull high
* 					attr == TLS_GPIO_ATTR_PULLLOW	:pull low 			
*
* Returns    : 	null
* Note:		
**********************************************************************************************************/
void tls_gpio_cfg(u16 gpio_pin, u16 dir, u16 attr)
{
	u32 ioctl;
	u32 pin_offset = 0;
	u32 reg;

	/*��gpio_pin���ó�gpio����*/
	if(gpio_pin >= 3 && gpio_pin <= 9)		/*����HR_IOCTL_GP_SDIO_I2C�Ĵ���*/
	{
		ioctl = tls_reg_read32(HR_IOCTL_GP_SDIO_I2C);
		pin_offset = (gpio_pin - 2)*2;
		ioctl &= ~(0x03 << pin_offset);
		ioctl |= (0x02 << pin_offset);
		tls_reg_write32(HR_IOCTL_GP_SDIO_I2C, ioctl);
	}
	else if(gpio_pin >= 11 && gpio_pin <= 17)	/*����HR_IOCTL_GP_JTAG�Ĵ���*/
	{
		ioctl = tls_reg_read32(HR_IOCTL_GP_JTAG);
		if(gpio_pin >= 11 && gpio_pin <= 13)		/*һ��bit����һ��io*/
		{
			pin_offset = (gpio_pin - 10);			/*��bit1 ��ʼ*/
			ioctl |= (1 << pin_offset);				/*gpio*/
		}
		else										/*����bit ����һ��io*/
		{
			pin_offset = (gpio_pin - 10)*2;
			ioctl &= ~(0x03 << pin_offset);
			ioctl |= (0x02 << pin_offset);
		}
		tls_reg_write32(HR_IOCTL_GP_JTAG, ioctl);
	
	}
	else if(10 == gpio_pin || (gpio_pin >= 18 && gpio_pin <= 20))	/*����HR_IOCTL_GP_SPI �Ĵ���*/
	{
		ioctl = tls_reg_read32(HR_IOCTL_GP_SPI);
		if(10 == gpio_pin || 18 == gpio_pin)
		{
			if(10 == gpio_pin)
			{
				pin_offset = 0;
			}
			else if(18 == gpio_pin)
			{
				pin_offset = 1;
			}
			ioctl |= (1 << pin_offset);
		}
		else
		{
			pin_offset = (gpio_pin - 18)*2;
			ioctl &= ~(0x03 << pin_offset);
			ioctl |= (0x01 << pin_offset);
		}
		tls_reg_write32(HR_IOCTL_GP_SPI, ioctl);
	}
	else if(gpio_pin <= 2)	/*����HR_IOCTL_DL_MODE�Ĵ���*/
	{
		ioctl = tls_reg_read32(HR_IOCTL_DL_MODE);
		ioctl |= 0x01;		/*0,1,2����Ϊgpioʹ��*/
		tls_reg_write32(HR_IOCTL_DL_MODE, ioctl);
	}

	/*����gpio����*/
	reg = tls_reg_read32(HR_GPIO_DIR);
	//TLS_DBGPRT_INFO("\gpio dir=%x\n",reg);
	switch(dir)
	{
		case TLS_GPIO_DIR_OUTPUT:
			reg |= (0x01 << gpio_pin);		/*bit==1 ���*/
			break;
		case TLS_GPIO_DIR_INPUT:
			reg &= ~(0x01 << gpio_pin);		/*bit==0 ����*/
			break;
		default:
			break;
	}
//	TLS_DBGPRT_INFO("\gpio dir 1====%x\n",reg);
	tls_reg_write32(HR_GPIO_DIR, reg);

	
	/*��������������*/
	reg = tls_reg_read32(HR_IOCTL_PE);
	if(gpio_pin<=9 || (gpio_pin >= 18 && gpio_pin <= 20))
		pin_offset = gpio_pin;
	else if(gpio_pin >= 11 && gpio_pin <=17)
		pin_offset = gpio_pin - 1;
	else if(10 == gpio_pin)
		pin_offset = 17;
	switch(attr)
	{
		case TLS_GPIO_ATTR_FLOATING:
			reg &= ~(0x1 << pin_offset);
			break;
		case TLS_GPIO_ATTR_PULLHIGH:
			if(gpio_pin >= 3 && gpio_pin <= 7)
			{
				reg |= (0x1 << pin_offset);
			}
			break;
		case TLS_GPIO_ATTR_PULLLOW:
			if((gpio_pin <= 2) || (gpio_pin >= 8 && gpio_pin <= 20))
			{
				reg |= (0x1 << pin_offset);
			}
			break;
		default:
			break;
	}
	tls_reg_write32(HR_IOCTL_PE, reg);
	
}


/**********************************************************************************************************
* Description: 	This function is used to read gpio status.
*
* Arguments  : 	gpio_pin		is gpio pin num,start from 0,end to 20.
*
* Returns    : 	0			low power level
* 		1			high power level
**********************************************************************************************************/
u16 tls_gpio_read(u16 gpio_pin)
{
//	u32	reg_en;
	u32 reg;

//	reg_en = tls_reg_read32(HR_GPIO_DATA_EN);
//	tls_reg_write32(HR_GPIO_DATA_EN, reg_en | (1<<gpio_pin));
	reg = tls_reg_read32(HR_GPIO_DATA);
//	tls_reg_write32(HR_GPIO_DATA_EN, reg_en);
	if(reg & (0x1 << gpio_pin))
		return 1;
	else
		return 0;
}

/**********************************************************************************************************
* Description: 	This function is used to modify gpio status.
*
* Arguments  : 	gpio_pin		is gpio pin num,start from 0,end to 20.
* 		value			is power level,define as follow:
* 					value == 0	:low power level
* 					value == 1	:high power level
*
* Returns    : 	
**********************************************************************************************************/
void tls_gpio_write(u16 gpio_pin, u16 value)
{
	u32 cpu_sr = 0;
	u32 reg;
	u8 irqcount;
	irqcount = tls_get_isr_count();
	if(0 == irqcount)	//freertos���������ж��е����ٽ�������
	{
		cpu_sr = tls_os_set_critical();
	}
	reg = tls_reg_read32(HR_GPIO_DATA);
	if(value)
		tls_reg_write32(HR_GPIO_DATA, reg |  (1<<gpio_pin));		/*д��*/
	else
		tls_reg_write32(HR_GPIO_DATA, reg & (~(1<<gpio_pin)));	/*д��*/
	if(0 == irqcount)	
	{
		tls_os_release_critical(cpu_sr);
	}
}


/**********************************************************************************************************
* Description: 	This function is used to config gpio interrupt.
*
* Arguments  : 	gpio_pin		is gpio pin num,start from 0,end to 20.
* 		mode			is interrupt trigger type,define as follow:
* 					mode == TLS_GPIO_INT_TRIG_RISING_EDGE	:rising edge arises the interrupt.
* 					mode == TLS_GPIO_INT_TRIG_FALLING_EDGE	:rising edge arises the interrupt.
* 					mode == TLS_GPIO_INT_TRIG_DOUBLE_EDGE	:both rising edge and falling edge arise the interrupt.
* 					mode == TLS_GPIO_INT_TRIG_HIGH_LEVEL	:high power level arises the interrupt.
* 					mode == TLS_GPIO_INT_TRIG_LOW_LEVEL	:low power level arises the interrupt.
*
* Returns    : 	
**********************************************************************************************************/
void tls_gpio_int_enable(u16 gpio_pin, u16 mode)
{
	u32 reg;

//	TLS_DBGPRT_INFO("\r\ntls_gpio_int_enable gpio pin =%d,mode==%d\r\n",gpio_pin,mode);
	
	switch(mode)
	{
		case TLS_GPIO_INT_TRIG_RISING_EDGE:
			reg = tls_reg_read32(HR_GPIO_IS);
			reg &= (~(0x1 << gpio_pin));
		//	TLS_DBGPRT_INFO("\r\nrising edge is ret=%x\r\n",reg);
			tls_reg_write32(HR_GPIO_IS, reg);		/*0 ���ش���*/
			reg = tls_reg_read32(HR_GPIO_IBE);
			reg &= (~(0x1 << gpio_pin));
		//	TLS_DBGPRT_INFO("\r\nrising edge ibe ret=%x\r\n",reg);	
			tls_reg_write32(HR_GPIO_IBE, reg & (~(0x1 << gpio_pin)));			/*������ʽ��HR_GPIO_IEV����*/
			reg = tls_reg_read32(HR_GPIO_IEV);
			reg |= (0x1 << gpio_pin);
		//	TLS_DBGPRT_INFO("\r\nrising edge iev ret=%x\r\n",reg);
			tls_reg_write32(HR_GPIO_IEV, reg );				/*1�������ش���*/			
			break;
		case TLS_GPIO_INT_TRIG_FALLING_EDGE:
			reg = tls_reg_read32(HR_GPIO_IS);
			reg &= (~(0x1 << gpio_pin));
		//	TLS_DBGPRT_INFO("\falling edge is ret=%x\n",reg);
			tls_reg_write32(HR_GPIO_IS, reg);		/*0 ���ش���*/
			reg = tls_reg_read32(HR_GPIO_IBE);
			reg &= (~(0x1 << gpio_pin));
		//	TLS_DBGPRT_INFO("\falling edge ibe ret=%x\n",reg);	
			tls_reg_write32(HR_GPIO_IBE, reg);			/*������ʽ��HR_GPIO_IEV����*/
			reg = tls_reg_read32(HR_GPIO_IEV);
			reg &= (~(0x1 << gpio_pin));
		//	TLS_DBGPRT_INFO("\falling edge iev ret=%x\n",reg);	
			tls_reg_write32(HR_GPIO_IEV, reg);				/*0���½��ش���*/			
			break;
		case TLS_GPIO_INT_TRIG_DOUBLE_EDGE:
			reg = tls_reg_read32(HR_GPIO_IS);
			tls_reg_write32(HR_GPIO_IS, reg & (~(0x1 << gpio_pin)));		/*0 ���ش���*/
			reg = tls_reg_read32(HR_GPIO_IBE);
			tls_reg_write32(HR_GPIO_IBE, reg |(0x1 << gpio_pin));			
			break;
		case TLS_GPIO_INT_TRIG_HIGH_LEVEL:
			reg = tls_reg_read32(HR_GPIO_IS);
			tls_reg_write32(HR_GPIO_IS, reg |(0x1 << gpio_pin));		/*1��ƽ����*/
			reg = tls_reg_read32(HR_GPIO_IEV);
			tls_reg_write32(HR_GPIO_IEV, reg |(0x1 << gpio_pin));				/*1���ߵ�ƽ����*/		
			break;
		case TLS_GPIO_INT_TRIG_LOW_LEVEL:
			reg = tls_reg_read32(HR_GPIO_IS);
			tls_reg_write32(HR_GPIO_IS, reg |(0x1 << gpio_pin));		/*1��ƽ����*/
			reg = tls_reg_read32(HR_GPIO_IEV);
			tls_reg_write32(HR_GPIO_IEV, reg & (~(0x1 << gpio_pin)));				/*0���͵�ƽ����*/		
			break;
	}

	reg = tls_reg_read32(HR_GPIO_IE);
	reg |= (0x1 << gpio_pin);
//	TLS_DBGPRT_INFO("\nie ret=%x\n",reg);
	tls_reg_write32(HR_GPIO_IE, reg);		/*�ж�ʹ��*/	
	tls_irq_enable(GPIO_INTTERRUPT);
}

int tls_gpio_int_if_enable(int gpio_pin)
{
	u32 reg;

	reg = tls_reg_read32(HR_GPIO_IE);
	if(reg & (0x1<<gpio_pin))
		return TRUE;
	else
		return FALSE;
		
}

/**********************************************************************************************************
* Description: 	This function is used to disable gpio interrupt.
*
* Arguments  : 	gpio_pin		is gpio pin num,start from 0,end to 20.
*
* Returns    : 	
**********************************************************************************************************/
void tls_gpio_int_disable(u16 gpio_pin)
{
	u32 reg;

	reg = tls_reg_read32(HR_GPIO_IE);
	tls_reg_write32(HR_GPIO_IE, reg & (~(0x1 << gpio_pin)));		/*�ж�disable*/	
	tls_irq_disable(GPIO_INTTERRUPT);
}

/**********************************************************************************************************
* Description: 	This function is used to get gpio interrupt status.
*
* Arguments  : 	gpio_pin		is gpio pin num,start from 0,end to 20.
*
* Returns    : 	return the interrupt status
**********************************************************************************************************/
int tls_get_gpio_int_flag(u16 gpio_pin)
{
	u32 reg;

	reg = tls_reg_read32(HR_GPIO_RIS);
	if(reg & (0x1 << gpio_pin))
		return	1;
	else
		return	0;
}

/**********************************************************************************************************
* Description: 	This function is used to clear gpio interrupt flag.
*
* Arguments  : 	gpio_pin		is gpio pin num,start from 0,end to 20.
*
* Returns    : 	
**********************************************************************************************************/
void tls_clr_gpio_int_flag(u16 gpio_pin)
{
	u32 reg;

	reg = tls_reg_read32(HR_GPIO_IC);
	tls_reg_write32(HR_GPIO_IC, reg |(0x1 << gpio_pin));		/*1 ���ж�*/
}



/**********************************************************************************************************
* Description: 	This function is used to register gpio interrupt.
*
* Arguments  : 	callback		is the gpio interrupt call back function
*			
*
* Returns    : 	
**********************************************************************************************************/
void tls_gpio_isr_register(void (*callback)(void *context), void *context)
{
 	tls_irq_register_handler(GPIO_INTTERRUPT, callback, context);		/*gpio ��Ӧ���жϺ���25*/
}

