/**
******************************************************************************
* @file     zc_hf_adpter.c
* @authors  cxy
* @version  V1.0.0
* @date     10-Sep-2014
* @brief    Event
******************************************************************************
*/
#include <zc_protocol_controller.h>
#include <zc_timer.h>
#include <zc_module_interface.h>
#include <zc_hf_adpter.h>
#include <stdlib.h>
#include "errno.h"
#include "wm_osal.h"
#include "wm_flash.h"
#include "wm_sockets.h"
#include "lwip/inet.h"
#include "ac_cfg.h"
#include "ac_api.h"
#include "wm_include.h"
#include "wm_fwup.h"

#define USER_FLASH_PARAM1_ADDR	(0xF8000)
#define USER_FLASH_PARAM2_ADDR	(0xF9000)

extern PTC_ProtocolCon  g_struProtocolController;
PTC_ModuleAdapter g_struHfAdapter;

MSG_Buffer g_struRecvBuffer;
MSG_Buffer g_struRetxBuffer;
MSG_Buffer g_struClientBuffer;


MSG_Queue  g_struRecvQueue;
MSG_Buffer g_struSendBuffer[MSG_BUFFER_SEND_MAX_NUM];
MSG_Queue  g_struSendQueue;

u8 g_u8MsgBuildBuffer[MSG_BULID_BUFFER_MAXLEN];
u8 g_u8ClientSendLen = 0;


u16 g_u16TcpMss;
u16 g_u16LocalPort;


u8 g_u8recvbuffer[HF_MAX_SOCKET_LEN];
ZC_UartBuffer g_struUartBuffer;
HF_TimerInfo g_struHfTimer[ZC_TIMER_MAX_NUM];
tls_os_sem_t *g_struTimermutex;
u8  g_u8BcSendBuffer[100];
u32 g_u32BcSleepCount;
struct sockaddr_in struRemoteAddr;

#define TASK_HF_Cloudfunc_STK_SIZE           200            /* Size of each task's stacks (# of WORDs)  */
#define TASK_HF_CloudRecvfunc_STK_SIZE       200
OS_STK TaskHFCloudfuncStk[TASK_HF_Cloudfunc_STK_SIZE];        /* Tasks stacks */
OS_STK TaskHFCloudRecvfuncStk[TASK_HF_CloudRecvfunc_STK_SIZE];

u32 g_u32session_id;

extern tls_os_queue_t *App_R_Q;
//extern tls_os_queue_t *App_S_Q;
#if 1
u8  g_u8EqVersion[]={0,0,0,0};
u8  g_u8ModuleKey[ZC_MODULE_KEY_LEN] = DEFAULT_IOT_PRIVATE_KEY;
u64  g_u64Domain = ((((u64)SUB_DOMAIN_ID)<<56) + (((u64)MAJOR_DOMAIN_ID)<<40)) ;
u8  g_u8DeviceId[ZC_HS_DEVICE_ID_LEN] = DEVICE_ID;
#endif
/*************************************************
* Function: HF_ReadRegisterInfoFromFlash
* Description:
* Author: cxy
* Returns:
* Parameter:
* History:
*************************************************/
void HF_ReadRegisterInfoFromFlash(void)
{
    //read register info
    tls_fls_read(USER_FLASH_PARAM2_ADDR, (u8 *)(&g_struRegisterInfo), sizeof(ZC_RegisterInfo));
}
/*************************************************
* Function: HF_ReadDataFromFlash
* Description:
* Author: cxy
* Returns:
* Parameter:
* History:
*************************************************/
void HF_ReadDataFromFlash(void)
{
    u32 u32MagicFlag = 0xFFFFFFFF;

    tls_fls_read(USER_FLASH_PARAM1_ADDR, (u8 *)(&u32MagicFlag), 4);
    if (ZC_MAGIC_FLAG == u32MagicFlag)
    {
        ZC_Printf("flash para\n");
        tls_fls_read(USER_FLASH_PARAM1_ADDR, (u8 *)(&g_struZcConfigDb), sizeof(ZC_ConfigDB));
    }
    else
    {
        ZC_Printf("no para, use default\n");
    }
}

