/***************************************************************************** 
* 
* File Name : wm_fwup.h 
* 
* Description: firmware update Module 
* 
* Copyright (c) 2014 Winner Micro Electronic Design Co., Ltd. 
* All rights reserved. 
* 
* Author : dave
* 
* Date : 2014-6-16
*****************************************************************************/ 
#ifndef WM_FWUP_H
#define WM_FWUP_H
#include "wm_osal.h"
#include "list.h"

#define TLS_FWUP_STATUS_OK      (0)
#define TLS_FWUP_STATUS_EINVALID      (1)
#define TLS_FWUP_STATUS_EMEM      (2)
#define TLS_FWUP_STATUS_EPERM      (3)
#define TLS_FWUP_STATUS_EBUSY      (4)
#define TLS_FWUP_STATUS_ESESSIONID      (5)
#define TLS_FWUP_STATUS_EIO      (6)
#define TLS_FWUP_STATUS_ESIGNATURE      (7)
#define TLS_FWUP_STATUS_ECRC      (8)
#define TLS_FWUP_STATUS_EUNDEF      (9)

#define TLS_FWUP_BOOT_IMAGE_SIGNATURE_WORD      0xa0ffff9f
#define TLS_FWUP_IMAGE_SIGNATURE_WORD      0x64365577
#define TLS_FWUP_IMAGE_SIGNATURE_WORD_B      0x62161900

#define TLS_FWUP_BLK_SIZE      512

#define TLS_FWUP_REQ_STATUS_IDLE      (0)
#define TLS_FWUP_REQ_STATUS_BUSY      (1)
#define TLS_FWUP_REQ_STATUS_SUCCESS      (2)
#define TLS_FWUP_REQ_STATUS_FIO      (3)
#define TLS_FWUP_REQ_STATUS_FSIGNATURE      (4)
#define TLS_FWUP_REQ_STATUS_FMEM      (5)
#define TLS_FWUP_REQ_STATUS_FCRC      (6)
#define TLS_FWUP_REQ_STATUS_FCOMPLETE      (7)

#define TLS_FWUP_STATE_UNDEF      (0xffff)
#define TLS_FWUP_STATE_BUSY      (1 << 0)
#define TLS_FWUP_STATE_COMPLETE      (1 << 1)
#define TLS_FWUP_STATE_ERROR_IO      (1 << 2)
#define TLS_FWUP_STATE_ERROR_SIGNATURE      (1 << 3)
#define TLS_FWUP_STATE_ERROR_MEM      (1 << 4)
#define TLS_FWUP_STATE_ERROR_CRC      (1 << 5)

#define TLS_FWUP_STATE_ERROR      (TLS_FWUP_STATE_ERROR_IO | TLS_FWUP_STATE_ERROR_SIGNATURE | TLS_FWUP_STATE_ERROR_MEM | TLS_FWUP_STATE_ERROR_CRC)

#define TLS_FWUP_DEST_SPECIFIC_FIRMWARE      (0)
#define TLS_FWUP_DEST_SPECIFIC_DATA      (1)


struct tls_fwup_boot_footer {
	u32 base;
	u32 length;
	u32 load_address;
	u32 exec_address;
	char name[16];
	u32 signature;
	u32 type;
	u32 image_checksum;
};

struct tls_fwup_image_hdr {
	u32 magic;
	u8 crc8;
	u8 dest_specific;  
	u16 dest_offset;  // unit: 4KB, valid when dest_specific is true
	u32 file_len;
	char time[4];
};

struct  tls_fwup_block {
	u16 number;	//0~Sum-1
	u16 sum;
	u8 data[TLS_FWUP_BLK_SIZE];
	u32 crc32;
	u8 pad[8];
};

enum tls_fwup_image_src {
	TLS_FWUP_IMAGE_SRC_LUART = 0,
	TLS_FWUP_IMAGE_SRC_HUART,
	TLS_FWUP_IMAGE_SRC_HSPI,
	TLS_FWUP_IMAGE_SRC_WEB
};

struct tls_fwup_request {
	struct dl_list list;
	u8 *data;
	u32 data_len;
	int status;
	void (*complete)(struct tls_fwup_request *request, void *arg);
	void *arg;
};

struct tls_fwup {
	struct dl_list wait_list;
	tls_os_sem_t *list_lock;
	
	bool busy;
	enum tls_fwup_image_src current_image_src;
	u16 current_state;
	u32 current_session_id;

	u32 received_len;
	u32 total_len;
	u32 updated_len;
	u32 program_base;
	u32 program_offset;
	s32 received_number;
};

/**********************************************************************************************************
* Description: 	This function is used to initial firmware update task.
*
* Arguments  : 	
*
* Returns    : 	TLS_FWUP_STATUS_OK			initial success
* 			TLS_FWUP_STATUS_EBUSY		already initialed
*			TLS_FWUP_STATUS_EMEM		memory error
**********************************************************************************************************/
int tls_fwup_init(void);

