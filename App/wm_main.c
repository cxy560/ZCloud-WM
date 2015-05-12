/*****************************************************************************
*
* File Name : wm_main.c
*
* Description: wm main
*
* Copyright (c) 2014 Winner Micro Electronic Design Co., Ltd.
* All rights reserved.
*
* Author :
*
* Date : 2014-6-14
*****************************************************************************/
#include <string.h>
#include "wm_irq.h"
#include "tls_sys.h"
#include "wm_gpio.h"
#include "wm_params.h"
#include "wm_hostspi.h"
#include "wm_flash.h"
#include "wm_fls_gd25qxx.h"
#include "wm_efuse.h"
#include "wm_debug.h"
#include "wm_netif.h"
#include "wm_fwup.h"
#include "wm_wifi_oneshot.h"
#if (GCC_COMPILE==1)
#include "wm_cmdp_hostif_gcc.h"
#else
#include "wm_cmdp_hostif.h"
#endif
#include "wm_uart_task.h"
#include "wm_config.h"
#include "wm_osal.h"
#include "wm_http_client.h"
#include "wm_cpu.h"

/* c librayr mutex */
tls_os_sem_t    *libc_sem;
#if (TLS_OS_FREERTOS && configUSE_HEAP3)
static u8 malloc_no = 0;
#endif
/*----------------------------------------------------------------------------
 *      Standard Library multithreading interface
 *---------------------------------------------------------------------------*/

#ifndef __MICROLIB
/*--------------------------- _mutex_initialize -----------------------------*/

int _mutex_initialize (u32 *mutex) {
    /* Allocate and initialize a system mutex. */
    //tls_os_sem_create(&libc_sem, 1);
    //mutex = (u32 *)libc_sem;
    return(1);
}


/*--------------------------- _mutex_acquire --------------------------------*/

void _mutex_acquire (u32 *mutex) {
    //u8 err;
    /* Acquire a system mutex, lock stdlib resources. */
#if     (TLS_OS_FREERTOS && configUSE_HEAP3)
    if(malloc_no < 2)
        malloc_no ++;
    else
#endif
        tls_os_sem_acquire(libc_sem, 0);
}


/*--------------------------- _mutex_release --------------------------------*/

void _mutex_release (u32 *mutex) {
    /* Release a system mutex, unlock stdlib resources. */
#if     (TLS_OS_FREERTOS && configUSE_HEAP3)
    if(2 == malloc_no)
        malloc_no ++;
    else if(malloc_no > 2)
#endif
            tls_os_sem_release(libc_sem);
}

#endif

u32 tls_debug_level = 0xf;

#define     TASK_START_STK_SIZE         300     /* Size of each task's stacks (# of WORDs)  */
OS_STK      TaskStartStk[TASK_START_STK_SIZE];         /* Tasks stacks */


#define FW_MAJOR_VER           0x01
#define FW_MINOR_VER           0x02
#define FW_PATCH_VER           0x15

const char FirmWareVer[4] = {
    'G',
    FW_MAJOR_VER,  /* Main version */
    FW_MINOR_VER, /* Subversion */
    FW_PATCH_VER  /* Internal version */
    };
const char HwVer[6] = {
    'H',
    0x1,
    0x0,
    0x0,
    0x0,
    0x0
};
extern const char WiFiVer[];
extern u8 tx_gain_group[];
extern void *tls_wl_init(u8 *tx_gain, u8* mac_addr);
extern int wpa_supplicant_init(u8 *mac_addr);
extern void tls_sys_auto_mode_run(void);

void task_start (void *data);


tls_os_timer_t *BLINKTIMER = NULL;
u8 net_up_status = 0;
static void tls_main_net_status_changed(u8 status)
{
    switch(status)
    {
        case NETIF_IP_NET_UP:
            net_up_status = 1;
            break;
        case NETIF_WIFI_DISCONNECTED:
            net_up_status = 0;
            break;
        default:
            break;
    }
}

