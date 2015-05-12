/***************************************************************************** 
* 
* File Name : wm_params.h 
* 
* Description: param manager Module 
* 
* Copyright (c) 2014 Winner Micro Electronic Design Co., Ltd. 
* All rights reserved. 
* 
* Author : dave
* 
* Date : 2014-6-12
*****************************************************************************/ 
#ifndef TLS_PARAMS_H
#define TLS_PARAMS_H
#include "wm_type_def.h"

/***************************************************************************************
	ID								|			Data Type                                   
*************************************************************************************** 
TLS_PARAM_ID_SSID      				|		  	struct tls_param_ssid ssid                  
TLS_PARAM_ID_ENCRY      		        	|		  	u8 encry                                    
TLS_PARAM_ID_KEY      		          	|		  	struct tls_param_key key                    
TLS_PARAM_ID_IP      			          	|		  	struct tls_param_ip ipcfg                   
TLS_PARAM_ID_AUTOMODE               		|		  	u8 auto_mode                                
TLS_PARAM_ID_DEFSOCKET              		|		  	N/A                                         
TLS_PARAM_ID_BSSID                  		|			struct tls_param_bssid bssid                
TLS_PARAM_ID_CHANNEL                		|			u8 channel                                  
TLS_PARAM_ID_CHANNEL_EN             	|			u8 channel_enable                           
TLS_PARAM_ID_COUNTRY_REGION         	|			u8 wireless_region               	          
TLS_PARAM_ID_WPROTOCOL              	|			u8 wireless_protocol                        
TLS_PARAM_ID_ADHOC_AUTOCREATE       |			N/A                                         
TLS_PARAM_ID_ROAMING                		|			u8 auto_roam                                
TLS_PARAM_ID_AUTO_RETRY_CNT         	|			N/A                                         
TLS_PARAM_ID_WBGR                   		|			struct tls_param_bgr wbgr                   
TLS_PARAM_ID_USRINTF                		|			N/A                                         
TLS_PARAM_ID_AUTO_TRIGGER_LENGTH   |			N/A                                         
TLS_PARAM_ID_DEBUG_MODE             	|			N/A                                         
TLS_PARAM_ID_HARDVERSION            	|			struct tls_param_hardware_version hardware_v
TLS_PARAM_ID_BRDSSID 	              	|			u8 ssid_broadcast_enable                    
TLS_PARAM_ID_DNSNAME                		|			u8 local_dnsname[32]     	                  
TLS_PARAM_ID_DEVNAME                		|			u8 local_device_name[32]               	    
TLS_PARAM_ID_PSM                    		|			u8 auto_powersave               	          
TLS_PARAM_ID_ORAY_CLIENT            	|			N/A                                         
TLS_PARAM_ID_UPNP                   		|			N/A                                         
TLS_PARAM_ID_UART                   		|			struct tls_param_uart uart_cfg              
TLS_PARAM_ID_WPS                    		|			struct tls_param_wps wps                    
TLS_PARAM_ID_AUTO_TRIGGER_PERIOD   |			N/A                                         
TLS_PARAM_ID_ESCAPE_CHAR            	|			N/A                                         
TLS_PARAM_ID_ESCAPE_PERIOD	        	|			N/A                                         
TLS_PARAM_ID_IO_MODE                		|			N/A                                         
TLS_PARAM_ID_CMD_MODE               	|			N/A                                         
TLS_PARAM_ID_PASSWORD	              	|			u8 PassWord[6]                              
TLS_PARAM_ID_CHANNEL_LIST           	|			u16 channellist                             
TLS_PARAM_ID_WEBS_CONFIG            	|			struct tls_webs_cfg WebsCfg                 
TLS_PARAM_ID_ONESHOT_CFG            	|			u8 oneshotflag                              
TLS_PARAM_ID_SHA1                   		|			N/A                                         
TLS_PARAM_ID_ORIGIN_KEY             		|			N/A                                         
TLS_PARAM_ID_ORIGIN_SSID            	|			N/A                                         
TLS_PARAM_ID_AUTO_RECONNECT         	|			u8 auto_reconnect                           
***************************************************************************************/

#define TLS_PARAM_STATUS_OK      (0)
#define TLS_PARAM_STATUS_EINVALID      (1)
#define TLS_PARAM_STATUS_EMEM      (2)
#define TLS_PARAM_STATUS_EIO      (3)
#define TLS_PARAM_STATUS_EPERM      (4)
#define TLS_PARAM_STATUS_EINVALIDID      (5)

