/***************************************************************************** 
* 
* File Name : wm_fwup.c
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "wm_mem.h"
#include "list.h"
#include "wm_debug.h"
#include "wm_flash.h"
#include "utils.h"
#include "wm_fwup.h"
#include "wm_cpu.h"
#include "wm_wifi.h"

#define FWUP_MSG_QUEUE_SIZE      (4)

#define FWUP_TASK_STK_SIZE      (256)

#define FWUP_MSG_START_ENGINEER      (1)

static struct tls_fwup *fwup = NULL;
static tls_os_queue_t *fwup_msg_queue = NULL;
static void *fwup_task_msg[FWUP_MSG_QUEUE_SIZE];
static u32 fwup_task_stk[FWUP_TASK_STK_SIZE];
static struct tls_fwup_image_hdr current_hdr;
static u8 oneshotback = 0;

static __inline unsigned short swap_16(unsigned short v)
{
	return ((v & 0xff) << 8) | (v >> 8);
}

static __inline unsigned int swap_32(unsigned int v)
{
	return ((v & 0xff) << 24) | ((v & 0xff00) << 8) |
		((v & 0xff0000) >> 8) | (v >> 24);
}

static void fwup_scheduler(void *data)
{
	u8 *buffer;
	int err;
	u32 msg;
	u32 len;
	struct tls_fwup_request *request;
	struct tls_fwup_request *temp;
	extern int tls_fls_fast_write(u32 addr, u8 *buf, u32 length);
	extern void tls_fls_fast_write_destroy(void);

	while (1) 
	{
		err = tls_os_queue_receive(fwup_msg_queue, (void **)&msg, 0, 0);
		if(err != TLS_OS_SUCCESS) 
		{
			continue;
		}
		switch(msg) 
		{
			case FWUP_MSG_START_ENGINEER:
				if(dl_list_empty(&fwup->wait_list) == 0) 
				{
					fwup->current_state |= TLS_FWUP_STATE_BUSY;
				}
				dl_list_for_each_safe(request, temp, &fwup->wait_list, struct tls_fwup_request, list) 
				{
					request->status = TLS_FWUP_REQ_STATUS_BUSY;
					if(fwup->current_state & TLS_FWUP_STATE_ERROR) 
					{
						TLS_DBGPRT_WARNING("some error happened during firmware update, so discard all the request in the waiting queue!\n");
						if(fwup->current_state & TLS_FWUP_STATE_ERROR_IO) 
						{
							request->status = TLS_FWUP_REQ_STATUS_FIO;
						}
						else if(fwup->current_state & TLS_FWUP_STATE_ERROR_SIGNATURE) 
						{
							request->status = TLS_FWUP_REQ_STATUS_FSIGNATURE;
						}
						else if(fwup->current_state & TLS_FWUP_STATE_ERROR_MEM) 
						{	
							request->status = TLS_FWUP_REQ_STATUS_FMEM;
						}
						else if(fwup->current_state & TLS_FWUP_STATE_ERROR_CRC) 
						{
							request->status = TLS_FWUP_REQ_STATUS_FCRC;
						}
						goto request_finish;
					} 
					else if(fwup->current_state & TLS_FWUP_STATE_COMPLETE) 
					{
						TLS_DBGPRT_WARNING("the firmware updating conpletes, so discard the request in the waiting queue!\n");
						request->status = TLS_FWUP_REQ_STATUS_FCOMPLETE;
						goto request_finish;
					}
					
					if(fwup->current_image_src == TLS_FWUP_IMAGE_SRC_LUART)
					{
						struct tls_fwup_block *blk;
						
						blk = (struct tls_fwup_block *)request->data;
						buffer = blk->data;

						if(blk->number == 0) 
						{
							struct tls_fwup_boot_footer footer;

							TLS_DBGPRT_INFO("Create a fake header.\n");
							
							err = tls_fls_read(TLS_FLASH_BOOT_FOOTER_ADDR, (INT8U *)(&footer), sizeof(footer));
							if(err != TLS_FLS_STATUS_OK) 
							{
								TLS_DBGPRT_ERR("read tl6 bootable image footer fails!\n");
								request->status = TLS_FWUP_REQ_STATUS_FIO;
								fwup->current_state |= TLS_FWUP_STATE_ERROR_IO;
								goto request_finish;
							}
							fwup->program_base = (footer.base == TLS_FLASH_FIRMWARE1_ADDR) ?  TLS_FLASH_FIRMWARE2_ADDR : TLS_FLASH_FIRMWARE1_ADDR;
							fwup->total_len = blk->sum * TLS_FWUP_BLK_SIZE;
							fwup->updated_len = 0;

							/* Create a fake header */
							memset(&current_hdr, 0, sizeof(current_hdr));
							current_hdr.file_len = fwup->total_len;
							current_hdr.dest_specific = 0;

							TLS_DBGPRT_INFO("[File Info]\n\r");
							TLS_DBGPRT_INFO(" time: %d.%02d.%02d\n\r", ((current_hdr.time[0]<<8)+current_hdr.time[1]), current_hdr.time[2], current_hdr.time[3]);
							TLS_DBGPRT_INFO(" size: %d x %d = %dbytes\n\r", TLS_FWUP_BLK_SIZE, blk->sum, current_hdr.file_len);
							TLS_DBGPRT_INFO(" type: %s(%d)\n\r", ((current_hdr.dest_specific) ? "data" : "firm"), current_hdr.dest_specific);
							TLS_DBGPRT_INFO(" offset: 0x%04x\n\r", current_hdr.dest_offset);
							TLS_DBGPRT_INFO(" crc: 0x%02x\n\r", current_hdr.crc8);
						}	

						fwup->received_len += request->data_len;
						
						request->data_len = TLS_FWUP_BLK_SIZE;
						current_hdr.crc8 = calculate_crc8(current_hdr.crc8, buffer, request->data_len);

					} 
					else
					{
						buffer = request->data;
						if(fwup->received_len < sizeof(struct tls_fwup_image_hdr)) 
						{
							len = sizeof(struct tls_fwup_image_hdr) - fwup->received_len;
							if(request->data_len < len) 
							{
								len = request->data_len;
							}
							MEMCPY(&current_hdr, buffer, len);
							request->data_len -= len;
							buffer += len;
							fwup->received_len += len;

							if(fwup->received_len == sizeof(struct tls_fwup_image_hdr)) 
							{
								if(fwup->current_image_src == TLS_FWUP_IMAGE_SRC_WEB) 
								{
									current_hdr.magic = swap_32(current_hdr.magic);
									current_hdr.file_len = swap_32(current_hdr.file_len);
									current_hdr.dest_offset = swap_16(current_hdr.dest_offset);
								}
								/* received header */
								if(current_hdr.magic != TLS_FWUP_IMAGE_SIGNATURE_WORD && current_hdr.magic != TLS_FWUP_IMAGE_SIGNATURE_WORD_B) 
								{
									TLS_DBGPRT_ERR("current_hdr.magic = %x invalid fimware file because of signature error!\n", current_hdr.magic);
									request->status = TLS_FWUP_REQ_STATUS_FSIGNATURE;
									fwup->current_state |= TLS_FWUP_STATE_ERROR_SIGNATURE;
									goto request_finish;
								}

								TLS_DBGPRT_INFO("[File Info]\n\r");
								TLS_DBGPRT_INFO(" time: %d.%02d.%02d\n\r", ((current_hdr.time[0]<<8)+current_hdr.time[1]), current_hdr.time[2], current_hdr.time[3]);
								TLS_DBGPRT_INFO(" size: %d bytes\n\r", current_hdr.file_len);
								TLS_DBGPRT_INFO(" type: %s(%d)\n\r", ((current_hdr.dest_specific) ? "data" : "firm"), current_hdr.dest_specific);
								TLS_DBGPRT_INFO(" offset: 0x%04x\n\r", current_hdr.dest_offset);
								TLS_DBGPRT_INFO(" crc: 0x%02x\n\r", current_hdr.crc8);

								if (current_hdr.dest_specific == TLS_FWUP_DEST_SPECIFIC_FIRMWARE) 
								{/* firmware */
									struct tls_fwup_boot_footer footer;
									err = tls_fls_read(TLS_FLASH_BOOT_FOOTER_ADDR, (INT8U *)(&footer), sizeof(footer));
									if (err != TLS_FLS_STATUS_OK) 
									{
										request->status = TLS_FWUP_REQ_STATUS_FIO;
										fwup->current_state |= TLS_FWUP_STATE_ERROR_IO;
										goto request_finish;
									}
									if (footer.signature == TLS_FWUP_BOOT_IMAGE_SIGNATURE_WORD) 
									{
										fwup->program_base = (footer.base == TLS_FLASH_FIRMWARE1_ADDR) ? TLS_FLASH_FIRMWARE2_ADDR : TLS_FLASH_FIRMWARE1_ADDR;
									} 
									else 
									{
										fwup->program_base = TLS_FLASH_FIRMWARE1_ADDR;
									}
								} 
								else 
								{
									fwup->program_base = current_hdr.dest_offset * 0x1000;
								}
								fwup->updated_len = 0;
								fwup->total_len = current_hdr.file_len;
							}
						}

						fwup->received_len += request->data_len;
					}
					if (request->data_len > 0) 
					{
					//	TLS_DBGPRT_INFO("write the firmware image to the flash. %x\n\r", fwup->program_base + fwup->program_offset);
						err = tls_fls_fast_write(fwup->program_base + fwup->program_offset, buffer, request->data_len);
						if(err != TLS_FLS_STATUS_OK) 
						{
							TLS_DBGPRT_ERR("failed to program flash!\n");
							request->status = TLS_FWUP_REQ_STATUS_FIO;
							fwup->current_state |= TLS_FWUP_STATE_ERROR_IO;
							goto request_finish;
						}	

						fwup->program_offset += request->data_len;
						fwup->updated_len += request->data_len;

						//TLS_DBGPRT_INFO("updated: %d bytes\n" , fwup->updated_len);

						if(fwup->updated_len >= (fwup->total_len)) 
						{
							u8 crc8= 0;
							u8 *buffer_t;
							u32 len, len2, left, offset;
							u32 image_checksum = 0;

							tls_fls_fast_write_destroy();
							buffer_t = tls_mem_alloc(1024);
							if (buffer_t == NULL) 
							{
								TLS_DBGPRT_ERR("unable to varify because of no memory\n");
								request->status = TLS_FWUP_REQ_STATUS_FMEM;
								fwup->current_state |= TLS_FWUP_STATE_ERROR_MEM;
								goto request_finish;
							} 
							else 
							{
								offset = 0;
								left = fwup->total_len;
								while (left > 0) 
								{
									len = left > 1024 ? 1024 : left;
									err = tls_fls_read(fwup->program_base + offset, buffer_t, len);
									if (err != TLS_FLS_STATUS_OK) 
									{
										request->status = TLS_FWUP_REQ_STATUS_FIO;
										fwup->current_state |= TLS_FWUP_STATE_ERROR_IO;
										goto request_finish;
									}
									crc8 = calculate_crc8(crc8, buffer_t, len);

									/* calculate checksum which is used in TL6 bootheader */						
									len2 = len / 4;
									if (len & 0x03)
									{
										int i;
										for (i=0;i<(4 -(len&0x03));i++) 
										{
											*(buffer_t + len + i) = 0;
										}
										len2 += 1;
									}
									image_checksum = checksum((u32 *)buffer_t, len2, offset);

									offset += len;
									left -= len;
								}
								tls_mem_free(buffer_t);
							}
							
							if (current_hdr.crc8 != crc8) 
							{
								TLS_DBGPRT_ERR("varify incorrect[0x%02x, but 0x%02x]\n", current_hdr.crc8, crc8);
								request->status = TLS_FWUP_REQ_STATUS_FCRC;
								fwup->current_state |= TLS_FWUP_STATE_ERROR_CRC;
								goto request_finish;
							} 
							else 
							{
								if (current_hdr.dest_specific == 0) 
								{
									/* Firmware, should update boot header */
									struct tls_fwup_boot_footer footer;

									memset(&footer, 0, sizeof(footer));
									footer.signature = TLS_FWUP_BOOT_IMAGE_SIGNATURE_WORD;
									footer.base = fwup->program_base;
									footer.length = fwup->total_len;
									footer.type = 4;
									footer.load_address = footer.exec_address = TLS_SRAM_FIRMWARE_EXEC_ADDR;
									footer.image_checksum = image_checksum;
									strcpy(footer.name, "cuckoo");

									err = tls_fls_write(TLS_FLASH_BOOT_FOOTER_ADDR, (u8 *)&footer, sizeof(footer));
									if (err != TLS_FLS_STATUS_OK) 
									{
										TLS_DBGPRT_ERR("fail to update firmware load address to 0x%08x\n\r", footer.base);
										request->status = TLS_FWUP_REQ_STATUS_FIO;
										fwup->current_state |= TLS_FWUP_STATE_ERROR_IO;
										goto request_finish;
									}
									TLS_DBGPRT_INFO("update firmware load address to 0x%08x\n\r", footer.base);
								}
							}
							TLS_DBGPRT_INFO("update the firmware successfully!\n");
							fwup->current_state |= TLS_FWUP_STATE_COMPLETE;
							if (oneshotback == 1){
								tls_wifi_set_oneshot_flag(oneshotback);	// »Ö¸´Ò»¼üÅäÖÃ
							}
							
						}
					}
					request->status = TLS_FWUP_REQ_STATUS_SUCCESS;

