#ifndef WM_NETIF_H
#define WM_NETIF_H

#include "wm_wifi.h"
#include "wm_type_def.h"
#include "wm_params.h"

#define	NETIF_WIFI_JOIN_SUCCESS       0x1
#define   NETIF_WIFI_JOIN_FAILED        0x2
#define	NETIF_WIFI_DISCONNECTED    0x3
#define	NETIF_IP_NET_UP                    0x4

struct ip_addr {
  u32_t addr;
};
typedef struct ip_addr ip_addr_t;
struct tls_ethif {
    struct ip_addr ip_addr;
    struct ip_addr netmask;
    struct ip_addr gw;
    struct ip_addr dns1;
    struct ip_addr dns2;
    u8 status;//0:net down; 1:net up
};

//type defination of netif status changed callback.
typedef void (*tls_netif_status_event_fn)(u8 status);
/*************************************************************************** 
* Function: tls_ethernet_init 
* Description: Initialize ethernet. 
* 
* Input: ipcfg: Ip parameters. 
*           wireless_protocol: See #define TLS_PARAM_IEEE80211_XXXX
* 
* Output: None
* 
* Return: 0: Succeed. 
* 
* Date : 2014-6-10 
****************************************************************************/ 
int tls_ethernet_init(void);
/*************************************************************************** 
* Function: tls_netif_get_ethif 
* Description: Get the ip parameters stored in tls_ethif struct.
* 
* Input: None
* 
* Output: None
* 
* Return: Pointer to struct tls_ethif. 
* 
* Date : 2014-6-10  
****************************************************************************/ 
struct tls_ethif * tls_netif_get_ethif(void);
/*************************************************************************** 
* Function: tls_dhcp_start 
* Description: Start DHCP negotiation for a network interface.
* 
* Input: None
* 
* Output: None
* 
* Return: 0: Succeed; minus: Failed. 
* 
* Date : 2014-6-10  
****************************************************************************/ 
err_t tls_dhcp_start(void);
/*************************************************************************** 
* Function: tls_dhcp_stop 
* Description: Remove the DHCP client from the interface.
* 
* Input: None
* 
* Output: None
* 
* Return: 0: Succeed; minus: Failed. 
* 
* Date : 2014-6-10  
****************************************************************************/ 
err_t tls_dhcp_stop(void);
/*************************************************************************** 
* Function: tls_netif_set_addr 
* Description: Change IP address configuration for a network interface (including netmask
*                   and default gateway).
* 
* Input: ipaddr:      the new IP address
*           netmask:  the new netmask
*           gw:           the new default gateway
* 
* Output: None
* 
* Return: 0: Succeed; minus: Failed. 
* 
* Date : 2014-6-10  
****************************************************************************/ 
err_t tls_netif_set_addr(ip_addr_t *ipaddr,
                        ip_addr_t *netmask,
                        ip_addr_t *gw);
/*************************************************************************** 
* Function: tls_netif_set_addr 
* Description: Initialize one of the DNS servers.
* 
* Input: numdns:     the index of the DNS server to set must be < DNS_MAX_SERVERS
*           dnsserver:  IP address of the DNS server to set
* 
* Output: None
* 
* Return: None
* 
* Date : 2014-6-10  
****************************************************************************/ 
void tls_netif_dns_setserver(u8 numdns, ip_addr_t *dnsserver);
/*************************************************************************** 
* Function: tls_netif_set_up 
* Description: Bring an interface up, available for processing traffic.
* 
* Input: None
* 
* Output: None
* 
* Return: 0: Succeed; minus: Failed. 
* 
* Date : 2014-6-10  
****************************************************************************/ 
err_t tls_netif_set_up(void);
/*************************************************************************** 
* Function: tls_netif_set_down 
* Description: Bring an interface down, disabling any traffic processing.
* 
* Input: None
* 
* Output: None
* 
* Return: 0: Succeed; minus: Failed. 
* 
* Date : 2014-6-10  
****************************************************************************/ 
err_t tls_netif_set_down(void);
/*************************************************************************** 
* Function: tls_netif_add_status_event 
* Description: Add netif status changed callback to event list, if exists, do nothing.
* 
* Input: event_fn: A pointer to tls_netif_status_event_fn.
* 
* Output: None
* 
* Return: 0: Succeed; minus: Failed. 
* 
* Date : 2014-6-10  
****************************************************************************/ 
err_t tls_netif_add_status_event(tls_netif_status_event_fn event_fn);
/*************************************************************************** 
* Function: tls_netif_remove_status_event 
* Description: Remove netif status changed callback from event list, if not exists, do nothing.
* 
* Input: event_fn: A pointer to tls_netif_status_event_fn.
* 
* Output: None
* 
* Return: 0: Succeed; minus: Failed. 
* 
* Date : 2014-6-10  
****************************************************************************/ 
err_t tls_netif_remove_status_event(tls_netif_status_event_fn event_fn);

struct netif *tls_get_netif(void);

#if TLS_CONFIG_AP
/*************************************************************************** 
* Function: tls_dhcps_start 
* Description: Start DHCP Server for a network interface.
* 
* Input: None
* 
* Output: None
* 
* Return: The DHCP Server error code:
*             DHCPS_ERR_SUCCESS - No error
*             DHCPS_ERR_MEM - Out of memory
*             DHCPS_ERR_LINKDOWN - The NI is inactive
* 
* Date : 2014-6-10  
****************************************************************************/ 
INT8S tls_dhcps_start(void);
/*************************************************************************** 
* Function: tls_dhcps_stop 
* Description: Disable DHCP Server.
* 
* Input: None
* 
* Output: None
* 
* Return: None
* 
* Date : 2014-6-10  
****************************************************************************/ 
void tls_dhcps_stop(void);
/*************************************************************************** 
* Function: tls_dnss_start 
* Description: Start the dns server's service.
* 
* Input: DnsName: Specify the server's dns name
* 
* Output: None
* 
* Return: The DNS Server error code:
*             DNSS_ERR_SUCCESS - No error
*             DNSS_ERR_PARAM - Input parameter error
*             DNSS_ERR_MEM - Out of memory
*             DNSS_ERR_LINKDOWN - The NI is inactive
* 
* Date : 2014-6-10  
****************************************************************************/ 
INT8S tls_dnss_start(INT8U * DnsName);
/*************************************************************************** 
* Function: tls_dnss_stop 
* Description: Stop the dns server's service.
* 
* Input: None
* 
* Output: None
* 
* Return: None
* 
* Date : 2014-6-10  
****************************************************************************/ 
void tls_dnss_stop(void);
#endif //TLS_CONFIG_AP
#endif //WM_NETIF_H
