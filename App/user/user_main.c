#include "user_main.h"
#include "zc_hf_adpter.h"
// user data
static USER_DEVICE_INFO gstUserDeviceInfo;
// task
static tls_os_queue_t *gsUserTaskQueue = NULL;
static void *UserTaskQueue[USER_QUEUE_SIZE];
static OS_STK UserTaskStk[USER_TASK_SIZE];
// key scan
static u8 gsKeyPreStatus = 0;
static u8 gsKeyStatus = 0;

// tcp client param
#define TCP_REMOTE_PORT 1000                // 用户tcp client 连接远程端口
static u8 TcpRemoteIp[4] = {192,168,31,168};            // TCP 远程服务器的IP
static struct tls_socket_desc raw_socket_c_desc;        // socket client info

// tcp server param
#define TCP_LOCAL_PORT      2000                // 用户tcp server 本地监听端口
struct tls_socket_desc raw_socket_s_desc;           // socket server info

// udp broadcast param
#define UDP_BROAD_PORT  65534               // udp 广播端口号

static void KeyScanTimerProc(void);
static void UdpBroadTimerProc(void);
static void UserWlanStatusChangedEvent(INT8U status);
static int CreateUserTask(void);
static void UserTaskProc(void);
static void UserBroadCastMac(void);
static void create_raw_socket_client(void);
static void create_raw_socket_server(void);