request_finish:
					tls_os_sem_acquire(fwup->list_lock, 0);
					dl_list_del(&request->list);
					tls_os_sem_release(fwup->list_lock);
					if(dl_list_empty(&fwup->wait_list) == 1) 
					{
						fwup->current_state &= ~TLS_FWUP_STATE_BUSY;
					}
					if(request->complete) 
					{
						request->complete(request, request->arg);
					}
					if(fwup->updated_len >= (fwup->total_len)) 
					{
						tls_sys_reset();
					}
				}
				break;

			default:
				break;
		}
	}
}

void fwup_request_complete(struct tls_fwup_request *request, void *arg)
{
	tls_os_sem_t *sem;

	if((request == NULL) || (arg == NULL)) 
	{
		return;
	}
	sem = (tls_os_sem_t *)arg;
	tls_os_sem_release(sem);
}

u32 tls_fwup_enter(enum tls_fwup_image_src image_src)
{
	u32 session_id = 0;
	u32 cpu_sr;
	extern int tls_fls_fast_write_init(void);

	
	if (fwup == NULL) 
	{
		TLS_DBGPRT_INFO("fwup is null!\n");
		return 0;
	}
	if (fwup->busy == true) 
	{
		TLS_DBGPRT_INFO("fwup is busy!\n");
		return 0;
	}

	cpu_sr = tls_os_set_critical();
	
	do 
	{
		session_id = rand();
	}while(session_id == 0);

	memset(&current_hdr, 0, sizeof(current_hdr));
	
	fwup->current_state = 0;
	fwup->current_image_src = image_src;

	fwup->received_len = 0;
	fwup->total_len = 0;
	fwup->updated_len = 0;
	fwup->program_base = 0;
	fwup->program_offset = 0;
	fwup->received_number = -1;
		
	fwup->current_session_id = session_id;
	fwup->busy = true;
	oneshotback = tls_wifi_get_oneshot_flag();
	if (oneshotback == 1){
		tls_wifi_set_oneshot_flag(0);	// ÍË³öÒ»¼üÅäÖÃ
	}
	tls_wifi_set_psflag(0, 0);
	tls_fls_fast_write_init();
	tls_os_release_critical(cpu_sr);
	return session_id;
}

