#ifndef USER_MAIN_H
#define USER_MAIN_H

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
#include "wm_sockets.h"

#define USER_QUEUE_SIZE				64		// 用户消息队列的size
#define USER_TASK_SIZE					1024	// 用户任务栈的size
#define USER_TASK_PRO					50		// 用户任务优先级

#define KEY_SCAN_TIME					50		// 一键配置按键扫描时间500ms
#define KEY_IO_ONESHOT					11		// 一键配置使用的IO

#define MSG_NET_UP						1		// 加网成功消息
#define MSG_NET_DOWN					2		// 断网消息
#define MSG_TIMER						3		// 定时器消息
#define MSG_ONESHOT					4		// 进入一键配置消息

#define MSG_SK_CLIENT_ERR				5		// socket client 错误消息
#define MSG_SK_CLIENT_RX_DATA			6		// socket clent 接收的网络的消息

#define MSG_SK_SERVER_ERR				8		// socket client 错误消息
#define MSG_SK_SERVER_RX_DATA		9		// socket clent 接收的网络的消息

#define UDP_BROAD_TIME				(100*10)	// UDP 时间间隔10s

#define TCP_RXBUFF_MAX					512		// TCP recive buff size
#define TCP_TXBUFF_MAX					512		// TCP send buff size
#define TCP_SERVER_NUM				4		// 允许同时连接的设备数
typedef struct _USER_DEVICE_INFO
{
	INT32 wlan_status;							// 1 联网: 0 断网
	tls_os_timer_t *udp_broad_timer;				// udp 广播timer
	
	INT32U fd_tcp_client;							// 当前连接tcp client 的socket 号	
	INT8U tcp_client_rx_buff[TCP_RXBUFF_MAX];		// tcp recive buff
	INT32S tcp_client_rx_len;						// tcp recive buff len
	INT8U tcp_client_tx_buff[TCP_TXBUFF_MAX];		// tcp send buff
	INT32S tcp_client_tx_len;						// tcp send buff len

	INT32U fd_tcp_server[TCP_SERVER_NUM];		// 当前连接tcp server的socket 号
	INT8U tcp_server_rx_buff[TCP_RXBUFF_MAX];		// tcp recive buff
	INT32S tcp_server_rx_len;						// tcp recive buff len
	INT8U tcp_server_tx_buff[TCP_TXBUFF_MAX];		// tcp send buff
	INT32S tcp_server_tx_len;						// tcp send buff len

	
}USER_DEVICE_INFO;

#endif
