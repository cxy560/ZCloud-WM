#include "user_flash.h"

static USER_FLASH_INFO gstFlashInfo;	// 存储在flash中的设备信息
/*************************************************************************** 
* Description: 获取用户存储到flash的设备参数
*
* Auth: houxf
*
*Date: 2015-3-31
****************************************************************************/ 
void UserLoadFlashInfo(void)
{
	INT32U id1, id2;
	
	tls_fls_read(USER_FLASH_PARAM1_ADDR, (u8 *)&gstFlashInfo, sizeof(USER_FLASH_INFO));
	if((USER_FLASH_ID1 != gstFlashInfo.id1) || (USER_FLASH_ID2 != gstFlashInfo.id2))
	{
		// 参数1区数据错误，读参数2区
		tls_fls_read(USER_FLASH_PARAM2_ADDR, (u8 *)&gstFlashInfo, sizeof(USER_FLASH_INFO));
		if((USER_FLASH_ID1 != gstFlashInfo.id1) || (USER_FLASH_ID2 != gstFlashInfo.id2))
		{
			// 参数2区数据错误，恢复默认参数
			memset((void *)&gstFlashInfo, 0, sizeof(USER_FLASH_INFO));
			gstFlashInfo.id1 = USER_FLASH_ID1;
			gstFlashInfo.id2 = USER_FLASH_ID2;
			tls_fls_write(USER_FLASH_PARAM1_ADDR, (u8 *)&gstFlashInfo, sizeof(USER_FLASH_INFO));
			tls_fls_write(USER_FLASH_PARAM2_ADDR, (u8 *)&gstFlashInfo, sizeof(USER_FLASH_INFO));
		}
		else
		{
			//恢复参数1区数据
			tls_fls_write(USER_FLASH_PARAM1_ADDR, (u8 *)&gstFlashInfo, sizeof(USER_FLASH_INFO));
		}
	}
	else
	{
		tls_fls_read(USER_FLASH_PARAM2_ADDR, (u8 *)&id1, 4);
		tls_fls_read(USER_FLASH_PARAM2_ADDR + 4, (u8 *)&id2, 4);
		if((USER_FLASH_ID1 != id1) || (USER_FLASH_ID2 != id2))
		{
			// 参数2区数据错误，从参数1恢复参数2
			tls_fls_write(USER_FLASH_PARAM2_ADDR, (u8 *)&gstFlashInfo, sizeof(USER_FLASH_INFO));
		}
	}
}
/*************************************************************************** 
* Description: 保存用户的设备参数到flash
*
* Auth: houxf
*
*Date: 2015-3-31
****************************************************************************/ 
static void UserSaveFlashInfo(void)
{
	tls_fls_write(USER_FLASH_PARAM1_ADDR, (u8 *)&gstFlashInfo, sizeof(USER_FLASH_INFO));
	tls_fls_write(USER_FLASH_PARAM2_ADDR, (u8 *)&gstFlashInfo, sizeof(USER_FLASH_INFO));
}
/*************************************************************************** 
* Description: 添加用户的设备参数
*
* Auth: houxf
*
*Date: 2015-3-31
****************************************************************************/ 
INT32S UserAddFlashInfo(INT8U *mac)
{
	INT32U i;

	for(i = 0; i < USER_DEVICE_NUM; i++)
	{
		if(0 == gstFlashInfo.device[i].valid)
		{
			memcpy((void *)&gstFlashInfo.device[i].mac, mac, 6);
			gstFlashInfo.device[i].valid = 1;
			UserSaveFlashInfo();
			return 0;
		}
	}
	return -1;
}
/*************************************************************************** 
* Description:删除用户的设备参数
*
* Auth: houxf
*
*Date: 2015-3-31
****************************************************************************/ 
INT32S UserDelFlashInfo(INT8U *mac)
{
	INT32U i;

	for(i = 0; i < USER_DEVICE_NUM; i++)
	{
		if(0 == memcmp(gstFlashInfo.device[i].mac, mac, 6))
		{
			memset((void *)&gstFlashInfo.device[i].mac, 0, 6);
			gstFlashInfo.device[i].valid = 0;
			UserSaveFlashInfo();
			return 0;
		}
	}
	return -1;

}

