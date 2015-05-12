/***************************************************************************** 
* 
* File Name : wm_fls.c 
* 
* Description: flash Driver Module 
* 
* Copyright (c) 2014 Winner Microelectronics Co., Ltd. 
* All rights reserved. 
* 
* Author : dave 
* 
* Date : 2014-6-4
*****************************************************************************/ 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "list.h"
#include "wm_hostspi.h"
#include "wm_flash.h"
#include "wm_debug.h"
#include "wm_mem.h"
#include "wm_fls_gd25qxx.h"


static struct tls_fls *spi_fls = NULL;


static int fls_read_id(u32 *id)
{
	u32 cmd;
	int err;

	cmd = FLS_CMD_READ_DEV_ID;
	*id = 0;
	fls_cs_low();
	err = tls_spi_write_then_read((const u8 *)&cmd, 4, (u8 *)id, 2);
	fls_cs_high();
	if (err != TLS_SPI_STATUS_OK) {
		TLS_DBGPRT_ERR("flash read ID fail(%d)!\n", err);
		return TLS_FLS_STATUS_EIO;
	}

	TLS_DBGPRT_FLASH("flash ID - 0x%x.\n", *id);

	return TLS_FLS_STATUS_OK;
}

/**********************************************************************************************************
* Description: 	This function is used to read data from the flash.
*
* Arguments  : 	addr			is byte offset addr for read from the flash.
* 		buf			is user for data buffer of flash read
* 		len			if byte length for read.
*
* Returns    : 	TLS_FLS_STATUS_OK	if read sucsess
* 		TLS_FLS_STATUS_EIO	if read fail
**********************************************************************************************************/
int tls_fls_read(u32 addr, u8 *buf, u32 len)
{
	int err;
	u32 read_bytes;
	struct tls_fls_drv *drv;

	if (spi_fls == NULL) {
		TLS_DBGPRT_ERR("flash driver module not beed installed!\n");
		return TLS_FLS_STATUS_EPERM;
	}
	
	if (spi_fls->current_drv == NULL) {
		TLS_DBGPRT_ERR("the current spi flash driver not installed!\n");
		return TLS_FLS_STATUS_ENODRV;
	}
	
	if ((addr >= spi_fls->current_drv->total_size) || (len == 0) || (buf == NULL)) {return TLS_FLS_STATUS_EINVAL;}
	tls_os_sem_acquire(spi_fls->fls_lock, 0);
	drv = spi_fls->current_drv;
	read_bytes = ((addr + len) > drv->total_size) ? (drv->total_size - addr) : len;

	err = TLS_FLS_STATUS_OK;

	err = drv->read(addr, buf, read_bytes);
	tls_os_sem_release(spi_fls->fls_lock);
	return err;
}

int tls_fls_fast_read(u32 addr, u8 *buf, u32 len)
{
	int err;
	u32 read_bytes;
	struct tls_fls_drv *drv;

	if (spi_fls == NULL) {
		TLS_DBGPRT_ERR("flash driver module not beed installed!\n");
		return TLS_FLS_STATUS_EPERM;
	}
	
	if (spi_fls->current_drv == NULL) {
		TLS_DBGPRT_ERR("the current spi flash driver not installed!\n");
		return TLS_FLS_STATUS_ENODRV;
	}
	
	if ((addr >= spi_fls->current_drv->total_size) || (len == 0) || (buf == NULL)) {return TLS_FLS_STATUS_EINVAL;}
	if ((spi_fls->current_drv->flags & TLS_FLS_FLAG_FAST_READ) != TLS_FLS_FLAG_FAST_READ) {return TLS_FLS_STATUS_ENOSUPPORT;}

	drv = spi_fls->current_drv;
	read_bytes = ((addr + len) > drv->total_size) ? (drv->total_size - addr) : len;

	err = TLS_FLS_STATUS_OK;

	err = drv->fast_read(addr, buf, read_bytes);

	return err;
}

