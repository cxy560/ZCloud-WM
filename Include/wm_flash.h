#ifndef WM_FLASH_H
#define WM_FLASH_H

#include "wm_type_def.h"
#include "wm_osal.h"

#define TLS_FLS_STATUS_OK      (0)
#define TLS_FLS_STATUS_EINVAL      (1)
#define TLS_FLS_STATUS_EBUSY      (2)
#define TLS_FLS_STATUS_EPERM      (3)
#define TLS_FLS_STATUS_ENOSUPPORT      (4)
#define TLS_FLS_STATUS_EEXIST      (5)
#define TLS_FLS_STATUS_ENOMEM      (6)
#define TLS_FLS_STATUS_EOVERFLOW      (7)
#define TLS_FLS_STATUS_ENODEV      (8)
#define TLS_FLS_STATUS_EDEV      (9)
#define TLS_FLS_STATUS_EIO      (10)
#define TLS_FLS_STATUS_ENODRV      (11)

#define TLS_FLS_PARAM_TYPE_ID      (0)
#define TLS_FLS_PARAM_TYPE_SIZE      (1)
#define TLS_FLS_PARAM_TYPE_PAGE_SIZE      (2)
#define TLS_FLS_PARAM_TYPE_PROG_SIZE      (3)
#define TLS_FLS_PARAM_TYPE_SECTOR_SIZE      (4)

#define TLS_FLS_FLAG_UNDER_PROTECT      (1<<0)
#define TLS_FLS_FLAG_FAST_READ      (1<<1)
#define TLS_FLS_FLAG_AAAI      (1<<2)

//��ȡflash device id����
#define FLS_CMD_READ_DEV_ID     (0x90)//read device id	//(0x9f)


struct fls_list {
	struct fls_list *next;
	struct fls_list *prev;
};


struct tls_fls_drv {
	struct fls_list drv_list;
	u32 id;
	u32 total_size;
	u32 page_size;
	u32 program_size;
	u32 sector_size;
	u32 clock;
	u8 mode;
	u8 cs_active;
	u8 flags;
	int (*read)(u32, u8 *, u32);
	int (*fast_read)(u32, u8 *, u32);
	int (*page_write)(u32, u8 *);
	int (*erase)(u32);
	int (*chip_erase)(void);
	int (*probe)(void);
	void (*remove)(void);
};

struct tls_fls {
	struct fls_list fls_drvs;
	struct tls_fls_drv *current_drv;
	tls_os_sem_t *fls_lock;
};

/**********************************************************************************************************
* Description: 	This function is used to initial flash module structer.
*
* Arguments  : 	
* Returns    : 	TLS_FLS_STATUS_OK			if init sucsess
* 			TLS_FLS_STATUS_EBUSY		already inited
*			TLS_FLS_STATUS_ENOMEM		memory error
**********************************************************************************************************/
int tls_fls_init(void);

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
int tls_fls_read(u32 addr, u8 *buf, u32 len);


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
int tls_fls_write(u32 addr, u8 *buf, u32 len);

#endif /* WM_FLASH_H */
