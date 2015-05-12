/**************************************************************************
 * File Name                   : tls_sys.c
 * Author                       :
 * Version                      :
 * Date                          :
 * Description                 :
 *
 * Copyright (c) 2014 Winner Microelectronics Co., Ltd. 
 * All rights reserved.
 *
 ***************************************************************************/

#include "wm_osal.h"
#include "tls_sys.h"
#include "wm_mem.h"
#include "wm_debug.h"
#include "wm_params.h"
#include "wm_regs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wm_wifi.h"
#include "wm_netif.h"
#include "wm_sockets.h"
#include "wm_include.h"
#include "httpd.h"

#define      SYS_TASK_STK_SIZE          256 
tls_os_queue_t *msg_queue;
#if TLS_CONFIG_TLS_DEBUG
void TLS_DBGPRT_DUMP(char *p, u32 len)
{
    int i;
	if (tls_debug_level & DBG_DUMP)
    {
        printf("dump length : %d\n", len);
        for (i=0;i<len;i++) {
            printf("%02X ", p[i]);
            if ((i+1)%16 == 0 && (i+1)%32 != 0) {
                printf("- ");
            }
            if ((i+1)%32 == 0) {
                printf("\n");
            }
			if (i==2000) {
				printf("\n");
				break;
			}
				
        }
        printf("\n");
    }
}
#endif

#define ip_addr_set_zero(ipaddr)      ((ipaddr)->addr = 0)
static void sys_net_up()
{
    struct ip_addr ip_addr, net_mask, gateway, dns1, dns2;
    struct tls_param_ip ip_param;
	u8 mode;
#if TLS_CONFIG_AP
	u8 dnsname[32];
#endif
	
    tls_param_get(TLS_PARAM_ID_IP, &ip_param, FALSE);
	tls_param_get(TLS_PARAM_ID_WPROTOCOL, (void* )&mode, FALSE);
    if (ip_param.dhcp_enable && 
            (mode != IEEE80211_MODE_AP)) {
        ip_addr_set_zero(&ip_addr);
        ip_addr_set_zero(&net_mask);
        ip_addr_set_zero(&gateway); 
        tls_netif_set_addr(
                &ip_addr, &net_mask, &gateway);
         tls_dhcp_start();
    } else {
        tls_dhcp_stop();
		MEMCPY((char *)&ip_addr.addr, &ip_param.ip, 4);
        MEMCPY((char *)&net_mask.addr, &ip_param.netmask, 4);
        MEMCPY((char *)&gateway.addr, &ip_param.gateway, 4);
        tls_netif_set_addr(
                &ip_addr, &net_mask, &gateway); 
        MEMCPY((char *)&dns1.addr, &ip_param.dns1, 4);
        MEMCPY((char *)&dns2.addr, &ip_param.dns2, 4);
        tls_netif_dns_setserver(0, &dns1);
        tls_netif_dns_setserver(1, &dns2);
        tls_netif_set_up();	
    }
	
#if TLS_CONFIG_AP
    if (mode == IEEE80211_MODE_AP){	
        tls_dhcps_start();
        tls_param_get(TLS_PARAM_ID_DNSNAME, dnsname, 0);
        tls_dnss_start(dnsname);
    }
#endif	
}

static void sys_net_down()
{
#if TLS_CONFIG_AP
	u8 mode;
#endif

    tls_netif_set_down();

	/* Try to reconnect if auto_connect is set*/
	tls_auto_reconnect();
	
#if TLS_CONFIG_AP
	tls_param_get(TLS_PARAM_ID_WPROTOCOL, (void* )&mode, FALSE);
	if (mode == IEEE80211_MODE_AP){
		tls_dhcps_stop();
		tls_dnss_stop();
	}
#endif
}

