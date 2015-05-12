/***************************************************************************** 
* 
* File Name : wm_fls_gd25qxx.h 
* 
* Description: 
* 
* Copyright (c) 2014 Winner Microelectronics Co., Ltd. 
* All rights reserved. 
* 
* Author : dave
* 
* Date : 2014-6-4
*****************************************************************************/ 


#ifndef TLS_FLS_GD25QXX_H
#define TLS_FLS_GD25QXX_H

#define SFLASH_GD25Q32					0
#define SFLASH_GD25Q40					1
#define SFLASH_GD25Q80					2
#define SFLASH_WB25Q16					3
#define SFLASH_TYPE						SFLASH_GD25Q80

#define SPI_SCLK							(10000000) /* 10MHz. */
#if(SFLASH_TYPE == SFLASH_GD25Q32)
#define TOTAL_SIZE						0x400000
#define PAGE_SIZE						256
#define PROGRAM_SIZE					256
#define PAGE_NUMBER					16384
#define PAGE_ADDR_OFFSET				8
#define SECTOR_SIZE						4096
#define SECTOR_NUMBER					1024
#define SECTOR_ADDR_OFFSET			12
#define BLOCK32_ADDR_OFFSET			15
#define BLOCK64_ADDR_OFFSET 			16
#define FLASH_PID						0x15C8
#define FLASH_NAME						"GD25Q32"
#elif(SFLASH_TYPE == SFLASH_GD25Q40)
#define TOTAL_SIZE						0x80000
#define PAGE_SIZE						256
#define PROGRAM_SIZE					256
#define PAGE_NUMBER					2048
#define PAGE_ADDR_OFFSET				8
#define SECTOR_SIZE						4096
#define SECTOR_NUMBER					128
#define SECTOR_ADDR_OFFSET			12
#define BLOCK32_ADDR_OFFSET			15
#define BLOCK64_ADDR_OFFSET			16
#define FLASH_PID						0x12C8
#define FLASH_NAME						"GD25Q40"
#elif(SFLASH_TYPE == SFLASH_GD25Q80)
#define TOTAL_SIZE						0x100000
#define PAGE_SIZE						256
#define PROGRAM_SIZE					256
#define PAGE_NUMBER					4096
#define PAGE_ADDR_OFFSET				8
#define SECTOR_SIZE						4096
#define SECTOR_NUMBER					256
#define SECTOR_ADDR_OFFSET			12
#define BLOCK32_ADDR_OFFSET			15
#define BLOCK64_ADDR_OFFSET			16
#define FLASH_PID						0x13C8
#define FLASH_NAME						"GD25Q80"
#elif(SFLASH_TYPE == SFLASH_WB25Q16)
#define TOTAL_SIZE						0x200000
#define PAGE_SIZE						256
#define PROGRAM_SIZE					256
#define PAGE_NUMBER					8192
#define PAGE_ADDR_OFFSET				8
#define SECTOR_SIZE						4096
#define SECTOR_NUMBER					512
#define SECTOR_ADDR_OFFSET			12
#define BLOCK32_ADDR_OFFSET			15
#define BLOCK64_ADDR_OFFSET			16
#define FLASH_PID						0x14EF
#define FLASH_NAME						"WB25Q16"
#endif


/**
 *  command code define.
 */
#define GD25QXX_WRITE_ENABLE      (0x06) /* Global write enable */
#define GD25QXX_WRITE_DISABLE      (0x04) /* Global write disable */
#define GD25QXX_READ_SR1      (0x05) /* Read flash status register s0~s7 */
#define GD25QXX_READ_SR2      (0x35) /* Read flash status register s8~s15 */
#define GD25QXX_WRITE_SR      (0x01) /* Write flash status register s0~s15 */
#define GD25QXX_PAGE_PROGRAM      (0x02) /* program one page */
#define GD25QXX_DATA_READ      (0x03) /* read data from specified address */
#define GD25QXX_DATA_FAST_READ      (0x0b) /* fast read data from specified address */
#define GD25QXX_SECTOR_ERASE      (0x20) /* Sector erase */
#define GD25QXX_BLOCK32_ERASE      (0x52) /* 32KB Block erase(128 pages) */
#define GD25QXX_BLOCK64_ERASE      (0xd8) /* 64kb Block erase(256 pages) */
#define GD25QXX_CHIP_ERASE      (0xc7) /* Chip erase */
#define GD25QXX_FLASH_DEVICEID      (0x90) /* Read flash manufacturer/device ID */
#define GD25QXX_FLASH_ID      (0x9f) /* Read flash ID */


#define FLASH_STATUS_BUSY      (1 << 0)
#define FLASH_STATUS_WEL      (1 << 1)

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
**********************************************************************************************************/
int tls_fls_gd25qxx_drv_install(void);

void fls_cs_low(void);
void fls_cs_high(void);

#endif /* TLS_FLS_GD25QXX_H */