/*************************************************
* Function: HF_WriteDataToFlash
* Description:
* Author: cxy
* Returns:
* Parameter:
* History:
*************************************************/
void HF_WriteDataToFlash(u8 *pu8Data, u16 u16Len)
{
    tls_fls_write(USER_FLASH_PARAM1_ADDR, (u8*)pu8Data, u16Len);
}
/*************************************************
* Function: HF_get_timer_id
* Description:
* Author: cxy
* Returns:
* Parameter:
* History:
*************************************************/
u8 HF_get_timer_id(hftimer_handle_t handle)
{
    u8 u8TimerId;

    for(u8TimerId = 0; u8TimerId < ZC_TIMER_MAX_NUM; ++u8TimerId)
    {
        if(g_struHfTimer[u8TimerId].struHandle == handle)
        {
            return u8TimerId;
        }
    }

    return 0xff;
}

/*************************************************
* Function: HF_timer_callback
* Description:
* Author: cxy
* Returns:
* Parameter:
* History:
*************************************************/
void HF_timer_callback(void *ptmr, void *parg)
{
    hftimer_handle_t htimer = ptmr;
    u8 u8TimerId;
    tls_os_sem_acquire(g_struTimermutex, 0);
    u8TimerId = HF_get_timer_id(htimer);
    if (1 == g_struHfTimer[u8TimerId].u32FirstFlag)
    {
        g_struHfTimer[u8TimerId].u32FirstFlag = 0;
        tls_os_timer_start(htimer);
    }
    else
    {
        TIMER_TimeoutAction(u8TimerId);
        TIMER_StopTimer(u8TimerId);
    }

    tls_os_sem_release(g_struTimermutex);
}

/*************************************************
* Function: HF_StopTimer
* Description:
* Author: cxy
* Returns:
* Parameter:
* History:
*************************************************/
void HF_StopTimer(u8 u8TimerIndex)
{
    //tls_os_timer_stop(g_struHfTimer[u8TimerIndex].struHandle);
    tls_os_timer_Del(g_struHfTimer[u8TimerIndex].struHandle);
}

/*************************************************
* Function: HF_SetTimer
* Description:
* Author: cxy
* Returns:
* Parameter:
* History:
*************************************************/
u32 HF_SetTimer(u8 u8Type, u32 u32Interval, u8 *pu8TimeIndex)
{
    tls_os_status_t status;
    u8 u8TimerIndex;
    u32 u32Retval;

    u32Retval = TIMER_FindIdleTimer(&u8TimerIndex);
    if (ZC_RET_OK == u32Retval)
    {
        status = tls_os_timer_create(&g_struHfTimer[u8TimerIndex].struHandle,
                                HF_timer_callback,
                                0, u32Interval/20,
                                false, NULL);
        if(TLS_OS_SUCCESS != status)
        {
            ZC_Printf("fengq: create timer error, 0x%x!\n", status);
            return ZC_RET_ERROR;
        }
        
        TIMER_AllocateTimer(u8Type, u8TimerIndex, (u8*)&g_struHfTimer[u8TimerIndex]);
        g_struHfTimer[u8TimerIndex].u32FirstFlag = 1;
        tls_os_timer_start((tls_os_timer_t *)g_struHfTimer[u8TimerIndex].struHandle);
        *pu8TimeIndex = u8TimerIndex;
    }
    else
    {
        ZC_Printf("fengq: find no timer!\n");
    }

    return u32Retval;
}

/*************************************************
* Function: HF_FirmwareUpdateFinish
* Description:
* Author: cxy
* Returns:
* Parameter:
* History:
*************************************************/
u32 HF_FirmwareUpdateFinish(u32 u32TotalLen)
{
    tls_fwup_exit(g_u32session_id);
    return ZC_RET_OK;
}