static void BlinkTimerProc(void *ptmr, void *parg)
{
    #define HIO_LINK    13
    static u32 div_blink = 0;
    static u32 mode_blink = 0;
    u32 blink[][2] = {
    /************************************************
           |<--------blink------->|
           |<--light-->|
           |-----------|          |-----------|
           |           |          |           |
        ---|           |----------|           |---
        blink | light
    *************************************************/
        {22, 2},    /* mode = 0 */
        {10, 5},    /* mode = 1 */
        {2, 1},     /* mode = 2 */
    };
    u32 mode;

    mode = net_up_status?1:0;
    if(tls_fwup_get_status())
    {
        mode = 2;
    }

    if (mode != mode_blink)
    {
        mode_blink = mode;
        div_blink = 0;
    }
    tls_gpio_cfg(HIO_LINK, TLS_GPIO_DIR_OUTPUT, TLS_GPIO_ATTR_FLOATING);
    if(blink[mode_blink][0] == 0)
    {
        if (blink[mode_blink][1] == 0)
        {
            tls_gpio_write(HIO_LINK, 0);
        }
        else
        {
            tls_gpio_write(HIO_LINK, 1);
        }
    }
    else
    {
        ++div_blink;
        if (div_blink <= blink[mode_blink][1])
        {
            tls_gpio_write(HIO_LINK, 1);
        }
        else if (div_blink <= blink[mode_blink][0])
        {
            tls_gpio_write(HIO_LINK, 0);
        }
        else
        {
            div_blink = 0;
        }
    }
}

/****************/
/* main program */
/****************/
int tls_main(void)
{
#if TLS_OS_UCOS
    /* Initialize uC/OS-II   */
    OSInit();

    /* before use malloc() function, must create mutex used by c_lib */
    tls_os_sem_create(&libc_sem, 1);

#endif

    tls_os_task_create(NULL, NULL,
                    task_start,
                    (void *)0,
                    (void *)TaskStartStk,          /* 任务栈的起始地址 */
                    TASK_START_STK_SIZE * sizeof(u32), /* 任务栈的大小     */
                    1,
                    0);

    return 0;
}

void disp_task_stat_info(void)
{
#if TLS_OS_UCOS
    OS_TCB      *ptcb;
    INT8U        prio;
#if TLS_CONFIG_TLS_DEBUG
    INT8U        str[3] [8] = {"PEND", "TO", "ABORT"};
#endif
    for (prio = 0u; prio <= OS_TASK_IDLE_PRIO; prio++) {
        ptcb = OSTCBPrioTbl[prio];
        if (ptcb != (OS_TCB *)0) {           /* Make sure task 'ptcb' is ...   */
            if (ptcb != OS_TCB_RESERVED) {   /* ... still valid.               */
#if OS_TASK_PROFILE_EN > 0u
                TLS_DBGPRT_INFO("task %d TCB Stack used %d bytes, %d(total), Base:0x%x, Status:%s\n",
                        prio,
                        ptcb->OSTCBStkCount, ptcb->OSTCBStkSize*sizeof(u32), ptcb->OSTCBStkBase, str[ptcb->OSTCBStat]);
#endif
            }
        }
    }
#elif TLS_OS_FREERTOS
    char *buf = NULL;

    buf = tls_mem_alloc(1024);
    if(NULL == buf)
        return;
#if configUSE_TRACE_FACILITY
    vTaskList((signed char *)buf);
#endif
    TLS_DBGPRT_INFO("\n%s",buf);
    tls_mem_free(buf);
    buf = NULL;
#endif
}