int tls_fwup_exit(u32 session_id)
{
	u32 cpu_sr;
	
	if ((fwup == NULL) || (fwup->busy == false)) 
	{
		return TLS_FWUP_STATUS_EPERM;
	}
	if (session_id != fwup->current_session_id) 
	{
		return TLS_FWUP_STATUS_ESESSIONID;
	}
	if (fwup->current_state & TLS_FWUP_STATE_BUSY) 
	{
		return TLS_FWUP_STATUS_EBUSY;
	}

	cpu_sr = tls_os_set_critical();

	memset(&current_hdr, 0, sizeof(current_hdr));
	
	fwup->current_state = 0;

	fwup->received_len = 0;
	fwup->total_len = 0;
	fwup->updated_len = 0;
	fwup->program_base = 0;
	fwup->program_offset = 0;
	fwup->received_number = -1;

	fwup->current_session_id = 0;
	fwup->busy = false;
	if (oneshotback == 1){
		tls_wifi_set_oneshot_flag(oneshotback); // »Ö¸´Ò»¼üÅäÖÃ
	}
	tls_wifi_set_psflag(1, 0);
	tls_os_release_critical(cpu_sr);

	return TLS_FWUP_STATUS_OK;
}

int tls_fwup_get_current_session_id(void)
{
	return fwup->current_session_id;
}

