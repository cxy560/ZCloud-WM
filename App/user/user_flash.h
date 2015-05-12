#ifndef USER_FLASH_H
#define USER_FLASH_H

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
#include "wm_flash.h"

#define USER_FLASH_PARAM1_ADDR	(0xF8000)
#define USER_FLASH_PARAM2_ADDR	(0xF9000)
#define USER_FLASH_ID1				0xAA555253
#define USER_FLASH_ID2				0xC98980FF
#define USER_DEVICE_NUM			100

typedef struct _FLASH_MAC_INFO
{
	INT16U valid;
	INT8U mac[6];
}FLASH_MAC_INFO;

typedef struct _USER_FLASH_INFO
{
	INT32U id1;									//0xAA555253
	INT32U id2;									//0xC98980FF
	FLASH_MAC_INFO device[USER_DEVICE_NUM];		// 设备信息
}USER_FLASH_INFO;

void UserLoadFlashInfo(void);
INT32S UserAddFlashInfo(INT8U *mac);
INT32S UserDelFlashInfo(INT8U *mac);

#endif
