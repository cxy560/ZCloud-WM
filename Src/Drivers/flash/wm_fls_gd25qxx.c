/***************************************************************************** 
* 
* File Name : wm_fls_gd25qxx.c 
* 
* Description: gd25qxx flash Driver Module 
* 
* Copyright (c) 2014 Winner Micro Electronic Design Co., Ltd. 
* All rights reserved. 
* 
* Author : dave
* 
* Date : 2014-6-3
*****************************************************************************/ 

#include <stdio.h>
#include <stdlib.h>
#include "wm_type_def.h"
#include "wm_flash.h"
#include "wm_hostspi.h"
#include "wm_fls_gd25qxx.h"
#include "wm_debug.h"
#include "wm_gpio.h"
#include "../VS10XX/VS10XX.h"

static int gd25qxx_read(u32 addr, u8 *buf, u32 len);
static int gd25qxx_fast_read(u32 addr, u8 *buf, u32 len);
static int gd25qxx_page_write(u32 page, u8 *buf);
static int gd25qxx_erase(u32 sector);
static int gd25qxx_chip_erase(void);
static int gd25qxx_probe(void);
static void gd25qxx_remove(void);

static struct tls_fls_drv gd25qxx_fls = {
	.drv_list = {NULL, NULL},
	.id = FLASH_PID,
	.total_size = TOTAL_SIZE,
	.page_size = PAGE_SIZE,
	.program_size = PROGRAM_SIZE,
	.sector_size = SECTOR_SIZE,
	.clock = SPI_SCLK,
	.mode = TLS_SPI_MODE_0,
	.cs_active = TLS_SPI_CS_LOW,
	.flags = 0,
	.read = gd25qxx_read,
	.fast_read = gd25qxx_fast_read,
	.page_write = gd25qxx_page_write,
	.erase = gd25qxx_erase,
	.chip_erase = gd25qxx_chip_erase,
	.probe = gd25qxx_probe,
	.remove = gd25qxx_remove
};

static struct tls_fls_drv *gd25qxx_drv = NULL;


static unsigned int swap32(unsigned int v)
{
	return ((v & 0xff) << 24) | ((v & 0xff00) << 8) |
		((v & 0xff0000) >> 8) | (v >> 24);
}

void fls_cs_low(void)
{
#if SPI_CS_AS_GPIO 
#if CODEC_VS1053
	while(0 == tls_gpio_read(VS_XDCS) || 0 == tls_gpio_read(VS_XCS))
	{
		printf("\ncodec cs busy,flash RW wait...\n");
		tls_os_time_delay(1);
	}
#endif	
	tls_gpio_write(SPI_CS_PIN, 0);
#endif
}

void fls_cs_high(void)
{
#if SPI_CS_AS_GPIO
	tls_gpio_write(SPI_CS_PIN, 1);
#endif

}

static int gd25qxx_read_id(u32 *id)
{
	u32 cmd;
	int err;

	//cmd = GD25QXX_FLASH_ID;
	cmd = GD25QXX_FLASH_DEVICEID;
	*id = 0;
	fls_cs_low();
	err = tls_spi_write_then_read((const u8 *)&cmd, 4, (u8 *)id, 2);
	fls_cs_high();
	if (err != TLS_SPI_STATUS_OK) {return TLS_FLS_STATUS_EIO;} 

	return TLS_FLS_STATUS_OK;
}

static int gd25qxx_write_enable(void)
{
	u8 cmd;
	int err;

	cmd = GD25QXX_WRITE_ENABLE;
	fls_cs_low();
	err = tls_spi_write((const u8 *)&cmd, 1);
	fls_cs_high();
	if (err != TLS_SPI_STATUS_OK) {return TLS_FLS_STATUS_EIO;} 

	return TLS_FLS_STATUS_OK;
}

static int gd25qxx_wait_write_enable(void)
{
	u8 cmd;
	u8 sr;
	int err;

	cmd = GD25QXX_READ_SR1;
	sr = 0;
	do {
		fls_cs_low();
		err = tls_spi_write_then_read((const u8 *)&cmd, 1, &sr, 1);
		fls_cs_high();
		if (err != TLS_SPI_STATUS_OK) {return TLS_FLS_STATUS_EIO;} 

		if (sr & FLASH_STATUS_WEL) {break;}
	} while(1);

	return TLS_FLS_STATUS_OK;
}

static int gd25qxx_wait_flash_ready(void)
{
	u8 cmd;
	u8 sr;
	int err;

	cmd = GD25QXX_READ_SR1;
	sr = 0;
	do {
		fls_cs_low();
		err = tls_spi_write_then_read((const u8 *)&cmd, 1, &sr, 1);
		fls_cs_high();
		if (err != TLS_SPI_STATUS_OK) {return TLS_FLS_STATUS_EIO;} 

		if ((sr & FLASH_STATUS_BUSY) == 0x00) {break;}
	} while(1);

	return TLS_FLS_STATUS_OK;
}

static int gd25qxx_read(u32 addr, u8 *buf, u32 len)
{
	u32 cmd;
	int err;

	cmd = 0;
	cmd |= GD25QXX_DATA_READ;
	cmd |= swap32(addr) & 0xffffff00;
	fls_cs_low();
	err = tls_spi_write_then_read((const u8 *)&cmd, 4, buf, len);
	fls_cs_high();
	if (err != TLS_SPI_STATUS_OK) {return TLS_FLS_STATUS_EIO;} 

	return TLS_FLS_STATUS_OK;
}