int tls_fwup_set_update_numer(int number)
{
	if(1 == number - fwup->received_number)
	{
		fwup->received_number = number;
		return TLS_FWUP_STATUS_OK;
	}
	return TLS_FWUP_STATE_UNDEF;
}

int tls_fwup_get_current_update_numer(void)
{
	return fwup->received_number;
}

int tls_fwup_get_status(void)
{
	return fwup->busy;
}

int tls_fwup_set_crc_error(u32 session_id)
{
	if(fwup == NULL) 
	{
		return TLS_FWUP_STATE_UNDEF;
	}
	if(session_id != fwup->current_session_id) 
	{
		return TLS_FWUP_STATE_UNDEF;
	}
	fwup->current_state |= TLS_FWUP_STATE_ERROR_CRC;

	return TLS_FWUP_STATUS_OK;
}

static int tls_fwup_request_async(u32 session_id, struct tls_fwup_request *request)
{
	u8 need_sched;
	
	if(fwup == NULL) 
	{
		return TLS_FWUP_STATUS_EPERM;
	}
	if(session_id != fwup->current_session_id) 
	{
		return TLS_FWUP_STATUS_ESESSIONID;
	}
	if((request == NULL) || (request->data == NULL) || (request->data_len == 0)) 
	{
		return TLS_FWUP_STATUS_EINVALID;
	}
	tls_os_sem_acquire(fwup->list_lock, 0);
	if(dl_list_empty(&fwup->wait_list)) 
	{
		need_sched = 1;
	}
	else
	{
		need_sched = 0;
	}
	request->status = TLS_FWUP_REQ_STATUS_IDLE;
	dl_list_add_tail(&fwup->wait_list, &request->list);
	tls_os_sem_release(fwup->list_lock);
	if(need_sched == 1) 
	{
		tls_os_queue_send(fwup_msg_queue, (void *)FWUP_MSG_START_ENGINEER, 0);
	}
	return TLS_FWUP_STATUS_OK;
}