int tls_fls_page_write(u32 page, u8 *buf, u32 page_cnt)
{
	int err;
	u32 write_pages;
	u32 i;
	struct tls_fls_drv *drv;

	if (spi_fls == NULL) {
		TLS_DBGPRT_ERR("flash driver module not beed installed!\n");
		return TLS_FLS_STATUS_EPERM;
	}
	
	if (spi_fls->current_drv == NULL) {
		TLS_DBGPRT_ERR("the current spi flash driver not installed!\n");
		return TLS_FLS_STATUS_ENODRV;
	}
	
	if ((page >= (spi_fls->current_drv->total_size/spi_fls->current_drv->page_size)) || (page_cnt == 0) || (buf == NULL)) {return TLS_FLS_STATUS_EINVAL;}

	drv = spi_fls->current_drv;
	write_pages = ((page + page_cnt) > (drv->total_size/drv->page_size)) ? ((drv->total_size/drv->page_size) - page) : page_cnt;

	err = TLS_FLS_STATUS_OK;

	for (i = 0; i < write_pages; i++) {
		err = drv->page_write(page + i, buf + i*drv->page_size);
		if (err != TLS_FLS_STATUS_OK) {
			TLS_DBGPRT_ERR("flash page write fail(page %d)!\n", (page + i));
			break;
		}
	}

	return err;
}

/**********************************************************************************************************
* Description: 	This function is used to write data to the flash.
*
* Arguments  : 	addr			is byte offset addr for write to the flash
* 		buf			is the data buffer want to write to flash
* 		len			is the byte length want to write
*
* Returns    : 	TLS_FLS_STATUS_OK	if write flash success
* 		TLS_FLS_STATUS_EPERM	if flash struct point is null
* 		TLS_FLS_STATUS_ENODRV	if flash driver is not installed
* 		TLS_FLS_STATUS_EINVAL	if argument is invalid
*		TLS_FLS_STATUS_EIO	if io error
**********************************************************************************************************/
int tls_fls_write(u32 addr, u8 *buf, u32 len)
{
	u8 *cache;
	int err;
	u32 sector_addr;
	u32 sector_num;
	u32 write_bytes;
	u32 i;
	struct tls_fls_drv *drv;

	if (spi_fls == NULL) {
		TLS_DBGPRT_ERR("flash driver module not beed installed!\n");
		return TLS_FLS_STATUS_EPERM;
	}
	
	if (spi_fls->current_drv == NULL) {
		TLS_DBGPRT_ERR("the current spi flash driver not installed!\n");
		return TLS_FLS_STATUS_ENODRV;
	}
	
	if ((addr >= spi_fls->current_drv->total_size) || (len == 0) || (buf == NULL)) {return TLS_FLS_STATUS_EINVAL;}
	tls_os_sem_acquire(spi_fls->fls_lock, 0);
	drv = spi_fls->current_drv;
	write_bytes = ((addr + len) > drv->total_size) ? (drv->total_size - addr) : len;
	sector_addr = addr/drv->sector_size;
	sector_num = (addr + write_bytes - 1)/drv->sector_size - sector_addr + 1;

	TLS_DBGPRT_FLASH("write to flash: sector address - %d, sectors - %d.\n", sector_addr, sector_num);
	
	err = TLS_FLS_STATUS_OK;

	cache = tls_mem_alloc(drv->sector_size);
	if (cache == NULL) {
		tls_os_sem_release(spi_fls->fls_lock);
		TLS_DBGPRT_ERR("allocate sector cache memory(%dB) fail!\n", drv->sector_size);
		return TLS_FLS_STATUS_ENOMEM;
	}

	for (i = 0; i < sector_num; i++) {
		TLS_DBGPRT_FLASH("firstly, read the sector - %d to cache.\n", sector_addr + i);
		err = drv->read((sector_addr + i) * drv->sector_size, cache, drv->sector_size);
		if (err != TLS_FLS_STATUS_OK) {
			tls_os_sem_release(spi_fls->fls_lock);
			TLS_DBGPRT_ERR("flash read fail(sector %d)!\n", (sector_addr + i));
			break;
		}
		
		if ((i == 0) && (i != (sector_num - 1))) {
			MEMCPY(cache+(addr%drv->sector_size), buf, drv->sector_size - (addr%drv->sector_size));
			buf += drv->sector_size - (addr%drv->sector_size);
			write_bytes -= drv->sector_size - (addr%drv->sector_size);
		} else if (i == (sector_num - 1)) {
			MEMCPY(cache+(addr%drv->sector_size), buf, write_bytes);
			buf += write_bytes;
			write_bytes = 0;
		} else {
			MEMCPY(cache, buf, drv->sector_size);
			buf += drv->sector_size;
			write_bytes -= drv->sector_size;
		}

		TLS_DBGPRT_FLASH("second, erase the sector - %d.\n", sector_addr + i);
		err = drv->erase(sector_addr + i);
		if (err != TLS_FLS_STATUS_OK) {
			tls_os_sem_release(spi_fls->fls_lock);
			TLS_DBGPRT_ERR("flash erase fail(sector %d)!\n", (sector_addr + i));
			break;
		}

		TLS_DBGPRT_FLASH("finnaly, write the data in cache to the sector - %d.\n", sector_addr + i);
		err = tls_fls_page_write((sector_addr + i) * (drv->sector_size/drv->page_size), cache, drv->sector_size/drv->page_size);
		if (err != TLS_FLS_STATUS_OK) {
			tls_os_sem_release(spi_fls->fls_lock);
			TLS_DBGPRT_ERR("flash write fail(sector %d)!\n", (sector_addr + i));
			break;
		}
	}

	tls_mem_free(cache);
	tls_os_sem_release(spi_fls->fls_lock);
	return err;
}