/***************************************************************************
* Description: 初始化用户的设备 相关
*
* Auth: houxf
*
*Date: 2015-3-31
****************************************************************************/
void UserDeviceInit(void)
{
    tls_os_timer_t *KeyScanTimer = NULL;
    INT8S autoconnect;

    memset((void *)&gstUserDeviceInfo, 0, sizeof(USER_DEVICE_INFO));
    // check autoconnect
    tls_wifi_auto_connect_flag(WIFI_AUTO_CNT_FLAG_GET, &autoconnect);
    if(WIFI_AUTO_CNT_OFF == autoconnect)
    {
        autoconnect = WIFI_AUTO_CNT_ON;
        // 设置自动重连
        tls_wifi_auto_connect_flag(WIFI_AUTO_CNT_FLAG_SET, &autoconnect);
    }
    // timer
    tls_os_timer_create(&KeyScanTimer, KeyScanTimerProc, NULL, KEY_SCAN_TIME, TRUE, NULL);
    tls_os_timer_start(KeyScanTimer);
    tls_os_timer_create(&gstUserDeviceInfo.udp_broad_timer, UdpBroadTimerProc, NULL, UDP_BROAD_TIME, TRUE, NULL);
    // create static user task
    CreateUserTask();
}
/***************************************************************************
* Description: 键盘扫描处理
*
* Auth: houxf
*
*Date: 2015-3-31
****************************************************************************/
static void KeyScanTimerProc(void)
{
    INT16U KeyValue;

    tls_gpio_cfg(KEY_IO_ONESHOT, TLS_GPIO_DIR_INPUT, TLS_GPIO_ATTR_PULLLOW);
    gsKeyStatus = tls_gpio_read(KEY_IO_ONESHOT);
    if(gsKeyStatus)
    {
        if(gsKeyPreStatus != gsKeyStatus)
        {
            gsKeyPreStatus = gsKeyStatus;
            tls_os_queue_send(gsUserTaskQueue, (void *)MSG_ONESHOT, 0);
            printf("houxf debug oneshot key pressed \r\n");
        }
    }
    else
    {
        if(gsKeyPreStatus != gsKeyStatus)
        {
            gsKeyPreStatus = gsKeyStatus = 0;
        }
    }
}
/***************************************************************************
* Description:udp 广播timer处理函数
*
* Auth: houxf
*
*Date: 2015-3-31
****************************************************************************/
static void UdpBroadTimerProc(void)
{
    tls_os_queue_send(gsUserTaskQueue, (void *)MSG_TIMER, 0);
}
/***************************************************************************
* Description: 网络状态变化回调函数
*
* Auth: houxf
*
*Date: 2015-3-31
****************************************************************************/
static void UserWlanStatusChangedEvent(INT8U status)
{
    switch(status)
    {
        case NETIF_WIFI_JOIN_SUCCESS:
            printf("houxf debug NETIF_WIFI_JOIN_SUCCESS\r\n");
            break;
        case NETIF_WIFI_JOIN_FAILED:
            printf("houxf debug NETIF_WIFI_JOIN_FAILED\r\n");
            break;
        case NETIF_WIFI_DISCONNECTED:
            printf("houxf debug NETIF_WIFI_DISCONNECTED\r\n");
            tls_os_queue_send(gsUserTaskQueue,(void *)MSG_NET_DOWN,0);
            break;
        case NETIF_IP_NET_UP:
            printf("houxf debug NETIF_IP_NET_UP\r\n");
            tls_os_queue_send(gsUserTaskQueue, (void *)MSG_NET_UP, 0);
            break;
        default:
            break;
    }
}
/***************************************************************************
* Description: 创建用户任务
*
* Auth: houxf
*
*Date: 2015-3-31
****************************************************************************/
static int CreateUserTask(void)
{
    tls_os_queue_create(&gsUserTaskQueue, &UserTaskQueue, USER_QUEUE_SIZE, 0);
    tls_os_task_create(NULL, NULL, UserTaskProc, NULL,
                    (void *)UserTaskStk,                     /* 任务栈的起始地址 */
                    USER_TASK_SIZE * sizeof(u32),        /* 任务栈的大小     */
                    USER_TASK_PRO,
                    0);
    return WM_SUCCESS;
}
/***************************************************************************
* Description: 用户任务处理回调函数
*
* Auth: houxf
*
*Date: 2015-3-31
****************************************************************************/
static void UserTaskProc(void)
{
    void *msg;
    INT32U i;

    // 注册网络状态回调函数
    tls_netif_add_status_event(UserWlanStatusChangedEvent);
    for(;;)
    {
        tls_os_queue_receive(gsUserTaskQueue, (void **)&msg, 0, 0);
        switch((u32)msg)
        {
            case MSG_NET_UP:                // 加网成功
                printf("fengq: MSG_NET_UP\n");
                UserBroadCastMac();
                create_raw_socket_server();
                create_raw_socket_client();
                tls_os_timer_start(gstUserDeviceInfo.udp_broad_timer);
                gstUserDeviceInfo.wlan_status = 1;
                HF_WakeUp();
                break;

            case MSG_NET_DOWN:              // 网络断开
                printf("fengq: MSG_NET_DOWN\n");
                tls_socket_close(gstUserDeviceInfo.fd_tcp_client);
                tls_os_timer_stop(gstUserDeviceInfo.udp_broad_timer);
                gstUserDeviceInfo.wlan_status = 0;
                HF_Sleep();
                HF_BcInit();
                break;

            case MSG_ONESHOT:               // 启动一键配置
                printf("fengq: MSG_ONESHOT\n");
                tls_wifi_set_oneshot_flag(1);
                break;

            case MSG_SK_CLIENT_ERR:         // socket 断开
                tls_os_time_delay(200);
//                printf("fengq: MSG_SK_CLIENT_ERR\n");
                tls_socket_create(&raw_socket_c_desc);
                break;

            case MSG_SK_CLIENT_RX_DATA: // socket client 收到数据客户解析自己的数据
                printf("fengq: MSG_SK_CLIENT_RX_DATA=%s\r\n", gstUserDeviceInfo.tcp_client_rx_buff);
                break;

            case MSG_SK_SERVER_ERR:         // socket 断开
//                printf("fengq: MSG_SK_SERVER_ERR\n");
                break;

            case MSG_SK_SERVER_RX_DATA: // socket client 收到数据客户解析自己的数据
                printf("fengq: MSG_SK_SERVER_RX_DATA=%s\r\n", gstUserDeviceInfo.tcp_server_rx_buff);
                break;

            case MSG_TIMER:
//                printf("fengq: MSG_TIMER\n");
                UserBroadCastMac();
                if(gstUserDeviceInfo.fd_tcp_client)
                {
                    tls_socket_send(gstUserDeviceInfo.fd_tcp_client, "tcp client tx data test.", 24);
                    //tls_socket_send(gstUserDeviceInfo.fd_tcp_client, gstUserDeviceInfo.tcp_client_tx_buff, gstUserDeviceInfo.tcp_client_tx_len);
                }
                for(i = 0; i < TCP_SERVER_NUM; i++)
                {
                    if(gstUserDeviceInfo.fd_tcp_server[i])
                    {
                        tls_socket_send(gstUserDeviceInfo.fd_tcp_server[i], "tcp server tx data test.", 24);
                    }
                }
                break;

            default:
                break;
        }
    }
}
/***************************************************************************
* Description: udp 广播发送数据三次
*
* Auth: houxf
*
*Date: 2015-3-31
****************************************************************************/
static void StdUdpBroadCast(INT8U *buff, INT16U len)
{
    struct sockaddr_in pin;
    int idx;
    int socket_num;

    memset(&pin, 0, sizeof(struct sockaddr));
    pin.sin_family = AF_INET;                           //AF_INET表示使用IPv4
    pin.sin_addr.s_addr = htonl(0xffffffffUL);      //IPADDR_BROADCAST
    pin.sin_port = htons(UDP_BROAD_PORT);
    socket_num = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    for(idx = 0; idx < 3; idx++)
    {
        sendto(socket_num, buff, len, 0, (struct sockaddr *)&pin, sizeof(struct sockaddr));
        tls_os_time_delay(10);
    }
    closesocket(socket_num);
}

