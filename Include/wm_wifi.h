#ifndef TLS_WIFI_FUNC_H
#define TLS_WIFI_FUNC_H

/* Return Error definition*/
#define	WM_WIFI_ERR_SSID		-1
#define	WM_WIFI_ERR_KEY			-2
#define WM_WIFI_WPS_BUSY		-3
#define WM_WIFI_SCANNING_BUSY	-4

#ifndef ETH_ALEN
#define ETH_ALEN 6
#endif

/* bss type definition*/
#ifndef IEEE80211_MODE_INFRA
#define IEEE80211_MODE_INFRA	0
#define IEEE80211_MODE_IBSS		1
#define IEEE80211_MODE_AP		2
#endif

#define IEEE80211_ENCRYT_NONE			0
#define IEEE80211_ENCRYT_WEP40			1
#define	IEEE80211_ENCRYT_WEP104			2
#define	IEEE80211_ENCRYT_TKIP_WPA		3
#define	IEEE80211_ENCRYT_CCMP_WPA		4
#define	IEEE80211_ENCRYT_TKIP_WPA2		5
#define	IEEE80211_ENCRYT_CCMP_WPA2		6
#define	IEEE80211_ENCRYT_AUTO_WPA		7
#define	IEEE80211_ENCRYT_AUTO_WPA2		8

struct tls_curr_bss_t{
	u8 bssid[ETH_ALEN];
	u8 ssid[32];
	u8 ssid_len;
	u8 channel;
	u8 type;
	u8 encryptype;
	u8 rssi;
};

struct tls_key_info_t{
	u8 format;
	u8 index;
	u8 key_len;
	u8 key[64];
};

struct tls_softap_info_t{
	u8 ssid[32];
	u8 encrypt;
	u8 channel;
	struct tls_key_info_t keyinfo;
};

struct tls_ip_info_t{
    u8 ip_addr[4];
    u8 netmask[4];
    u8 dnsname[32];
};

struct tls_ibss_info_t{
	u8 ssid[32];
	u8 encrypt;
	u8 channel;
	struct tls_key_info_t keyinfo;
};

struct tls_ibssip_info_t{
	u8 ip[4];
	u8 netmask[4];
	u8 gateway[4];
	u8 dns1[4];
	u8 dns2[4];
};


struct tls_bss_info_t {
    u8      bssid[ETH_ALEN];
    u8      mode;
    u8      channel;
    u8      privacy;
    u8      ssid_len;
    u8      rssi;
    u8      ssid[32]; 
};

struct tls_scan_bss_t {
    u32     count;
    u32     length; /* bss info total length */
    struct tls_bss_info_t bss[1];
};


/**********************************************************************************************************
* Description: 	This function is used to enable/disable monitor mode;
* Arguments  : 	
* Returns    :    	None
**********************************************************************************************************/
void tls_wifi_set_listen_mode(u8 enable);


/**********************************************************************************************************
* Description: 	This function is used to register recv wifi data callback function.
*
* Arguments  : 	callback: pointer to recv wifi data function			
*
* Returns    : 		None
**********************************************************************************************************/
typedef void (*tls_wifi_data_recv_callback)(u8* data, u32 data_len);

void tls_wifi_data_recv_cb_register(tls_wifi_data_recv_callback callback);

/**********************************************************************************************************
* Description: 	This function is used to set oneshot config flag
*
* Arguments  : 	flag 0:one shot  closed
* 		                    1:one shot  open
*                                2:one shot has been configured
* Returns    :   	None
**********************************************************************************************************/
void   tls_wifi_set_oneshot_flag(u8 flag);


/**********************************************************************************************************
* Description: 	This function is used to get one shot flag
*
* Arguments  : 	
*
* Returns    : 		0:	one shot  closed
* 		       	1:	one shot  open
*                   		2:	one shot has been configured
**********************************************************************************************************/
int     tls_wifi_get_oneshot_flag(void);


/**********************************************************************************************************
* Description: 	This function is used to switch channel actively
*
* Arguments  : 	Chanid
*
* Returns : 		
* Note:			Normally, it is just used in monitor mode;
**********************************************************************************************************/
void tls_wifi_change_chanel(u32 chanid);

/*************************************************************************** 
* Description: 	trigger  scanning;
* 
* Arguments: 		None
* 
* Return: 		WM_SUCCESS: will start scan;
*				WM_WIFI_SCANNING_BUSY: wifi module is scanning now.
*				WM_FAILED: other Error;
* Note:			in case not SUCCESS, user need to call this function again to trigger the scan
****************************************************************************/ 
int tls_wifi_scan(void);

