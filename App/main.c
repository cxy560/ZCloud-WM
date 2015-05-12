/*****************************************************************************
*
* File Name : main.c
*
* Description: main
*
* Copyright (c) 2014 Winner Micro Electronic Design Co., Ltd.
* All rights reserved.
*
* Author : dave
*
* Date : 2014-6-14
*****************************************************************************/
#include "wm_include.h"
#include <zc_hf_adpter.h>
#include "ac_api.h"
#include "ac_hal.h"
extern tls_os_sem_t    *libc_sem;
int main(void)
{
#if TLS_OS_UCOS
    tls_main();
    OSStart();      // 开始多任务调度
#elif TLS_OS_FREERTOS
    tls_irq_init();
    tls_os_sem_create(&libc_sem, 1);
    tls_main();
    vTaskStartScheduler();
#endif

    return 0;
}
#define APP_R_Q_SIZE                          32
tls_os_queue_t *App_R_Q = NULL;
void *App_R_Queue[APP_R_Q_SIZE];

//#define APP_S_Q_SIZE                          32
//tls_os_queue_t *App_S_Q = NULL;
//void *App_S_Queue[APP_S_Q_SIZE];

#define AppTask_STK_SIZE       200
OS_STK AppTaskStk[AppTask_STK_SIZE];
void AppTask(void* arg);

extern void UserDeviceInit(void);
void CreateUserTask(void)
{

    tls_os_status_t Status;
    printf("\n user task\n");
    UserDeviceInit();
#if DEMO_CONSOLE
    CreateDemoTask();
#endif
    /* +fengqiang*/
    //HF_Rest();
    HF_Init();
    HF_ReadDataFromFlash();

    // 创建应用任务的接受消息队列
    Status = tls_os_queue_create(&App_R_Q,
                             &App_R_Queue[0],
                             APP_R_Q_SIZE, 0);
    if(Status)
    {
        printf("fengq: create receive message queue error!\n");
    }

    // 创建应用任务
    Status = tls_os_task_create(NULL, NULL, AppTask, (void *)0,
                            (void *)AppTaskStk, AppTask_STK_SIZE * sizeof(u32),
                            31, 0);
    if(Status)
    {
        printf("fengq: create task error!\n");
    }

}

void AppTask(void* arg)
{
    ZC_MessageHead *pstruMsg;
    tls_os_status_t Status;

    while(1)
    {
        Status = tls_os_queue_receive(App_R_Q, (void **)&pstruMsg, 0, 0);
        if(Status)
        {
            printf("app: receive message error!\n");
        }

        AC_RecvMessage(pstruMsg);
        tls_mem_free(pstruMsg);
    }
}
void RestoreParamToDefault(void)
{
#if 0
    struct tls_user_param *user_param = NULL;

    user_param = (struct tls_user_param *)tls_mem_alloc(sizeof(struct tls_user_param));
    if(NULL == user_param)
    {
        return -1;
    }
    memset(user_param, 0, sizeof(*user_param));
    user_param->wireless_protocol = 0;  // sta 0; adhoc 1; ap 2
    user_param->auto_mode = 1;
    user_param->baudrate = 9600;
    user_param->user_port_mode = 0;     // LUart 0; HUart 1; HSPI 2; SDIO 3
    user_param->dhcp_enable = 1;
    user_param->auto_powersave = 0;
    user_param->ip[0] = 192;
    user_param->ip[1] = 168;
    user_param->ip[2] = 1;
    user_param->ip[3] = 1;
    user_param->netmask[0] = 255;
    user_param->netmask[1] = 255;
    user_param->netmask[2] = 255;
    user_param->netmask[3] = 0;
    user_param->gateway[0] = 192;
    user_param->gateway[1] = 168;
    user_param->gateway[2] = 1;
    user_param->gateway[3] = 1;
    user_param->dns[0] = 192;
    user_param->dns[1] = 168;
    user_param->dns[2] = 1;
    user_param->dns[3] = 1;

    user_param->socket_protocol = 0;    // TCP 0; UDP 1
    user_param->socket_client_or_server = 0;    // client 0; server 1
    user_param->socket_port_num = 1000;
    memset(user_param->socket_host, 0, 32);
    memset(user_param->PassWord, '0', 6);

    tls_param_save_user(user_param);
    tls_mem_free(user_param);

    tls_param_to_default();
#endif
}