#define TLS_PARAM_INIT_MAGIC  (0xffffffff)
#define TLS_PARAM_MAGIC   (0x4947414d)

#define TLS_PARAM_PARTITION_NUM      (2)

#define TLS_PARAM_ID_ALL      (-1)  
#define TLS_PARAM_ID_SSID      (0)
#define TLS_PARAM_ID_ENCRY      (1)
#define TLS_PARAM_ID_KEY      (2)
#define TLS_PARAM_ID_IP      (3)
#define TLS_PARAM_ID_AUTOMODE      (4)
#define TLS_PARAM_ID_DEFSOCKET      (5)
#define TLS_PARAM_ID_BSSID      (6)
#define TLS_PARAM_ID_CHANNEL      (7)
#define TLS_PARAM_ID_CHANNEL_EN      (8)
#define TLS_PARAM_ID_COUNTRY_REGION      (9)
#define TLS_PARAM_ID_WPROTOCOL      (10)
#define TLS_PARAM_ID_ADHOC_AUTOCREATE      (11)
#define TLS_PARAM_ID_ROAMING      (12)
#define TLS_PARAM_ID_AUTO_RETRY_CNT      (13)
#define TLS_PARAM_ID_WBGR      (14)
#define TLS_PARAM_ID_USRINTF      (15)
#define TLS_PARAM_ID_AUTO_TRIGGER_LENGTH      (16)
#define TLS_PARAM_ID_DEBUG_MODE      (17)
#define TLS_PARAM_ID_HARDVERSION      (18)
#define TLS_PARAM_ID_BRDSSID      (19)
#define TLS_PARAM_ID_DNSNAME      (20)
#define TLS_PARAM_ID_DEVNAME      (21)
#define TLS_PARAM_ID_PSM      (22)
#define TLS_PARAM_ID_ORAY_CLIENT      (23)
#define TLS_PARAM_ID_UPNP      (24)
#define TLS_PARAM_ID_UART      (26)
#define TLS_PARAM_ID_WPS      (27)
#define TLS_PARAM_ID_AUTO_TRIGGER_PERIOD  (28)
#define TLS_PARAM_ID_ESCAPE_CHAR    (29)
#define TLS_PARAM_ID_ESCAPE_PERIOD	 (30)
#define TLS_PARAM_ID_IO_MODE     (31)
#define TLS_PARAM_ID_CMD_MODE     (32)
#define TLS_PARAM_ID_PASSWORD	  (33)
#define TLS_PARAM_ID_CHANNEL_LIST (34)
#define TLS_PARAM_ID_WEBS_CONFIG   (35)
#define TLS_PARAM_ID_ONESHOT_CFG   (36)
#define TLS_PARAM_ID_SHA1   (37)
#define TLS_PARAM_ID_ORIGIN_KEY	(38)
#define TLS_PARAM_ID_ORIGIN_SSID	(39)
#define TLS_PARAM_ID_AUTO_RECONNECT	(40)
#define TLS_PARAM_ID_QUICK_CONNECT	(41)

#define TLS_PARAM_ID_MAX      (42)

#define TLS_PARAM_PHY_11BG_MIXED      (0)
#define TLS_PARAM_PHY_11B      (1)

#define TLS_PARAM_TX_RATEIDX_1M          (0)
#define TLS_PARAM_TX_RATEIDX_2M          (1)
#define TLS_PARAM_TX_RATEIDX_5_5M        (2)  
#define TLS_PARAM_TX_RATEIDX_11M         (3)
#define TLS_PARAM_TX_RATEIDX_6M          (4)
#define TLS_PARAM_TX_RATEIDX_9M          (5)
#define TLS_PARAM_TX_RATEIDX_12M         (6)
#define TLS_PARAM_TX_RATEIDX_18M         (7)
#define TLS_PARAM_TX_RATEIDX_24M         (8)
#define TLS_PARAM_TX_RATEIDX_36M         (9)
#define TLS_PARAM_TX_RATEIDX_48M         (10)
#define TLS_PARAM_TX_RATEIDX_54M         (11)
#define TLS_PARAM_TX_RATEIDX_MCS0         (12)
#define TLS_PARAM_TX_RATEIDX_MCS1         (13)
#define TLS_PARAM_TX_RATEIDX_MCS2         (14)
#define TLS_PARAM_TX_RATEIDX_MCS3         (15)
#define TLS_PARAM_TX_RATEIDX_MCS4         (16)
#define TLS_PARAM_TX_RATEIDX_MCS5         (17)
#define TLS_PARAM_TX_RATEIDX_MCS6         (18)
#define TLS_PARAM_TX_RATEIDX_MCS7         (19)
#define TLS_PARAM_TX_RATEIDX_MCS8         (20)
#define TLS_PARAM_TX_RATEIDX_MCS9         (21)
#define TLS_PARAM_TX_RATEIDX_MCS10         (22)
#define TLS_PARAM_TX_RATEIDX_MCS11         (23)
#define TLS_PARAM_TX_RATEIDX_MCS12         (24)
#define TLS_PARAM_TX_RATEIDX_MCS13         (25)
#define TLS_PARAM_TX_RATEIDX_MCS14         (26)
#define TLS_PARAM_TX_RATEIDX_MCS15         (27)
#define TLS_PARAM_TX_RATEIDX_MCS32         (28)