/***************************************************************************
* Description: 发送设备的mac地址
*
* Auth: houxf
*
*Date: 2015-3-31
****************************************************************************/
static void UserBroadCastMac(void)
{
    u8 mac_addr[8];

    tls_get_mac_addr(mac_addr);
    StdUdpBroadCast(mac_addr, 6);
}

/***************************************************************************
* 以下接口是TCP CLIENT 相关的代码
***************************************************************************/
/***************************************************************************
* Description: socket client 接收数据处理流程
*
* Auth: houxf
*
*Date: 2015-3-31
****************************************************************************/
static err_t  raw_sk_client_recv(u8 skt_num, struct pbuf *p, err_t err)
{
    INT32U len = 0;

    if(p->tot_len < TCP_RXBUFF_MAX)
    {
        len = p->tot_len;
    }
    else
    {
        len = TCP_RXBUFF_MAX - 1;
    }
    gstUserDeviceInfo.tcp_client_rx_len = pbuf_copy_partial(p, gstUserDeviceInfo.tcp_client_rx_buff, len, 0);
    if(gstUserDeviceInfo.tcp_client_rx_len > 0)
    {
        gstUserDeviceInfo.tcp_client_rx_buff[gstUserDeviceInfo.tcp_client_rx_len] = 0;
        tls_os_queue_send(gsUserTaskQueue, (void *)MSG_SK_CLIENT_RX_DATA, 0);
    }
    if(p)
    {
            pbuf_free(p);
    }
    return ERR_OK;
}
/***************************************************************************
* Description: socket client 连接成功回调函数
*
* Auth: houxf
*
*Date: 2015-3-31
****************************************************************************/
static err_t raw_sk_client_connected(u8 skt_num, err_t err)
{
    printf("houxf debug client connected socket num = %d, err = %d\n", skt_num, err);
    if(ERR_OK == err)
    {
        gstUserDeviceInfo.fd_tcp_client = skt_num;
    }
    return ERR_OK;
}
/***************************************************************************
* Description: socket client 错误回调函数
*
* Auth: houxf
*
*Date: 2015-3-31
****************************************************************************/
static void  raw_sk_client_err(u8 skt_num, err_t err)
{
    //printf("err socket num=%d,err=%d\n", skt_num,err);
    tls_socket_close(gstUserDeviceInfo.fd_tcp_client);
    gstUserDeviceInfo.fd_tcp_client = 0;
    tls_os_queue_send(gsUserTaskQueue, (void *)MSG_SK_CLIENT_ERR, 0);
}
/***************************************************************************
* Description: socket client poll 回调函数
*
* Auth: houxf
*
*Date: 2015-3-31
****************************************************************************/
static err_t raw_sk_client_poll(u8 skt_num)
{
    //printf("socketpoll skt_num : %d\n", skt_num);
    return ERR_OK;
}
/***************************************************************************
* Description: socket  client 创建连接
*
* Auth: houxf
*
*Date: 2015-3-31
****************************************************************************/
static void create_raw_socket_client(void)
{
    int idx;

    memset(&raw_socket_c_desc, 0, sizeof(struct tls_socket_desc));
    raw_socket_c_desc.recvf = raw_sk_client_recv;
    raw_socket_c_desc.errf = raw_sk_client_err;
    raw_socket_c_desc.pollf = raw_sk_client_poll;

    raw_socket_c_desc.cs_mode = SOCKET_CS_MODE_CLIENT;
    raw_socket_c_desc.connf = raw_sk_client_connected;

    raw_socket_c_desc.protocol = SOCKET_PROTO_TCP;
    for(idx = 0; idx < 4; idx++)
    {
        raw_socket_c_desc.ip_addr[idx] = TcpRemoteIp[idx];
    }
    raw_socket_c_desc.port = TCP_REMOTE_PORT;

    printf("houxf debug client connect to =%d.%d.%d.%d,port=%d\r\n",
        TcpRemoteIp[0], TcpRemoteIp[1],
        TcpRemoteIp[2], TcpRemoteIp[3],
        raw_socket_c_desc.port);

    if(0 == gstUserDeviceInfo.fd_tcp_client)
    {
        tls_socket_create(&raw_socket_c_desc);
    }
}
/***************************************************************************
* 以下接口是TCP SERVER 相关的代码
***************************************************************************/
/***************************************************************************
* Description: socket server 接收数据处理流程
*
* Auth: houxf
*
*Date: 2015-3-31
****************************************************************************/
static err_t  raw_sk_server_recv(u8 skt_num, struct pbuf *p, err_t err)
{
    INT32U len = 0;

    if(p->tot_len < TCP_RXBUFF_MAX)
    {
        len = p->tot_len;
    }
    else
    {
        len = TCP_RXBUFF_MAX - 1;
    }
    gstUserDeviceInfo.tcp_server_rx_len = pbuf_copy_partial(p, gstUserDeviceInfo.tcp_server_rx_buff, len, 0);
    if(gstUserDeviceInfo.tcp_server_rx_len > 0)
    {
        gstUserDeviceInfo.tcp_server_rx_buff[gstUserDeviceInfo.tcp_server_rx_len] = 0;
        tls_os_queue_send(gsUserTaskQueue, (void *)MSG_SK_SERVER_RX_DATA, 0);
    }
    if(p)
    {
            pbuf_free(p);
    }
    return ERR_OK;
}
/***************************************************************************
* Description: socket server 连接设备断开
*
* Auth: houxf
*
*Date: 2015-3-31
****************************************************************************/
static void  raw_sk_server_err(u8 skt_num, err_t err)
{
    INT32U i;

    for(i = 0; i < TCP_SERVER_NUM; i++)
    {
        if(skt_num == gstUserDeviceInfo.fd_tcp_server[i])
        {
            gstUserDeviceInfo.fd_tcp_server[i] = 0;
            return;
        }
    }
//  tls_os_queue_send(gsUserTaskQueue, (void *)MSG_SK_SERVER_ERR, 0);
}
/***************************************************************************
* Description: socket server poll 回调函数
*
* Auth: houxf
*
*Date: 2015-3-31
****************************************************************************/
static err_t raw_sk_server_poll(u8 skt_num)
{
    //printf("socketpoll skt_num : %d\n", skt_num);
    return ERR_OK;
}
/***************************************************************************
* Description: socket server 连接设备成功
*
* Auth: houxf
*
*Date: 2015-3-31
****************************************************************************/
static err_t raw_sk_server_accept(u8 skt_num, err_t err)
{
    INT32U i;

    printf("houxf debug server accept socket num=%d, err= %d\n", skt_num, err);
    if(ERR_OK == err)
    {
        for(i = 0; i < TCP_SERVER_NUM; i++)
        {
            if(0 == gstUserDeviceInfo.fd_tcp_server[i])
            {
                gstUserDeviceInfo.fd_tcp_server[i] = skt_num;
                return ERR_OK;
            }
        }
    }
    return err;
}
/***************************************************************************
* Description: socket server 创建服务
*
* Auth: houxf
*
*Date: 2015-3-31
****************************************************************************/
static void create_raw_socket_server(void)
{
    struct tls_ethif * ethif;

    ethif = tls_netif_get_ethif();
    printf("houxf debug tcp server ip=%d.%d.%d.%d\r\n", ip4_addr1(&ethif->ip_addr.addr),
        ip4_addr2(&ethif->ip_addr.addr),
        ip4_addr3(&ethif->ip_addr.addr),
        ip4_addr4(&ethif->ip_addr.addr));

    memset(&raw_socket_s_desc, 0, sizeof(struct tls_socket_desc));
    raw_socket_s_desc.recvf = raw_sk_server_recv;
    raw_socket_s_desc.errf = raw_sk_server_err;
    raw_socket_s_desc.pollf = raw_sk_server_poll;

    raw_socket_s_desc.cs_mode = SOCKET_CS_MODE_SERVER;
    raw_socket_s_desc.acceptf = raw_sk_server_accept;

    raw_socket_s_desc.protocol = SOCKET_PROTO_TCP;
    raw_socket_s_desc.port = TCP_LOCAL_PORT;
    printf("houxf debug tcp server listen port=%d\r\n", raw_socket_s_desc.port);
    tls_socket_create(&raw_socket_s_desc);
}