static u8 *gsflscache = NULL;
static u32 gsSecOffset = 0;
static u32 gsSector = 0;


static void tls_fls_flush_sector(void)
{
	struct tls_fls_drv *drv;
	int page, pagepersector;

	drv = spi_fls->current_drv;
	drv->erase(gsSector);
	pagepersector = drv->sector_size / drv->page_size;
	page = gsSector * pagepersector;
	tls_fls_page_write(page, gsflscache, pagepersector);
	gsSecOffset = 0;
}

int tls_fls_fast_write_init(void)
{

	if((spi_fls == NULL) || (spi_fls->current_drv == NULL))
	{
		TLS_DBGPRT_ERR("flash driver module not beed installed!\n");
		return TLS_FLS_STATUS_EPERM;
	}
	if(NULL != gsflscache)
	{
		TLS_DBGPRT_ERR("tls_fls_fast_write_init installed!\n");
		return -1;
	}
	gsflscache = tls_mem_alloc(spi_fls->current_drv->sector_size);
	if(NULL == gsflscache)
	{
		TLS_DBGPRT_ERR("tls_fls_fast_write_init malloc err!\n");
		return -1;
	}
	return 0;
}

void tls_fls_fast_write_destroy(void)
{
	if(NULL != gsflscache)
	{
		tls_fls_flush_sector();
		tls_mem_free(gsflscache);
		gsflscache = NULL;
	}
}

int tls_fls_fast_write(u32 addr, u8 *buf, u32 length)
{
	struct tls_fls_drv *drv;
	u32 sector, offset, maxlen, len;

	if((spi_fls == NULL) || (spi_fls->current_drv == NULL))
	{
		TLS_DBGPRT_ERR("flash driver module not beed installed!\n");
		return TLS_FLS_STATUS_EPERM;
	}
	if ((addr >= spi_fls->current_drv->total_size) 
		|| (length == 0) || (buf == NULL)) 
	{
		return TLS_FLS_STATUS_EINVAL;
	}
	tls_os_sem_acquire(spi_fls->fls_lock, 0);
	drv = spi_fls->current_drv;
	sector = addr / drv->sector_size;
	offset = addr % drv->sector_size;
	maxlen = drv->sector_size;
	
	if((sector != gsSector) && (gsSecOffset != 0))
	{
		tls_fls_flush_sector();
	}
	gsSector = sector;
	if(offset > 0) 
	{
		maxlen -= offset;
	}
	while(length > 0)
	{
		len = (length > maxlen) ?  maxlen : length;
		MEMCPY(gsflscache + offset, buf, len);
		gsSecOffset += len;
		if(gsSecOffset >= drv->sector_size)
		{
			tls_fls_flush_sector();
			gsSector++;
		}
		offset = 0;
		maxlen = drv->sector_size;
		sector++;
		buf += len;
		length -= len;
	}
	tls_os_sem_release(spi_fls->fls_lock);
	return 0;
}