/*************************************************
* Function: HF_FirmwareUpdate
* Description:
* Author: cxy
* Returns:
* Parameter:
* History:
*************************************************/
u32 HF_FirmwareUpdate(u8 *pu8FileData, u32 u32Offset, u32 u32DataLen)
{
    int retval;

    if (0 == u32Offset)
    {
        g_u32session_id = tls_fwup_enter(TLS_FWUP_IMAGE_SRC_WEB);
        if(g_u32session_id == 0)
        {
            return ZC_RET_ERROR;
        }
    }

    retval = tls_fwup_request_sync(g_u32session_id, pu8FileData, u32DataLen);
    if (retval < 0)
    {
        return ZC_RET_ERROR;
    }
    
    return ZC_RET_OK;

}

/*************************************************
* Function: HF_SendDataToMoudle
* Description:
* Author: cxy
* Returns:
* Parameter:
* History:
*************************************************/
u32 HF_SendDataToMoudle(u8 *pu8Data, u16 u16DataLen)
{
    tls_os_status_t Status;
    u8*msg  = tls_mem_alloc(u16DataLen);
    MEMCPY(msg,pu8Data,u16DataLen);
    Status = tls_os_queue_send(App_R_Q, msg, u16DataLen);
    if(Status)
    {
        ZC_Printf("fengq: send message error!\n");
    }

    return ZC_RET_OK;
}


/*************************************************
* Function: HF_Rest
* Description:
* Author: cxy
* Returns:
* Parameter:
* History:
*************************************************/
void HF_Rest(void)
{
    tls_wifi_set_oneshot_flag(1);
}
/*************************************************
* Function: HF_SendTcpData
* Description:
* Author: cxy
* Returns:
* Parameter:
* History:
*************************************************/
void HF_SendTcpData(u32 u32Fd, u8 *pu8Data, u16 u16DataLen, ZC_SendParam *pstruParam)
{
    u16 u16SendLen = 0;
    s32 s32TmpLen;

    while(u16SendLen < u16DataLen)
    {
        s32TmpLen = send(u32Fd, pu8Data + u16SendLen, u16DataLen - u16SendLen, 0);
        if(s32TmpLen < 0)
        {
            ZC_Printf("fengq: send error!\n");
            return;
        }
        u16SendLen += s32TmpLen;
    }
}
/*************************************************
* Function: HF_SendUdpData
* Description:
* Author: cxy
* Returns:
* Parameter:
* History:
*************************************************/
void HF_SendUdpData(u32 u32Fd, u8 *pu8Data, u16 u16DataLen, ZC_SendParam *pstruParam)
{
    sendto(u32Fd,(char*)pu8Data,u16DataLen,0,
        (struct sockaddr *)pstruParam->pu8AddrPara,
        sizeof(struct sockaddr_in));
}