#define TLS_PARAM_SSIDBRD_DISABLE      (0)
#define TLS_PARAM_SSIDBRD_ENABLE      (1)

#define TLS_PARAM_ROAM_DISABLE      (0)
#define TLS_PARAM_ROAM_ENABLE      (1)

#define TLS_PARAM_PSM_DISABLE      (0)
#define TLS_PARAM_PSM_ENABLE      (1)

#define TLS_PARAM_DDNS_DISABLE      (0)
#define TLS_PARAM_DDNS_ENABLE      (1)

#define TLS_PARAM_AUTO_CREATE_ADHOC_DISABLE      (0)
#define TLS_PARAM_AUTO_CREATE_ADHOC_ENABLE      (1)

#define TLS_PARAM_WPS_DISABLE      (0)
#define TLS_PARAM_WPS_ENABLE      (1)
#define TLS_PARAM_WPS_MODE_PIN      (0)
#define TLS_PARAM_WPS_MODE_PBC      (1)
#define TLS_PARAM_WPS_FLAG_NEGOTIATED      (1 << 0)

#define TLS_PARAM_UPNP_DISABLE      (0)
#define TLS_PARAM_UPNP_ENABLE      (1)

#define TLS_PARAM_IEEE80211_INFRA      (0)
#define TLS_PARAM_IEEE80211_ADHOC      (1)
#define TLS_PARAM_IEEE80211_SOFTAP      (2)

#define TLS_PARAM_MANUAL_MODE      (0)
#define TLS_PARAM_AUTO_MODE      (1)

#define TLS_PARAM_ALWAYS_ONLINE_ENABLE      (1)
#define TLS_PARAM_ALWAYS_ONLINE_DISABLE      (0)

#define TLS_PARAM_DHCP_ENABLE      (1)
#define TLS_PARAM_DHCP_DISABLE      (0)

#define TLS_PARAM_REGION_0_BG_BAND      (0) /* 1-11 */
#define TLS_PARAM_REGION_1_BG_BAND      (1) /* 1-13 */
#define TLS_PARAM_REGION_2_BG_BAND      (2) /* 10-11 */
#define TLS_PARAM_REGION_3_BG_BAND      (3) /* 10-13 */
#define TLS_PARAM_REGION_4_BG_BAND      (4) /* 14 */
#define TLS_PARAM_REGION_5_BG_BAND      (5) /* 1-14 */
#define TLS_PARAM_REGION_6_BG_BAND      (6) /* 3-9 */
#define TLS_PARAM_REGION_7_BG_BAND      (7) /* 5-13 */
#define TLS_PARAM_REGION_MAXIMUM_BG_BAND      TLS_PARAM_REGION_7_BG_BAND

#define TLS_PARAM_ENCRY_OPEN      (0)
#define TLS_PARAM_ENCRY_WEP64      (1)
#define TLS_PARAM_ENCRY_WEP128      (2)
#define TLS_PARAM_ENCRY_WPA_PSK_TKIP      (3)
#define TLS_PARAM_ENCRY_WPA_PSK_AES      (4)
#define TLS_PARAM_ENCRY_WPA2_PSK_TKIP      (5)
#define TLS_PARAM_ENCRY_WPA2_PSK_AES      (6)