int tls_fls_erase(u32 sector)
{
	int err;
	struct tls_fls_drv *drv;

	if (spi_fls == NULL) {
		TLS_DBGPRT_ERR("flash driver module not beed installed!\n");
		return TLS_FLS_STATUS_EPERM;
	}
	
	if (spi_fls->current_drv == NULL) {
		TLS_DBGPRT_ERR("the current spi flash driver not installed!\n");
		return TLS_FLS_STATUS_ENODRV;
	}
	
	if (sector >= (spi_fls->current_drv->total_size/spi_fls->current_drv->sector_size)) {
		TLS_DBGPRT_ERR("the sector to be erase overflow!\n");
		return TLS_FLS_STATUS_EINVAL;
	}
	tls_os_sem_acquire(spi_fls->fls_lock, 0);
	drv = spi_fls->current_drv;

	err = TLS_FLS_STATUS_OK;

	err = drv->erase(sector);
	tls_os_sem_release(spi_fls->fls_lock);
	return err;
}

int tls_fls_chip_erase(void)
{
	int err;
	struct tls_fls_drv *drv;

	if (spi_fls == NULL) {
		TLS_DBGPRT_ERR("flash driver module not beed installed!\n");
		return TLS_FLS_STATUS_EPERM;
	}
	
	if (spi_fls->current_drv == NULL) {
		TLS_DBGPRT_ERR("the current spi flash driver not installed!\n");
		return TLS_FLS_STATUS_ENODRV;
	}

	drv = spi_fls->current_drv;

	err = TLS_FLS_STATUS_OK;

	err = drv->chip_erase();

	return err;
}

int tls_fls_get_param(u8 type, void *param)
{
	int err;
	struct tls_fls_drv *drv;

	if (spi_fls == NULL) {
		TLS_DBGPRT_ERR("flash driver module not beed installed!\n");
		return TLS_FLS_STATUS_EPERM;
	}
	
	if (spi_fls->current_drv == NULL) {
		TLS_DBGPRT_ERR("the current spi flash driver not installed!\n");
		return TLS_FLS_STATUS_ENODRV;
	}
	
	if (param == NULL) {return TLS_FLS_STATUS_EINVAL;}
	tls_os_sem_acquire(spi_fls->fls_lock, 0);
	drv = spi_fls->current_drv;
	err = TLS_FLS_STATUS_OK;
	switch (type) {
		case TLS_FLS_PARAM_TYPE_ID:
			*((u32 *)param) = drv->id;
			break;
			
		case TLS_FLS_PARAM_TYPE_SIZE:
			*((u32 *)param) = drv->total_size;
			break;
			
		case TLS_FLS_PARAM_TYPE_PAGE_SIZE:
			*((u32 *)param) = drv->page_size;
			break;
			
		case TLS_FLS_PARAM_TYPE_PROG_SIZE:
			*((u32 *)param) = drv->program_size;
			break;
			
		case TLS_FLS_PARAM_TYPE_SECTOR_SIZE:
			*((u32 *)param) = drv->sector_size;
			break;
			
		default:
			TLS_DBGPRT_WARNING("invalid parameter ID!\n");
			err = TLS_FLS_STATUS_EINVAL;
			break;
	}
	tls_os_sem_release(spi_fls->fls_lock);
	return err;
}