void tls_auto_reconnect(void)
{
	struct tls_param_ssid ssid;

	u8 auto_reconnect=0xff;
	u8 wireless_protocol = 0;

	tls_os_time_delay(200);
	if (tls_wifi_get_oneshot_flag()){ /*oneshot config ongoing,do not reconnect WiFi*/
		return;
	}

	tls_wifi_auto_connect_flag(WIFI_AUTO_CNT_FLAG_GET, &auto_reconnect);
	if(auto_reconnect == WIFI_AUTO_CNT_OFF)
		return;
	else if(auto_reconnect == WIFI_AUTO_CNT_TMP_OFF){
		auto_reconnect = WIFI_AUTO_CNT_ON;
		tls_wifi_auto_connect_flag(WIFI_AUTO_CNT_FLAG_SET, &auto_reconnect);
		return; //tmparary return, for active "DISCONNECT" , such as AT CMD
	}

	tls_param_get(TLS_PARAM_ID_WPROTOCOL, (void *)&wireless_protocol, true);
	switch (wireless_protocol){
		case TLS_PARAM_IEEE80211_INFRA:
		{	
			struct tls_param_original_key origin_key;
			struct tls_param_bssid bssid;
			tls_param_get(TLS_PARAM_ID_BSSID, (void *)&bssid, true);
			if(bssid.bssid_enable){
				tls_param_get(TLS_PARAM_ID_ORIGIN_KEY, (void *)&origin_key, true);
				tls_wifi_connect_by_bssid(bssid.bssid, origin_key.psk, origin_key.key_length);
			}else{
				tls_param_get(TLS_PARAM_ID_SSID, (void *)&ssid, true);
				tls_param_get(TLS_PARAM_ID_ORIGIN_KEY, (void *)&origin_key, true);
				
				tls_wifi_connect(ssid.ssid, ssid.ssid_len, origin_key.psk, origin_key.key_length);
			}
		}
		break;
#if TLS_CONFIG_AP		
		case TLS_PARAM_IEEE80211_SOFTAP:
		{
			struct tls_softap_info_t* apinfo;
			struct tls_ip_info_t* ipinfo;
			struct tls_param_ip ip_param;
			struct tls_param_key  key;

			apinfo = tls_mem_alloc(sizeof(struct tls_softap_info_t));
			if(apinfo == NULL)
				return;
			ipinfo = tls_mem_alloc(sizeof(struct tls_ip_info_t));
			if(ipinfo == NULL){
				tls_mem_free(apinfo);
				return;
			}
				
			tls_param_get(TLS_PARAM_ID_SSID, (void *)&ssid, true);
			memcpy(apinfo->ssid, ssid.ssid, ssid.ssid_len);
			apinfo->ssid[ssid.ssid_len] = '\0';
			
			tls_param_get(TLS_PARAM_ID_ENCRY, (void *)&apinfo->encrypt, true);
			tls_param_get(TLS_PARAM_ID_CHANNEL, (void *)&apinfo->channel, true);
			
			tls_param_get(TLS_PARAM_ID_KEY, (void *)&key, 1);
			apinfo->keyinfo.key_len = key.key_length;
			apinfo->keyinfo.format = key.key_format;
			apinfo->keyinfo.index = key.key_index;
			memcpy(apinfo->keyinfo.key, key.psk, key.key_length);	
			
			tls_param_get(TLS_PARAM_ID_IP, &ip_param, true);
			/*ip配置信息:ip地址，掩码，dns名称*/
			memcpy(ipinfo->ip_addr, ip_param.ip, 4);
			memcpy(ipinfo->netmask, ip_param.netmask, 4);
			tls_param_get(TLS_PARAM_ID_DNSNAME, ipinfo->dnsname, 0);
			
			tls_wifi_softap_create(apinfo, ipinfo);
			if(apinfo)
				tls_mem_free(apinfo);
			if(ipinfo)
				tls_mem_free(ipinfo);
		}
		break;
#endif
#if TLS_CONFIG_IBSS
		case TLS_PARAM_IEEE80211_ADHOC:
		{
			struct tls_ibss_info_t* ibssinfo;
			struct tls_ibssip_info_t* ipinfo;
			struct tls_param_ip ip_param;
			struct tls_param_key  key;
			ibssinfo = tls_mem_alloc(sizeof(struct tls_softap_info_t));
			if(ibssinfo == NULL)
				return;
			ipinfo = tls_mem_alloc(sizeof(struct tls_ip_info_t));
			if(ipinfo == NULL){
				tls_mem_free(ibssinfo);
				return;
			}
				
			tls_param_get(TLS_PARAM_ID_SSID, (void *)&ssid, true);
			memcpy(ibssinfo->ssid, ssid.ssid, ssid.ssid_len);
			ibssinfo->ssid[ssid.ssid_len] = '\0';
			
			tls_param_get(TLS_PARAM_ID_ENCRY, (void *)&ibssinfo->encrypt, true);
			tls_param_get(TLS_PARAM_ID_CHANNEL, (void *)&ibssinfo->channel, true);
			
			tls_param_get(TLS_PARAM_ID_KEY, (void *)&key, 1);
			ibssinfo->keyinfo.key_len = key.key_length;
			ibssinfo->keyinfo.format = key.key_format;
			ibssinfo->keyinfo.index = key.key_index;
			memcpy(ibssinfo->keyinfo.key, key.psk, key.key_length);	
			
			tls_param_get(TLS_PARAM_ID_IP, &ip_param, true);
			/*ip配置信息:ip地址，掩码，dns名称*/
			memcpy(ipinfo->ip, ip_param.ip, 4);
			memcpy(ipinfo->netmask, ip_param.netmask, 4);
			memcpy(ipinfo->gateway, ip_param.gateway, 4);
			memcpy(ipinfo->dns1, ip_param.dns1, 4);
			memcpy(ipinfo->dns2, ip_param.dns2, 4);
		    tls_wifi_ibss_create(ibssinfo, ipinfo);
			if(ibssinfo)
				tls_mem_free(ibssinfo);
			if(ipinfo)
				tls_mem_free(ipinfo);
		}
		break;
#endif		
		default:
		break;
	}
	
	return; 
}