int tls_fwup_request_sync(u32 session_id, u8 *data, u32 data_len)
{
	int err;
	tls_os_sem_t *sem;
	struct tls_fwup_request request;

	if(fwup == NULL) 
	{
		return TLS_FWUP_STATUS_EPERM;
	}
	if(session_id != fwup->current_session_id) 
	{
		return TLS_FWUP_STATUS_ESESSIONID;
	}
	if((data == NULL) || (data_len == 0)) 
	{
		return TLS_FWUP_STATUS_EINVALID;
	}

	err = tls_os_sem_create(&sem, 0);
	if(err != TLS_OS_SUCCESS) 
	{
		return TLS_FWUP_STATUS_EMEM;
	}
	request.data = data;
	request.data_len = data_len;
	request.complete = fwup_request_complete;
	request.arg = (void *)sem;

	err = tls_fwup_request_async(session_id, &request);
	if(err == TLS_FWUP_STATUS_OK) 
	{
		tls_os_sem_acquire(sem, 0);
	}
	tls_os_sem_delete(sem);

	switch(request.status) 
	{
		case TLS_FWUP_REQ_STATUS_SUCCESS:
			err = TLS_FWUP_STATUS_OK;
			break;

		case TLS_FWUP_REQ_STATUS_FIO:
			err = TLS_FWUP_STATUS_EIO;
			break;

		case TLS_FWUP_REQ_STATUS_FSIGNATURE:
			err = TLS_FWUP_STATUS_ESIGNATURE;
			break;

		case TLS_FWUP_REQ_STATUS_FMEM:
			err = TLS_FWUP_STATUS_EMEM;
			break;

		case TLS_FWUP_REQ_STATUS_FCRC:
			err = TLS_FWUP_STATUS_ECRC;
			break;

		case TLS_FWUP_REQ_STATUS_FCOMPLETE:
			err = TLS_FWUP_STATUS_EIO;
			break;

		default:
			err = TLS_FWUP_STATUS_EUNDEF;
			break;
	}
	return err;
}