/*************************************************************************** 
* Description: 	before calling tls_wifi_scan() , application should call this function to register the call back function;
* 
* Arguments: 		callback function pointer:
* 
* Return: 		None 
* Note:    		n callback function, suggest user send out a MSG and return immediately;
*				after the callback been called, the scan result can be get by calling tls_wifi_get_scan_rslt()
****************************************************************************/ 
void tls_wifi_scan_result_cb_register(void (*callback)(void));

/*************************************************************************** 
* Description:   	get result of last scan
* 
* Arguments: 	 	buffer: address to store returned BSS info
* 			 	buffer_size: the length of the buffer
* Output: 	  	BSS info got by last scan
* 
* Return: 		WM_SUCCESS: return ok;
*				WM_FAILED: failed to get result;
* Note:			user need to memalloc a buffer in advance. size for one item of scan result is 43Bytes;
*				the buffer size depends how many items user wants.
****************************************************************************/ 
int tls_wifi_get_scan_rslt(u8* buf, u32 buffer_size);


/**********************************************************************************************************
* Description: 	This function is used to create soft ap
*
* Arguments  : 	
*			  	apinfo
*             	 	ipinfo
* Returns    :  
*			  	WM_WIFI_ERR_SSID:	ssid is NULL
*			  	WM_WIFI_ERR_KEY:	key info not correct
*			    	WM_SUCCESS:		soft ap create ok
**********************************************************************************************************/
int tls_wifi_softap_create(struct tls_softap_info_t* apinfo, struct tls_ip_info_t* ipinfo);

/*************************************************************************** 
* Description: 	This function is used to create or join ibss
*
* Arguments  : 	
*			  	ibssinfo
*             	ipinfo
* Returns    :  
*			  	WM_WIFI_ERR_SSID:	ssid is NULL
*			  	WM_WIFI_ERR_KEY:	key info not correct
*			    WM_SUCCESS:		    ibss join or create ok
* 
* Date : 2014-7-25 
*****************************************************************************/
int tls_wifi_ibss_create(struct tls_ibss_info_t *ibssinfo, struct tls_ibssip_info_t *ipinfo);

/**********************************************************************************************************
* Description: 	This function is used to disconnet wifi network
*
* Arguments  : 	
* Returns    :  
**********************************************************************************************************/
void tls_wifi_disconnect(void);

/**********************************************************************************************************
* Description: 	This function is used to connect AP
*
* Arguments  : 	ssid:  SSID  Name to connect
*				ssid_len: length of SSID
* 				pwd:  password to connect AP
*				pwd_len: length of password
* Returns    : 	
*              		WM_SUCCESS: config ok, wifi will start to connect AP; 
*							 APP should register network status callback function to get result;
*							 wifi_status_change_cb just return WIFI MAC layer status; 
*							 APP should register netif status callback to get IP lay status; 		
* 		       	WM_FAILED: config fail
**********************************************************************************************************/
int tls_wifi_connect(u8 *ssid, u8 ssid_len, u8 *pwd, u8 pwd_len);


/**********************************************************************************************************
* Description: 	This function is used to connect AP by BSSID
*
* Arguments  : 	bssid:  BSSID to connect
*				pwd:  password to connect AP
* 				pwd_len: length of password
*
* Returns    : 	
*              		WM_SUCCESS: config ok
* 		       	WM_FAILED: config fail
**********************************************************************************************************/
int tls_wifi_connect_by_bssid(u8 *bssid, u8 *pwd, u8 pwd_len);


/*************************************************************************** 
* Description:  	Set auto connect mode: Enable  Disable. & Get auto connect mode parameter
* 
* Arguments:  	opt: operation mode: SET or GET
* 				mode: if opt = SET: set auto-connect enbled or disabled; 
*					   if opt = GET: return the value previous set
* 				
* Output:			if opt = GET: return the value previous set 
* 	
* Return: 		WM_SUCCESS: operation ok;
* 				WM_FAILED:	  operation failed;
* Note: 
* WIFI_AUTO_CNT_OFF			Disable/d;
* WIFI_AUTO_CNT_ON			Enable/d;
* WIFI_AUTO_CNT_TMP_OFF		For user initiated "DISCONNECT", such as AT CMD;
*				in such case, user might expect "disconnect witout reconnection, even in
*				 WIFI_AUTO_CNT_ON status; XX_TMP_OFF flag just be effective for one time of
*				 "DISCONNECT"; After that or reboot, the FLAG will be set to the previous value;
*****************************************************************************/
#define WIFI_AUTO_CNT_FLAG_SET		1
#define WIFI_AUTO_CNT_FLAG_GET		0

