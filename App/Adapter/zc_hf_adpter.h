/**
******************************************************************************
* @file     zc_hf_adpter.h
* @authors  cxy
* @version  V1.0.0
* @date     10-Sep-2014
* @brief    HANDSHAKE
******************************************************************************
*/

#ifndef  __ZC_HF_ADPTER_H__
#define  __ZC_HF_ADPTER_H__

#include <zc_common.h>
#include <zc_protocol_controller.h>
#include <zc_module_interface.h>
#include "wm_osal.h"

typedef tls_os_timer_t * hftimer_handle_t;
typedef struct
{
    u32 u32FirstFlag;
    hftimer_handle_t struHandle;
}HF_TimerInfo;


#define HF_MAX_SOCKET_LEN    (1000)



typedef struct
{
    u8 u8CloudKey[36];
    u8 u8PrivateKey[112];
    u8 u8DeviceId[ZC_HS_DEVICE_ID_LEN + ZC_DOMAIN_LEN];
    u8 u8CloudAddr[20];
    u8 u8EqVersion[ZC_EQVERSION_LEN];
    u8 u8TokenKey[16];
}HF_StaInfo;


#ifdef __cplusplus
extern "C" {
#endif
void HF_Init(void);
void HF_WakeUp(void);
void HF_Sleep(void);
void HF_ReadDataFromFlash(void);
void HF_BcInit(void);

#ifdef __cplusplus
}
#endif
#endif
/******************************* FILE END ***********************************/