/*
 * sys task stack
 */
u32      sys_task_stk[SYS_TASK_STK_SIZE];     

void tls_sys_task(void *data)
{
    u8 err;
    u32 *msg ;
    //u8 oneshotflag = 0;
    //u8 auto_mode = 0;
    for (;;) {
        err = tls_os_queue_receive(msg_queue, (void **)&msg, 0, 0);
        if (!err) {
            switch((u32)msg) {
                case SYS_MSG_NET_UP:
                    sys_net_up();
                    break;

                case SYS_MSG_NET_DOWN:
                    sys_net_down();
                    break;
				case SYS_MSG_CONNECT_FAILED:
					tls_auto_reconnect();
					break;
               case SYS_MSG_AUTO_MODE_RUN:	
			   		tls_auto_reconnect();
			   break;				
               default:
		            break;
            }
        } else {

        }
    }
}

void tls_sys_auto_mode_run(void)
{
    tls_os_queue_send(msg_queue, (void *)SYS_MSG_AUTO_MODE_RUN, 0); 
}

static void tls_sys_net_up(void)
{
    tls_os_queue_send(msg_queue, 
            (void *)SYS_MSG_NET_UP, 0);
}
static void tls_sys_net_down(void)
{
    tls_os_queue_send(msg_queue, 
            (void *)SYS_MSG_NET_DOWN, 0);
}
static void tls_sys_connect_failed(void)
{
    tls_os_queue_send(msg_queue, 
            (void *)SYS_MSG_CONNECT_FAILED, 0);
}

static void sys_net_status_changed(u8 status)
{
#if TLS_CONFIG_TLS_DEBUG
	struct tls_ethif * ethif;
#endif

    switch(status)
    {
        case NETIF_WIFI_JOIN_SUCCESS:
            TLS_DBGPRT_INFO("join net success\n");
            tls_sys_net_up();
            break;
	case NETIF_WIFI_JOIN_FAILED:
            TLS_DBGPRT_INFO("join net failed\n");
		tls_sys_connect_failed();
		break;
        case NETIF_WIFI_DISCONNECTED:
            TLS_DBGPRT_INFO("net disconnected\n");
            tls_sys_net_down();
            break;
        case NETIF_IP_NET_UP:
#if TLS_CONFIG_TLS_DEBUG			
			ethif = tls_netif_get_ethif();
            TLS_DBGPRT_INFO("net up ==> ip = %d.%d.%d.%d\n",ip4_addr1(&ethif->ip_addr.addr),ip4_addr2(&ethif->ip_addr.addr),
		ip4_addr3(&ethif->ip_addr.addr),ip4_addr4(&ethif->ip_addr.addr));
#endif			
            break;
        default:
            break;
    }
}
int tls_sys_init()
{
    int err;
    void *msg;
#if TLS_CONFIG_WEB_SERVER
	struct tls_webs_cfg webcfg;
#endif

    /* create messge queue */
#define SYS_MSG_SIZE     20
    msg  = tls_mem_alloc(SYS_MSG_SIZE * sizeof(void *));
    if (!msg)
        return -1;

    err = tls_os_queue_create(&msg_queue,
            msg,
            SYS_MSG_SIZE, 0);
    if (err)
        return -1;

    /* create task */
    tls_os_task_create(NULL, NULL,
            tls_sys_task,
            (void *)0,
            (void *)&sys_task_stk,          /* 任务栈的起始地址 */
            SYS_TASK_STK_SIZE * sizeof(u32), /* 任务栈的大小     */
            TLS_SYS_TASK_PRIO,
            0);

    tls_netif_add_status_event(sys_net_status_changed);

#if TLS_CONFIG_WEB_SERVER
    tls_param_get(TLS_PARAM_ID_WEBS_CONFIG, (void *)&webcfg, FALSE);
    if ((webcfg.AutoRun)){
        httpd_init(webcfg.PortNum);
        printf("Web manager server start (tcp port:%d)\n\r",webcfg.PortNum);
    }
#endif
    return 0;

}