/**********************************************************************************************************
* Description: 	This function is used to entry firmware update.
*
* Arguments  : 	image_src			image file source,from TLS_FWUP_IMAGE_SRC_LUART to TLS_FWUP_IMAGE_SRC_WEB
*				
*
* Returns    : 		session id			not zero
*				NULL				if entry failed		
**********************************************************************************************************/
u32 tls_fwup_enter(enum tls_fwup_image_src image_src);

/**********************************************************************************************************
* Description: 	This function is used to exit firmware update.
*
* Arguments  : 	session_id			
*				
*
* Returns    : 		TLS_FWUP_STATUS_OK				exit success
*				TLS_FWUP_STATUS_EPERM			globle param is not initialed	
*				TLS_FWUP_STATUS_ESESSIONID		error session id
*				TLS_FWUP_STATUS_EBUSY			update state is busy
**********************************************************************************************************/
int tls_fwup_exit(u32 session_id);

/**********************************************************************************************************
* Description: 	This function is used to start update.
*
* Arguments  : 	session_id		current sessin id
*				data				the data want to update
*				data_len			data length
*			
* Returns    : 		TLS_FWUP_STATUS_OK				updade success
*				TLS_FWUP_STATUS_EPERM			globle param is not initialed	
*				TLS_FWUP_STATUS_ESESSIONID		error session id
*				TLS_FWUP_STATUS_EINVALID		invalid param
*				TLS_FWUP_STATUS_EMEM			memory error
*				TLS_FWUP_STATUS_EIO			write flash error
*				TLS_FWUP_STATUS_ECRC			crc error
*				TLS_FWUP_STATUS_ESIGNATURE		head error
*				TLS_FWUP_STATUS_EUNDEF			other error	
**********************************************************************************************************/
int tls_fwup_request_sync(u32 session_id, u8 *data, u32 data_len);

/**********************************************************************************************************
* Description: 	This function is used to get current update status.
*
* Arguments  : 	session_id		current sessin id
*			
* Returns    : 		current state
*				TLS_FWUP_STATUS_EUNDEF			error	
**********************************************************************************************************/
u16 tls_fwup_current_state(u32 session_id);

/**********************************************************************************************************
* Description: 	This function is used to reset update information.
*
* Arguments  : 	session_id		current sessin id
*			
* Returns    : 		TLS_FWUP_STATUS_OK				reset success
*				TLS_FWUP_STATUS_EPERM			globle param is not initialed	
*				TLS_FWUP_STATUS_ESESSIONID		error session id
*				TLS_FWUP_STATUS_EBUSY			update state is busy
**********************************************************************************************************/
int tls_fwup_reset(u32 session_id);

/**********************************************************************************************************
* Description: 	This function is used to clear error update state.
*
* Arguments  : 	session_id		current sessin id
*			
* Returns    : 		TLS_FWUP_STATUS_OK				reset success
*				TLS_FWUP_STATUS_EPERM			globle param is not initialed	
*				TLS_FWUP_STATUS_ESESSIONID		error session id
*				TLS_FWUP_STATUS_EBUSY			update state is busy
**********************************************************************************************************/
int tls_fwup_clear_error(u32 session_id);

/**********************************************************************************************************
* Description: 	This function is used to set update state to TLS_FWUP_STATE_ERROR_CRC.
*
* Arguments  : 	session_id		current sessin id
*			
* Returns    : 		TLS_FWUP_STATUS_OK				set success
*				TLS_FWUP_STATE_UNDEF			set failed
**********************************************************************************************************/
int tls_fwup_set_crc_error(u32 session_id);

/**********************************************************************************************************
* Description: 	This function is used to get update state if busy.
*
* Arguments  : 	
*			
* Returns    : 		true or fause
**********************************************************************************************************/
int tls_fwup_get_status(void);

/**********************************************************************************************************
* Description: 	This function is used to set update package number.
*
* Arguments  : 	
*			
* Returns    : 		TLS_FWUP_STATUS_OK		set success
*				TLS_FWUP_STATE_UNDEF	set fail
**********************************************************************************************************/
int tls_fwup_set_update_numer(int number);

/**********************************************************************************************************
* Description: 	This function is used to get received update package number.
*
* Arguments  : 	
*			
* Returns    : 		
**********************************************************************************************************/
int tls_fwup_get_current_update_numer(void);


/**********************************************************************************************************
* Description: 	This function is used to get current session id.
*
* Arguments  : 	
*			
* Returns    : 		session id
**********************************************************************************************************/
int tls_fwup_get_current_session_id(void);


#endif /* WM_FWUP_H */