#define TLS_PARAM_USR_INTF_LUART      (0)
#define TLS_PARAM_USR_INTF_HUART      (1)
#define TLS_PARAM_USR_INTF_HSPI      (2)
#define TLS_PARAM_USR_INTF_HSDIO	 (3)

#define TLS_PARAM_UART_BAUDRATE_B600      (600)	
#define TLS_PARAM_UART_BAUDRATE_B1200      (1200)
#define TLS_PARAM_UART_BAUDRATE_B1800      (1800)	
#define TLS_PARAM_UART_BAUDRATE_B2400      (2400)	
#define TLS_PARAM_UART_BAUDRATE_B4800      (4800)	
#define TLS_PARAM_UART_BAUDRATE_B9600      (9600)	
#define TLS_PARAM_UART_BAUDRATE_B19200      (19200)	
#define TLS_PARAM_UART_BAUDRATE_B38400      (38400)	
#define TLS_PARAM_UART_BAUDRATE_B57600      (57600) 
#define TLS_PARAM_UART_BAUDRATE_B115200      (115200)
#define TLS_PARAM_HUART_BAUDRATE_B230400      (230400) 
#define TLS_PARAM_HUART_BAUDRATE_B460800      (460800) 
#define TLS_PARAM_HUART_BAUDRATE_B921600      (921600)	
#define TLS_PARAM_HUART_BAUDRATE_B1000000      (1000000)
#define TLS_PARAM_HUART_BAUDRATE_B1250000      (1250000)
#define TLS_PARAM_HUART_BAUDRATE_B1500000      (1500000)
#define TLS_PARAM_HUART_BAUDRATE_B2000000      (2000000)

#define TLS_PARAM_UART_PARITY_NONE      (0)
#define TLS_PARAM_UART_PARITY_EVEN      (1)
#define TLS_PARAM_UART_PARITY_ODD      (2)
#define TLS_PARAM_UART_PARITY_MARK      (3)
#define TLS_PARAM_UART_PARITY_SPACE      (4)

#define TLS_PARAM_UART_STOPBITS_1BITS      (0)
#define TLS_PARAM_UART_STOPBITS_2BITS      (1)

#define TLS_PARAM_UART_FLOW_DISABLE      (0)
#define TLS_PARAM_UART_FLOW_ENABLE      (1)
#define TLS_PARAM_HSPI_MAX_SCLK      (33000000)

#define TLS_PARAM_SOCKET_TCP      (0)
#define TLS_PARAM_SOCKET_UDP      (1)
#define TLS_PARAM_SOCKET_CLIENT      (0)
#define TLS_PARAM_SOCKET_SERVER      (1)
#define TLS_PARAM_SOCKET_DEFAULT_PORT      (60000)

struct tls_param_hardware_version {
	u8 mark; 
	u8 main; 
	u8 sub; 
	u8 material; 
	u8 year; 
	u8 week; 
	u8 res[2];
};

struct tls_param_bssid {
	u8 bssid_enable;
	u8 res;
	u8 bssid[6];
};


struct tls_param_ssid {
	u8 ssid[32];
	u32 ssid_len;
};

struct tls_param_key {
	u8 psk[64];
	u8 key_length; 
	u8 key_index;
	u8 key_format;
	u8 res;
};

struct tls_param_original_key {
	u8 psk[64];
	u32 key_length; 
};

struct tls_param_sha1{
	u8 psk_set;
	u8 psk[32];
};

struct tls_param_bgr {
	u8 bg;
	u8 max_rate;
	u8 res[2];
};

struct tls_param_ip {
	u8 dhcp_enable;
	u8 res[3];
	u8 ip[4];
	u8 netmask[4];
	u8 gateway[4];
	u8 dns1[4];
	u8 dns2[4];
};

struct tls_param_uart {
	u32 baudrate;
	u8 stop_bits;
	u8 parity;
	u8 flow;
	u8 charsize;
};

struct tls_param_socket {
	u8 protocol;
	u8 client_or_server;
	u16 port_num;
	u8 host[32];
};

struct tls_param_wps {
	u8 wps_enable;
	u8 mode;
	u8 flag;
	u8 res;
	u8 pin[8];
};

struct tls_param_oray_client {
	u32 oray_client_enable;
	u8 oray_client_user[32];
	u8 oray_client_pass[32];
};

struct tls_webs_cfg {
	u8	AutoRun;
	u8	Reserved;
	u16 PortNum;
};