void disp_version_info(void)
{
    TLS_DBGPRT_INFO("\n\n");
    TLS_DBGPRT_INFO("****************************************************************\n");
    TLS_DBGPRT_INFO("*                                                              *\n");
    TLS_DBGPRT_INFO("* Copyright (C) 2014 WinnerMicro Co. Ltd.                      *\n");
    TLS_DBGPRT_INFO("* All rights reserved.                                         *\n");
    TLS_DBGPRT_INFO("* WinnerMicro Firmware Version: %x.%x.%X                         *\n",
           FirmWareVer[1], FirmWareVer[2], FirmWareVer[3]);
    TLS_DBGPRT_INFO("* WinnerMicro Hardware Version: %x.%x.%x.%x.%x                      *\n",
           HwVer[1], HwVer[2], HwVer[3],HwVer[4],HwVer[5]);
    TLS_DBGPRT_INFO("*                                                              *\n");
    TLS_DBGPRT_INFO("* WinnerMicro Wi-Fi Lib Version: %x.%x.%x                         *\n",
           WiFiVer[0], WiFiVer[1], WiFiVer[2]);
    TLS_DBGPRT_INFO("****************************************************************\n");
}


/*****************************************************************************
 * Function Name        // task_start
 * Descriptor             // before create multi_task, we create a task_start task
 *                             // in this example, this task display the cpu usage
 * Input
 * Output
 * Return
 ****************************************************************************/
void task_start (void *data)
{
    extern void CreateUserTask(void);
    extern void  RestoreParamToDefault(void);
    int err = 0;
    u8 mac_addr[6];


#if TLS_OS_UCOS
    tls_irq_init();
    /* initial os ticker */
    tls_os_timer_init();

#if OS_TASK_STAT_EN > 0
    /* Initialize uC/OS-II's statistics */
    OSStatInit();

#endif
#endif

    tls_spi_slave_sel(SPI_SLAVE_FLASH);
#if CODEC_VS1053
    VS_Init();  //codec init
#endif
    tls_spi_init();
    tls_fls_init();
    tls_fls_gd25qxx_drv_install();

    tls_restore_param_from_backup();
    RestoreParamToDefault();
    //tls_param_to_default();

    err = tls_param_init();
    if (err) {
        TLS_DBGPRT_INFO("read spi param err\n");
    }
    tls_fwup_init();

    /* 读取efuse中的mac地址 */
    tls_get_mac_addr(mac_addr);
    tls_get_tx_gain(tx_gain_group);

    TLS_DBGPRT_INFO("tx gain ");
    TLS_DBGPRT_DUMP((char *)(&tx_gain_group[0]), 12);
    TLS_DBGPRT_INFO("mac addr ");
    TLS_DBGPRT_DUMP((char *)(&mac_addr[0]), 6);

    if(tls_wl_init(NULL, &mac_addr[0]) == NULL){
        TLS_DBGPRT_INFO("wl driver initial failured\n");
    }
    if (wpa_supplicant_init(mac_addr)) {
        TLS_DBGPRT_INFO("supplicant initial failured\n");
    }

    tls_ethernet_init();    // 网络初始化接口

    tls_sys_init();

#if TLS_CONFIG_HOSTIF
    tls_hostif_init();
#endif
#if (TLS_CONFIG_HOSTIF && TLS_CONFIG_HS_SPI)
    tls_hspi_init();
#endif

#if (TLS_CONFIG_HOSTIF &&TLS_CONFIG_UART)
    tls_uart_init();
#endif
#if TLS_CONFIG_HTTP_CLIENT_TASK
    http_client_task_init();
#endif

    //tls_sys_auto_mode_run();
    tls_netif_add_status_event(tls_main_net_status_changed);
//      err = tls_os_timer_create(&BLINKTIMER,
//            BlinkTimerProc,
//            NULL,
//            10,
//            TRUE,
//            NULL);

//    if(TLS_OS_SUCCESS == err){
//        tls_os_timer_start(BLINKTIMER);
//    }


    disp_version_info();
    /* User start here */
    CreateUserTask();
    tls_sys_auto_mode_run();
    for (;;)
    {
#if 1
    tls_os_time_delay(0x10000000);
#else
    tls_os_time_delay(1000);
    disp_task_stat_info();

#endif
    }
}

#if TLS_OS_UCOS
void  OSTaskReturnHook        (OS_TCB          *ptcb)
{
    (void)ptcb;
}
#endif
#if !TLS_OS_UCOS
//为了让os_cpu_a.s编译通过
void  OSIntExit (void)
{}

void  OSTaskSwHook (void)
{
}


#endif