int tls_fls_drv_register(struct tls_fls_drv *fls_drv)
{
	u32 cpu_sr;
	struct tls_fls_drv *drv;
	
	if (fls_drv == NULL) {
		TLS_DBGPRT_ERR("flash driver module not beed installed!\n");
		return TLS_FLS_STATUS_EINVAL;
	}

	dl_list_for_each (drv, &spi_fls->fls_drvs, struct tls_fls_drv, drv_list) {
		if (drv->id == fls_drv->id) {
			TLS_DBGPRT_WARNING("corresponding spi flash driver has registered!\n");
			return TLS_FLS_STATUS_EEXIST;
		}
	}

	cpu_sr = tls_os_set_critical();
	dl_list_add_tail((struct dl_list*)&spi_fls->fls_drvs, (struct dl_list*)&fls_drv->drv_list);
	tls_os_release_critical(cpu_sr);

	TLS_DBGPRT_FLASH("the spi flash driver is registered successfully!\n");

	return TLS_FLS_STATUS_OK;
}

int tls_fls_drv_unregister(struct tls_fls_drv *fls_drv)
{
	TLS_DBGPRT_WARNING("unregister spi flash driver operation is not supported!\n");
	return TLS_FLS_STATUS_EPERM;
}

int tls_fls_probe(void)
{
	int err;
	u32 id;
	struct tls_fls_drv *drv;
	
	if (spi_fls == NULL) {
		TLS_DBGPRT_ERR("flash driver module not beed installed!\n");
		return TLS_FLS_STATUS_EPERM;
	}
	if (spi_fls->current_drv != NULL) {
		TLS_DBGPRT_ERR("the current spi flash has fount the matched driver!\n");
		return TLS_FLS_STATUS_EBUSY;
	}

	TLS_DBGPRT_FLASH("try to read the current spi flash ID.\n");
	err = fls_read_id(&id);
	if (err != TLS_FLS_STATUS_OK) {return err;}

	TLS_DBGPRT_FLASH("current spi flash ID - 0x%x.\n", id);

	dl_list_for_each (drv, &spi_fls->fls_drvs, struct tls_fls_drv, drv_list) {
		if (drv->id == id) {
			err = drv->probe();
			if (err != TLS_FLS_STATUS_OK) {continue;}

			tls_spi_setup(drv->mode, drv->cs_active, drv->clock);

			TLS_DBGPRT_FLASH("matched the spi flash driver.\n");
			spi_fls->current_drv = drv;
			break;
		}
	}

	if (spi_fls->current_drv == NULL) {
		TLS_DBGPRT_WARNING("not found the matched spi flash driver!\n");
		return TLS_FLS_STATUS_ENODRV;
	}

	return TLS_FLS_STATUS_OK;
}

int tls_fls_init(void)
{
	struct tls_fls *fls;
	int err;
	
	if (spi_fls != NULL) {
		TLS_DBGPRT_ERR("flash driver module has been installed!\n");
		return TLS_FLS_STATUS_EBUSY;
	}

	fls = (struct tls_fls *)tls_mem_alloc(sizeof(struct tls_fls));
	if (fls == NULL) {
		TLS_DBGPRT_ERR("allocate @spi_fls fail!\n");
		return TLS_FLS_STATUS_ENOMEM;
	}

	memset(fls, 0, sizeof(*fls));
	dl_list_init((struct dl_list*)&fls->fls_drvs);
	err = tls_os_sem_create(&fls->fls_lock, 1);
	if (err != TLS_OS_SUCCESS) 
	{
		tls_mem_free(fls);
		TLS_DBGPRT_ERR("create semaphore @fls_lock fail!\n");
		return TLS_FLS_STATUS_ENOMEM;
	}
	spi_fls = fls;

	return TLS_FLS_STATUS_OK;
}

int tls_fls_exit(void)
{
	TLS_DBGPRT_FLASH("Not support flash driver module uninstalled!\n");
	return TLS_FLS_STATUS_EPERM;
}
 