static int gd25qxx_fast_read(u32 addr, u8 *buf, u32 len)
{
	return TLS_FLS_STATUS_ENOSUPPORT;
}

static int gd25qxx_page_write(u32 page, u8 *buf)
{
	u32 cmd;
	int err;

	err = gd25qxx_write_enable();
	if (err != TLS_SPI_STATUS_OK) {return TLS_FLS_STATUS_EIO;} 

	err = gd25qxx_wait_write_enable();
	if (err != TLS_SPI_STATUS_OK) {return TLS_FLS_STATUS_EIO;} 
	
	cmd = 0;
	cmd |= GD25QXX_PAGE_PROGRAM;
	cmd |= swap32(page * gd25qxx_drv->page_size) & 0xffffff00;
	fls_cs_low();
	err = tls_spi_write32_then_writen((const u32 *)&cmd, (const u8 *)buf, gd25qxx_drv->page_size);
	fls_cs_high();
	if (err != TLS_SPI_STATUS_OK) {return TLS_FLS_STATUS_EIO;} 

	err = gd25qxx_wait_flash_ready();
	if (err != TLS_SPI_STATUS_OK) {return TLS_FLS_STATUS_EIO;} 

	return TLS_FLS_STATUS_OK;
}

static int gd25qxx_erase(u32 sector)
{
	u32 cmd;
	int err;

	err = gd25qxx_write_enable();
	if (err != TLS_SPI_STATUS_OK) {return TLS_FLS_STATUS_EIO;} 
	err = gd25qxx_wait_write_enable();
	if (err != TLS_SPI_STATUS_OK) {return TLS_FLS_STATUS_EIO;} 
	cmd = 0;
	cmd |= GD25QXX_SECTOR_ERASE;
	cmd |= swap32(sector * gd25qxx_drv->sector_size) & 0xffffff00;
	fls_cs_low();
	err = tls_spi_write((const u8 *)&cmd, 4);
	fls_cs_high();
	if (err != TLS_SPI_STATUS_OK) {return TLS_FLS_STATUS_EIO;} 
	tls_os_time_delay(6);
	err = gd25qxx_wait_flash_ready();
	if (err != TLS_SPI_STATUS_OK) {return TLS_FLS_STATUS_EIO;} 
	return TLS_FLS_STATUS_OK;
}

static int gd25qxx_chip_erase(void)
{
	u8 cmd;
	int err;

	err = gd25qxx_write_enable();
	if (err != TLS_SPI_STATUS_OK) {return TLS_FLS_STATUS_EIO;} 

	err = gd25qxx_wait_write_enable();
	if (err != TLS_SPI_STATUS_OK) {return TLS_FLS_STATUS_EIO;} 
	
	cmd = GD25QXX_CHIP_ERASE;
	fls_cs_low();
	err = tls_spi_write((const u8 *)&cmd, 1);
	fls_cs_high();
	if (err != TLS_SPI_STATUS_OK) {return TLS_FLS_STATUS_EIO;} 

	err = gd25qxx_wait_flash_ready();
	if (err != TLS_SPI_STATUS_OK) {return TLS_FLS_STATUS_EIO;} 

	return TLS_FLS_STATUS_OK;
}
static int gd25qxx_probe(void)
{
	int err;
	u32 id;

	err = gd25qxx_read_id(&id);
	if (err != TLS_SPI_STATUS_OK) {return TLS_FLS_STATUS_EIO;}

	TLS_DBGPRT_INFO("gd25qxx spi flash ID - 0x%x.\n", id);

	if (id == gd25qxx_fls.id) {
		gd25qxx_drv = &gd25qxx_fls;
		TLS_DBGPRT_INFO("install gd25q80 spi flash driver.\n");
	} else {
		TLS_DBGPRT_WARNING("not found the current gd25qxx spi flash driver\n");
		return TLS_FLS_STATUS_ENODRV;
	}

	return TLS_FLS_STATUS_OK;
}

static void gd25qxx_remove(void)
{
	gd25qxx_drv = NULL;
}


/**********************************************************************************************************
* Description: 	This function is used to install gd25qxx driver.
*
* Arguments  : 	
* Returns    : 	TLS_FLS_STATUS_OK	if write flash success
* 		TLS_FLS_STATUS_EPERM	if flash struct point is null
* 		TLS_FLS_STATUS_ENODRV	if flash driver is not installed
* 		TLS_FLS_STATUS_EINVAL	if argument is invalid
*		TLS_FLS_STATUS_EIO		if io error
*		TLS_FLS_STATUS_EEXIST	if driver is already existed
*********************************************************************************************************/
int tls_fls_gd25qxx_drv_install(void)
{
	int err;
	extern int tls_fls_probe(void);
	extern int tls_fls_drv_register(struct tls_fls_drv *fls_drv);

	err = tls_fls_drv_register((struct tls_fls_drv *)&gd25qxx_fls);
	
	TLS_DBGPRT_INFO("register the gd25q80 spi flash driver - %d.\n", err);

	err = tls_fls_probe();
	TLS_DBGPRT_INFO("probe spi flash - %d.\n", err);

	return err;
}