typedef struct tls_user_param{
	
	u8 wireless_protocol;
	u8 auto_mode;
	u8 dhcp_enable;
	u8 user_port_mode;
	
	u8 ip[4];
	u8 netmask[4];
	u8 gateway[4];
	u8 dns[4];

	u32 baudrate;
	
	u8 socket_protocol;
	u8 socket_client_or_server;
	u16 socket_port_num;
	u8 socket_host[32];
	u8 auto_powersave;
	u8 PassWord[6];
}ST_Wm_User_Param;


/**********************************************************************************************************
* Description: 	This function is used to initial system param.
*
* Arguments  : 	sys_param		is the system param addr
*				
*
* Returns    :		TLS_PARAM_STATUS_OK		init success	
*				TLS_PARAM_STATUS_EMEM		memory error		
*				TLS_PARAM_STATUS_EIO		io error
*				TLS_PARAM_STATUS_EPERM		
**********************************************************************************************************/
int tls_param_init(void);

/**********************************************************************************************************
* Description: 	This function is used to load the system default parameters.
*
* Arguments  : 	param		is the param point
*				

* Returns    :
*
* Notes	:		This function read user defined parameters first, if wrong, all the parameters restore factory settings.
**********************************************************************************************************/
void tls_param_load_factory_default(void);

/**********************************************************************************************************
* Description: 	This function is used to set system parameter.
*
* Arguments  : 	id		param id,from TLS_PARAM_ID_SSID to (TLS_PARAM_ID_MAX - 1)
*				argc		store parameters
*				to_flash	whether the parameter is written to flash,1 write to flash

* Returns    :		TLS_PARAM_STATUS_OK		set success
*				TLS_PARAM_STATUS_EINVALID	invalid param
**********************************************************************************************************/
int tls_param_set(int id, void *argv, bool to_flash);

/**********************************************************************************************************
* Description: 	This function is used to get system parameter.
*
* Arguments  : 	id		param id,from TLS_PARAM_ID_SSID to (TLS_PARAM_ID_MAX - 1)
*				argc		store parameters
*				from_flash	whether the parameter is readed from flash,1 read from flash
*
* Returns    :		TLS_PARAM_STATUS_OK	success
*				TLS_PARAM_STATUS_EINVALID	invalid param
**********************************************************************************************************/
int tls_param_get(int id, void *argv, bool from_flash);

/**********************************************************************************************************
* Description: 	This function is used to write parameter to flash.
*
* Arguments  : 	id		param id,from TLS_PARAM_ID_ALL to (TLS_PARAM_ID_MAX - 1)
*
* Returns    :		TLS_PARAM_STATUS_OK	success
*				TLS_PARAM_STATUS_EINVALID	invalid param
*				TLS_PARAM_STATUS_EIO		error
**********************************************************************************************************/
int tls_param_to_flash(int id);

/**********************************************************************************************************
* Description: 	This function is used to recovery the parameters from the backup area to the parameter area, 
*				and read to memory.
*
* Arguments  : 	
*
* Returns    :		
**********************************************************************************************************/
void tls_restore_param_from_backup(void);

/**********************************************************************************************************
* Description: 	This function is used to load default parametes to memory. 
*
* Arguments  : 	
*
* Returns    :
*
* Notes		:	This function read user defined parameters first, if wrong, all the parameters restore factory settings.				
**********************************************************************************************************/
int tls_param_to_default(void);

/**********************************************************************************************************
* Description: 	This function is used to write user parameters to the flash.
*
* Arguments  : 	
*
* Returns    :		TLS_PARAM_STATUS_OK
**********************************************************************************************************/
int tls_param_save_user_default(void);

/**********************************************************************************************************
* Description: 	This function is used to modify user default parameters,then write to flash.
*
* Arguments  : 	
*
* Returns    :		TLS_PARAM_STATUS_OK
**********************************************************************************************************/
int tls_param_save_user(struct tls_user_param *user_param);

/**********************************************************************************************************
* Description: 	This function is used to get updp mode.(updp:update user default parameters)
*
* Arguments  : 	
*
* Returns    :		updp mode
**********************************************************************************************************/
u8 tls_param_get_updp_mode(void);

/**********************************************************************************************************
* Description: 	This function is used to set updp mode.
*
* Arguments  : 	
*
* Returns    :
**********************************************************************************************************/
void tls_param_set_updp_mode(u8 mode);

#endif /* WM_PARAM_H */

