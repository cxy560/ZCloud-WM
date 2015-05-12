/**************************************************************************
 * File Name                    : wm_wifi_oneshot.h
 * Author                       : WinnerMicro
 * Version                      :
 * Date                         : 05/30/2014
 * Description                  :
 *
 * Copyright (C) 2014 Beijing Winner Micro Electronics Co., Ltd.
 * All rights reserved.
 *
 ***************************************************************************/
#ifndef WM_WIFI_ONESHOT_H
#define WM_WIFI_ONESHOT_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wm_type_def.h>
#if (GCC_COMPILE==1)
#include "wm_ieee80211_gcc.h"
#else
#include <wm_ieee80211.h>
#endif
/*UDP ONE SHOT*/
#define CONFIG_UDP_ONE_SHOT 1
/*PROBE REQUEST ONE SHOT*/
//#define CONFIG_PROBE_REQUEST_ONE_SHOT
typedef struct WIFI_BSSID_STRU{
	u8 *DataBssid[2];
}WIFI_BSSID;
typedef struct WIFI_SSID_STRU{
	u8 *DataSsid[8];
}WIFI_SSID;
typedef struct WIFI_PWD_STRU{
	u8 *DataPwd[32];
}WIFI_PWD;
typedef struct WIFI_CUST_DATA_STRU{
	u8 *DataCustom[32];
}WIFI_CUST_DATA;
#if CONFIG_PROBE_REQUEST_ONE_SHOT
typedef struct stOneKey{
	s8 header[3];
	s8 len;
	s8 tag_Id;
	u8 seq_total;
	u8 seq_num;
	u8 *data;
	u8 checksum;
}OneKey;
#endif
u8 tls_wifi_dataframe_recv(struct ieee80211_hdr *hdr, u32 data_len);

void tls_wifi_probereq_recv(struct ieee80211_hdr *hdr, u32 data_len);
#endif /*WM_WIFI_ONESHOT_H*/