/*************************************************
* Function: HF_CloudRecvfunc
* Description:
* Author: cxy
* Returns:
* Parameter:
* History:
*************************************************/
void HF_CloudRecvfunc(void* arg)
{
    s32 s32RecvLen=0;
    fd_set fdread;
    u32 u32Index;
    u32 u32Len=0;
    u32 u32ActiveFlag = 0;
    struct sockaddr_in cliaddr;
    int connfd;
    extern u8 g_u8ClientStart;
    u32 u32MaxFd = 0;
    struct timeval timeout;
    struct sockaddr_in addr;
    int tmp=1;

    while(1)
    {
        tls_os_time_delay(10);
        ZC_StartClientListen();

        u32ActiveFlag = 0;

        timeout.tv_sec= 0;
        timeout.tv_usec= 1000;

        FD_ZERO(&fdread);

        FD_SET(g_Bcfd, &fdread);
        u32MaxFd = u32MaxFd > g_Bcfd ? u32MaxFd : g_Bcfd;

        if (PCT_INVAILD_SOCKET != g_struProtocolController.struClientConnection.u32Socket)
        {
            FD_SET(g_struProtocolController.struClientConnection.u32Socket, &fdread);
            u32MaxFd = u32MaxFd > g_struProtocolController.struClientConnection.u32Socket ? u32MaxFd : g_struProtocolController.struClientConnection.u32Socket;
            u32ActiveFlag = 1;
        }

        if ((g_struProtocolController.u8MainState >= PCT_STATE_WAIT_ACCESSRSP)
        && (g_struProtocolController.u8MainState < PCT_STATE_DISCONNECT_CLOUD))
        {
            FD_SET(g_struProtocolController.struCloudConnection.u32Socket, &fdread);
            u32MaxFd = u32MaxFd > g_struProtocolController.struCloudConnection.u32Socket ? u32MaxFd : g_struProtocolController.struCloudConnection.u32Socket;
            u32ActiveFlag = 1;
        }


        for (u32Index = 0; u32Index < ZC_MAX_CLIENT_NUM; u32Index++)
        {
            if (0 == g_struClientInfo.u32ClientVaildFlag[u32Index])
            {
                FD_SET(g_struClientInfo.u32ClientFd[u32Index], &fdread);
                u32MaxFd = u32MaxFd > g_struClientInfo.u32ClientFd[u32Index] ? u32MaxFd : g_struClientInfo.u32ClientFd[u32Index];
                u32ActiveFlag = 1;
            }
        }


        if (0 == u32ActiveFlag)
        {
            continue;
        }

        select(u32MaxFd + 1, &fdread, NULL, NULL, &timeout);

        if ((g_struProtocolController.u8MainState >= PCT_STATE_WAIT_ACCESSRSP)
        && (g_struProtocolController.u8MainState < PCT_STATE_DISCONNECT_CLOUD))
        {
            if (FD_ISSET(g_struProtocolController.struCloudConnection.u32Socket, &fdread))
            {
                s32RecvLen = recv(g_struProtocolController.struCloudConnection.u32Socket, g_u8recvbuffer, HF_MAX_SOCKET_LEN, 0);
                if(s32RecvLen > 0)
                {
                    ZC_Printf("recv data len = %d\n", s32RecvLen);
                    MSG_RecvDataFromCloud(g_u8recvbuffer, s32RecvLen);
                }
                else
                {
                    ZC_Printf("recv error, len = %d\n",s32RecvLen);
                    PCT_DisConnectCloud(&g_struProtocolController);

                    g_struUartBuffer.u32Status = MSG_BUFFER_IDLE;
                    g_struUartBuffer.u32RecvLen = 0;
                }
            }

        }


        for (u32Index = 0; u32Index < ZC_MAX_CLIENT_NUM; u32Index++)
        {
            if (0 == g_struClientInfo.u32ClientVaildFlag[u32Index])
            {
                if (FD_ISSET(g_struClientInfo.u32ClientFd[u32Index], &fdread))
                {
                    s32RecvLen = recv(g_struClientInfo.u32ClientFd[u32Index], g_u8recvbuffer, HF_MAX_SOCKET_LEN, 0);
                    if (s32RecvLen > 0)
                    {
                        ZC_RecvDataFromClient(g_struClientInfo.u32ClientFd[u32Index], g_u8recvbuffer, s32RecvLen);
                    }
                    else
                    {
                        ZC_ClientDisconnect(g_struClientInfo.u32ClientFd[u32Index]);
                        closesocket(g_struClientInfo.u32ClientFd[u32Index]);
                    }

                }
            }

        }

        if (PCT_INVAILD_SOCKET != g_struProtocolController.struClientConnection.u32Socket)
        {
            if (FD_ISSET(g_struProtocolController.struClientConnection.u32Socket, &fdread))
            {
                connfd = accept(g_struProtocolController.struClientConnection.u32Socket,(struct sockaddr *)&cliaddr,&u32Len);

                if (ZC_RET_ERROR == ZC_ClientConnect((u32)connfd))
                {
                    closesocket(connfd);
                }
                else
                {
                    ZC_Printf("accept client = %d\n", connfd);
                }
            }
        }

        if (FD_ISSET(g_Bcfd, &fdread))
        {
            tmp = sizeof(addr);
            s32RecvLen = recvfrom(g_Bcfd, g_u8BcSendBuffer, 100, 0, (struct sockaddr *)&addr, (socklen_t*)&tmp);
            if(s32RecvLen > 0)
            {
                ZC_SendClientQueryReq(g_u8BcSendBuffer, (u16)s32RecvLen);
            }
        }
    }
}