u16 tls_fwup_current_state(u32 session_id)
{
	if(fwup == NULL) 
	{
		return TLS_FWUP_STATE_UNDEF;
	}
	if(session_id != fwup->current_session_id) 
	{
		return TLS_FWUP_STATE_UNDEF;
	}
	return fwup->current_state;
}

int tls_fwup_reset(u32 session_id)
{
	u32 cpu_sr;
	
	if ((fwup == NULL) || (fwup->busy == false)) {return TLS_FWUP_STATUS_EPERM;}
	if (session_id != fwup->current_session_id) {return TLS_FWUP_STATUS_ESESSIONID;}
	if (fwup->current_state & TLS_FWUP_STATE_BUSY) {return TLS_FWUP_STATUS_EBUSY;}

	cpu_sr = tls_os_set_critical();

	memset(&current_hdr, 0, sizeof(current_hdr));

	fwup->current_state = 0;

	fwup->received_len = 0;
	fwup->total_len = 0;
	fwup->updated_len = 0;
	fwup->program_base = 0;
	fwup->program_offset = 0;
	
	tls_os_release_critical(cpu_sr);
	
	return TLS_FWUP_STATUS_OK;
}

int tls_fwup_clear_error(u32 session_id)
{
	u32 cpu_sr;
	
	if ((fwup == NULL) || (fwup->busy == false)) {return TLS_FWUP_STATUS_EPERM;}
	if (session_id != fwup->current_session_id) {return TLS_FWUP_STATUS_ESESSIONID;}
	if (fwup->current_state & TLS_FWUP_STATE_BUSY) {return TLS_FWUP_STATUS_EBUSY;}

	cpu_sr = tls_os_set_critical();

	fwup->current_state &= ~TLS_FWUP_STATE_ERROR;
	
	tls_os_release_critical(cpu_sr);

	return TLS_FWUP_STATUS_OK;
}

int tls_fwup_init(void)
{
	int err;
	
	if(fwup != NULL) 
	{
		TLS_DBGPRT_ERR("firmware update module has been installed!\n");
		return TLS_FWUP_STATUS_EBUSY;
	}

	fwup = tls_mem_alloc(sizeof(*fwup));
	if(fwup == NULL) 
	{
		TLS_DBGPRT_ERR("allocate @fwup fail!\n");
		return TLS_FWUP_STATUS_EMEM;
	}
	memset(fwup, 0, sizeof(*fwup));
	
	err = tls_os_sem_create(&fwup->list_lock, 1);
	if(err != TLS_OS_SUCCESS) 
	{
		TLS_DBGPRT_ERR("create semaphore @fwup->list_lock fail!\n");
		tls_mem_free(fwup);
		return TLS_FWUP_STATUS_EMEM;
	}

	dl_list_init(&fwup->wait_list);
	fwup->busy = false;

	err = tls_os_queue_create(&fwup_msg_queue, (void *)fwup_task_msg, FWUP_MSG_QUEUE_SIZE, 0);
	if (err != TLS_OS_SUCCESS) 
	{
		TLS_DBGPRT_ERR("create message queue @fwup_msg_queue fail!\n");
		tls_os_sem_delete(fwup->list_lock);
		tls_mem_free(fwup);
		return TLS_FWUP_STATUS_EMEM;
	}

	err = tls_os_task_create(NULL, NULL,
						fwup_scheduler,
						(void *)fwup,
						(void *)&fwup_task_stk,
						FWUP_TASK_STK_SIZE * sizeof(u32),
						TLS_FWUP_TASK_PRIO,
						0);
	if (err != TLS_OS_SUCCESS) 
	{
		TLS_DBGPRT_ERR("create firmware update process task fail!\n");
		tls_os_queue_delete(fwup_msg_queue);
		tls_os_sem_delete(fwup->list_lock);
		tls_mem_free(fwup);
		return TLS_FWUP_STATUS_EMEM;
	}

	return TLS_FWUP_STATUS_OK;
}

