/***************************************************************************** 
* 
* File Name : wm_debug.h 
* 
* Description: debug Module 
* 
* Copyright (c) 2014 Winner Micro Electronic Design Co., Ltd. 
* All rights reserved. 
* 
* Author : dave
* 
* Date : 2014-6-14
*****************************************************************************/ 
#ifndef WM_DEBUG_H
#define WM_DEBUG_H

#include <stdio.h>
#include <stdlib.h>
#include "wm_config.h"
#include "wm_osal.h"

#if TLS_CONFIG_TLS_DEBUG
extern u32     tls_debug_level;

/**
 * Define the debugging level
 */
/* 0x0000000F - 0x00000001 */
#define DBG_INFO		    (1 << 0)
#define DBG_WARNING         (1 << 1)
#define DBG_ERR             (1 << 2)
#define DBG_DUMP            (1 << 3)
#define DBG_FLASH           (1 << 4)
#define DBG_SPI             (1 << 5)

#define TLS_DBGPRT(level, fmt, ...)				            \
do {									                        \
	if (tls_debug_level & (level)){				            \
		INT32U time = tls_os_get_time();	\
		printf("[TLS_DBG] <%d.%02d> %s : "fmt, (time/100), (time%100), __func__ , ##__VA_ARGS__);				\
		}\
} while (0)

#define TLS_DBGPRT_INFO(f, a...)     TLS_DBGPRT(DBG_INFO, f, ##a)
#define TLS_DBGPRT_WARNING(f, a...)  TLS_DBGPRT(DBG_WARNING, f, ##a)
#define TLS_DBGPRT_ERR(f, a...)      TLS_DBGPRT(DBG_ERR, f, ##a)
#define TLS_DBGPRT_FLASH(f, a...)      TLS_DBGPRT(DBG_FLASH, f, ##a)
#define TLS_DBGPRT_SPI(f, a...)      TLS_DBGPRT(DBG_SPI, f, ##a)
void TLS_DBGPRT_DUMP(char *p, u32 len);

#else /* TLS_DEBUG */

#define TLS_DBGPRT(level, fmt, ...)
#define TLS_DBGPRT_DUMP(p, len)

#define TLS_DBGPRT_INFO(f, a...)
#define TLS_DBGPRT_WARNING(f, a...)  
#define TLS_DBGPRT_ERR(f, a...)
#define TLS_DBGPRT_FLASH(f, a...)
#define TLS_DBGPRT_SPI(f, a...) 

#endif /*  end of TLS_DEBUG */

#ifdef TLS_DEBUG_COUNTERS
#define TLS_DEBUG_INC(c) (c)++
#else 
#define TLS_DEBUG_INC(c) do { } while (0)
#endif 

#endif /* end of WM_DEBUG_H */