/*************************************************
* Function: HF_ConnectToCloud
* Description:
* Author: cxy
* Returns:
* Parameter:
* History:
*************************************************/
u32 HF_ConnectToCloud(PTC_Connection *pstruConnection)
{
    int fd;
    struct sockaddr_in addr;
    struct ip_addr struIp;
    int retval;
    u16 port;
    struct hostent* HostEntry;
    memset((char*)&addr,0,sizeof(addr));
    if (1 == g_struZcConfigDb.struSwitchInfo.u32ServerAddrConfig)
    {
        ZC_Printf("fengq: connect cloud test addr!\n");
        port = g_struZcConfigDb.struSwitchInfo.u16ServerPort;
        struIp.addr = htonl(g_struZcConfigDb.struSwitchInfo.u32ServerIp);
        retval = HF_SUCCESS;
    }
    else
    {
        port = ZC_CLOUD_PORT;
        HostEntry = gethostbyname((const char *)g_struZcConfigDb.struCloudInfo.u8CloudAddr);
        if(HostEntry)
        {
            struIp.addr = *((u32 *)HostEntry->h_addr_list[0]);
            retval = HF_SUCCESS;
        }
        else
        {
            retval = HF_FAIL;
        }
    }

    if (HF_SUCCESS != retval)
    {
        return ZC_RET_ERROR;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr=struIp.addr;
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0)
    {
        ZC_Printf("fengq: socket error!\n");
        return ZC_RET_ERROR;
    }

    ZC_Printf("fengq: addr = %x, prot = 0x%x\n", addr.sin_addr.s_addr, addr.sin_port);
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        ZC_Printf("fengq: connect cloud failed!\n");
        closesocket(fd);
        if(g_struProtocolController.struCloudConnection.u32ConnectionTimes++ > 20)
        {
           g_struZcConfigDb.struSwitchInfo.u32ServerAddrConfig = 0;
        }

        return ZC_RET_ERROR;
    }

    g_struProtocolController.struCloudConnection.u32ConnectionTimes = 0;
    g_struProtocolController.struCloudConnection.u32Socket = fd;


    ZC_Rand(g_struProtocolController.RandMsg);

    return ZC_RET_OK;
}
/*************************************************
* Function: HF_ConnectToCloud
* Description:
* Author: cxy
* Returns:
* Parameter:
* History:
*************************************************/
u32 HF_ListenClient(PTC_Connection *pstruConnection)
{
    int fd;
    struct sockaddr_in servaddr;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd<0)
        return ZC_RET_ERROR;

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servaddr.sin_port = htons(pstruConnection->u16Port);
    if(bind(fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        closesocket(fd);
        return ZC_RET_ERROR;
    }

    if (listen(fd, TCP_DEFAULT_LISTEN_BACKLOG) < 0)
    {
        closesocket(fd);
        return ZC_RET_ERROR;
    }

    ZC_Printf("Tcp Listen Port = %d\n", pstruConnection->u16Port);
    g_struProtocolController.struClientConnection.u32Socket = fd;

    return ZC_RET_OK;
}