#define WIFI_AUTO_CNT_OFF			0x0
#define WIFI_AUTO_CNT_ON			0x1
#define WIFI_AUTO_CNT_TMP_OFF		0x3 

int tls_wifi_auto_connect_flag( u8 opt, u8* mode);

/*************************************************************************** 
* Description: 	This function is used to register wifi status changed callback function.
*
* Arguments: 			callback: pointer to recv wifi data function 
* 
* Output: : 		None
* Return: 		None 
* 
* Note :	callback func status: 
* 			WIFI_JOIN_SUCCESS; 	connect with wifi AP correctly in Wifi layer; No IP address
* 			WIFI_JOIN_FAILED;   	did not connect with wifi AP; normally,  timeout in 20s after start connection
*			WIFI_DISCONNECTED;	STA is disconnected with AP for any case, such as wifi AP shut dow, 
*								wifi AP changed password, and so on;
****************************************************************************/ 
#define	WIFI_JOIN_SUCCESS	0x1
#define WIFI_JOIN_FAILED	0x2
#define	WIFI_DISCONNECTED	0x3

void tls_wifi_status_change_cb_register(void (*callback)(u8 status));

/*************************************************************************** 
* Description: 	This function is used to register wifi status changed callback function.
*
* Arguments: 		bss: address where the network parameters will be write;
* 
* Output: 		bss 
* Return: 		None 
* 
****************************************************************************/ 
void tls_wifi_get_current_bss(struct tls_curr_bss_t* bss);

/*********************************************************************************************************
					Wifi WPS API
*********************************************************************************************************/
#ifdef TLS_CONFIG_WPS

#define WPS_PIN_LEN 	8


/**********************************************************************************************************
* Description : 	This function generate random PIN code
* Arguments  : 	pin: memory to place the pin code
* output	:      	pin code, 8 Bytes
* Returns    : 		
*              		WM_SUCCESS: OK
**********************************************************************************************************/
int tls_wps_get_pin(u8* pin);


/**********************************************************************************************************
* Description: 	set PIN code into system
* Arguments  : 	pin: 8bytes string
*				pin_len: length of @pin, should be 8 as current WPS spec
* Returns    : 	
*              		WM_SUCCESS: OK;
*				WM_FAILED: Failed
* Note :			normally, the @pin code will be hard coded during manufacturing. should not use this function;
**********************************************************************************************************/
int tls_wps_set_pin(u8* pin, u8 pin_len);


/**********************************************************************************************************
* Description: 	Start WPS process via PIN mode 
* Arguments  : 	
* Returns    : 	
*              		WM_SUCCESS: OK;
*				WM_FAILED: Failed
*				WM_WIFI_WPS_BUSY: last WPS process is not finished; normally, 120s for WPS protocol, but for us, 180s totally; 
* Note :			adaptor will use the PIN code in system for WPS process; and before that, the PIN code should be input 
*				into AP/Registrar mannually;
**********************************************************************************************************/
int tls_wps_start_pin(void);


/**********************************************************************************************************
* Description: 	Start WPS process via PBC/PushButton mode 
*
* Arguments  : 	
* Returns    : 	
*              		WM_SUCCESS: OK;
*				WM_FAILED: Failed
*				WM_WIFI_WPS_BUSY: last WPS process is not finished; normally, 120s for WPS protocol, but for us, 180s totally; 
*				if ok, adaptor will start the WPS connection; APP should wifi/netif status call back to get result;
**********************************************************************************************************/
int tls_wps_start_pbc(void);
#endif

/*************************************************************************** 
* Function: 		tls_ethernet_data_rx_callback 
*
* Arguments  : 	callback: pointer to net rx data function 
* Description: 	This function is used to register ETHERNET data rx callback function.
* 
* Output: 		None 
* 
* Return: 		None 
* 
****************************************************************************/ 
typedef int (*net_rx_data_cb)(u8 *buf, u32 buf_len);

void tls_ethernet_data_rx_callback(net_rx_data_cb callback);


/**********************************************************************************************************
* Description: 	set powersave flag
* Arguments  : 	psflag: powersave enable or disable flag
*               alwaysflag: 0: only once, lost when restart;  !0: always
* Returns    : 	None
* 
**********************************************************************************************************/
void tls_wifi_set_psflag(bool enable, bool alwaysflag);

/**********************************************************************************************************
* Description: 	get current powersave flag
* Arguments  : 	
* Returns    : 	
*               0: powersave disable
*			   !0: powersave enable
**********************************************************************************************************/
u32 tls_wifi_get_psflag(void);


#endif /* TLS_WIFI_FUNC_H */