/*************************************************
* Function: HF_BcInit
* Description:
* Author: cxy
* Returns:
* Parameter:
* History:
*************************************************/
void HF_BcInit(void)
{
    int tmp=1;
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(ZC_MOUDLE_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    g_Bcfd = socket(AF_INET, SOCK_DGRAM, 0);

    tmp = 1;
    setsockopt(g_Bcfd, SOL_SOCKET, SO_BROADCAST, &tmp, sizeof(tmp));

    bind(g_Bcfd, (struct sockaddr*)&addr, sizeof(addr));
    g_struProtocolController.u16SendBcNum = 0;

    memset((char*)&struRemoteAddr, 0, sizeof(struRemoteAddr));
    struRemoteAddr.sin_family = AF_INET;
    struRemoteAddr.sin_port = htons(ZC_MOUDLE_BROADCAST_PORT);
    struRemoteAddr.sin_addr.s_addr=inet_addr("255.255.255.255");
    g_pu8RemoteAddr = (u8*)&struRemoteAddr;
    g_u32BcSleepCount = 10;

    return;
}

/*************************************************
* Function: HF_Cloudfunc
* Description:
* Author: cxy
* Returns:
* Parameter:
* History:
*************************************************/
void HF_Cloudfunc(void* arg)
{
    int fd;
    u32 u32Timer = 0;
    HF_BcInit();

    while(1)
    {
        tls_os_time_delay(10);
        fd = g_struProtocolController.struCloudConnection.u32Socket;
        PCT_Run();

        if (PCT_STATE_DISCONNECT_CLOUD == g_struProtocolController.u8MainState)
        {
            closesocket(fd);
            u32Timer = rand();
            u32Timer = (PCT_TIMER_INTERVAL_RECONNECT) * (u32Timer % 10 + 1);
            ZC_Printf("reconect timer = %d\n", u32Timer);
            PCT_ReconnectCloud(&g_struProtocolController, u32Timer);
            g_struUartBuffer.u32Status = MSG_BUFFER_IDLE;
            g_struUartBuffer.u32RecvLen = 0;
        }
        else
        {
            MSG_SendDataToCloud((u8*)&g_struProtocolController.struCloudConnection);
        }
        ZC_SendBc();
    }
}

/*************************************************
* Function: HF_Init
* Description:
* Author: cxy
* Returns:
* Parameter:
* History:
*************************************************/
void HF_Init()
{
    tls_os_status_t status;
    ZC_Printf("MT Init\n");
    g_struHfAdapter.pfunConnectToCloud = HF_ConnectToCloud;
    g_struHfAdapter.pfunListenClient = HF_ListenClient;
    g_struHfAdapter.pfunSendTcpData = HF_SendTcpData;
    g_struHfAdapter.pfunUpdate = HF_FirmwareUpdate;
    g_struHfAdapter.pfunUpdateFinish = HF_FirmwareUpdateFinish;
    g_struHfAdapter.pfunSendToMoudle = HF_SendDataToMoudle;
    g_struHfAdapter.pfunSetTimer = HF_SetTimer;
    g_struHfAdapter.pfunStopTimer = HF_StopTimer;

    g_struHfAdapter.pfunRest = HF_Rest;
    g_struHfAdapter.pfunWriteFlash = HF_WriteDataToFlash;
    g_struHfAdapter.pfunSendUdpData = HF_SendUdpData;


    g_u16TcpMss = 1000;
    PCT_Init(&g_struHfAdapter);

    g_struUartBuffer.u32Status = MSG_BUFFER_IDLE;
    g_struUartBuffer.u32RecvLen = 0;

    status = tls_os_task_create(NULL, NULL, HF_Cloudfunc, (void *)0,
                           (void *)TaskHFCloudfuncStk, TASK_HF_Cloudfunc_STK_SIZE * sizeof(u32),
                           59, 0);
    if(status)
    {
        ZC_Printf("fengq: create HF_Cloudfunc task error!\n");
    }

    status = tls_os_task_create(NULL, NULL, HF_CloudRecvfunc, (void *)0,
                            (void *)TaskHFCloudRecvfuncStk, TASK_HF_CloudRecvfunc_STK_SIZE * sizeof(u32),
                            60, 0);
    if(status)
    {
        ZC_Printf("fengq: create HF_CloudRecvfunc task error!\n");
    }

    status = tls_os_sem_create(&g_struTimermutex, 1);
    if(status)
    {
        ZC_Printf("fengq: create timer mutex error!\n");
    }

    g_struZcConfigDb.struSwitchInfo.u32SecSwitch = 0;
    g_struZcConfigDb.struSwitchInfo.u32TraceSwitch = 0;
    g_struZcConfigDb.struSwitchInfo.u32WifiConfig = 0;
    g_struZcConfigDb.struSwitchInfo.u32ServerAddrConfig = 0;
    memcpy(g_struZcConfigDb.struCloudInfo.u8CloudAddr, "test.ablecloud.cn", ZC_CLOUD_ADDR_MAX_LEN);
}

/*************************************************
* Function: HF_WakeUp
* Description:
* Author: cxy
* Returns:
* Parameter:
* History:
*************************************************/
void HF_WakeUp()
{
    PCT_WakeUp();
    if(PCT_TIMER_INVAILD != g_struProtocolController.u8RegisterTimer)
    {
        TIMER_StopTimer(g_struProtocolController.u8RegisterTimer);
        g_struProtocolController.u8RegisterTimer = PCT_TIMER_INVAILD;
    }
#if 1    
    memcpy(g_struRegisterInfo.u8PrivateKey, g_u8ModuleKey, ZC_MODULE_KEY_LEN);
    memcpy(g_struRegisterInfo.u8DeviciId, g_u8DeviceId, ZC_HS_DEVICE_ID_LEN);
    memcpy(g_struRegisterInfo.u8DeviciId + ZC_HS_DEVICE_ID_LEN, &g_u64Domain, ZC_DOMAIN_LEN);
    memcpy(g_struRegisterInfo.u8EqVersion, g_u8EqVersion, ZC_EQVERSION_LEN);
    tls_fls_write(USER_FLASH_PARAM2_ADDR, (u8 *)(&g_struRegisterInfo), sizeof(ZC_RegisterInfo));

    tls_fls_read(USER_FLASH_PARAM2_ADDR, (u8 *)(&g_struRegisterInfo), sizeof(ZC_RegisterInfo));
#endif

    HF_ReadRegisterInfoFromFlash();
    g_struProtocolController.u8MainState = PCT_STATE_ACCESS_NET; 
}
/*************************************************
* Function: HF_Sleep
* Description:
* Author: cxy
* Returns:
* Parameter:
* History:
*************************************************/
void HF_Sleep()
{
    u32 u32Index;

    closesocket(g_Bcfd);

    if (PCT_INVAILD_SOCKET != g_struProtocolController.struClientConnection.u32Socket)
    {
        closesocket(g_struProtocolController.struClientConnection.u32Socket);
        g_struProtocolController.struClientConnection.u32Socket = PCT_INVAILD_SOCKET;
    }

    if (PCT_INVAILD_SOCKET != g_struProtocolController.struCloudConnection.u32Socket)
    {
        closesocket(g_struProtocolController.struCloudConnection.u32Socket);
        g_struProtocolController.struCloudConnection.u32Socket = PCT_INVAILD_SOCKET;
    }

    for (u32Index = 0; u32Index < ZC_MAX_CLIENT_NUM; u32Index++)
    {
        if (0 == g_struClientInfo.u32ClientVaildFlag[u32Index])
        {
            closesocket(g_struClientInfo.u32ClientFd[u32Index]);
            g_struClientInfo.u32ClientFd[u32Index] = PCT_INVAILD_SOCKET;
        }
    }

    PCT_Sleep();

    g_struUartBuffer.u32Status = MSG_BUFFER_IDLE;
    g_struUartBuffer.u32RecvLen = 0;
}

/******************************* FILE END ***********************************/


