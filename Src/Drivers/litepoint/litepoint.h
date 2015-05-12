#ifndef __LITEPOINT_H
#define __LITEPOINT_H

typedef struct _ltpt_rx_info
{
	int bprocess;
	int cnt_total;
	int cnt_good;
	int cnt_bad;
	unsigned char valid;
	unsigned char snr;
	unsigned char rcpi;
	unsigned char rate;
}ltpt_rx_info;

typedef struct _ltpt_tx_info
{
	int bprocess;
	int cnt_total;
	/* input parameters */
	int channel;
	int packetcount;	
	int psdulen;    
	int txgain;      
	int datarate;   
}ltpt_tx_info;


enum Rate 
{
	S2M = 0x0000,
	S5M5 = 0x0001,
	S11M = 0x0002,
	L1M = 0x0003,
	L2M = 0x0004,
	L5M5 = 0x0005,
	L11M = 0x0006,
	R06M = 0x0100,
	R09M = 0x0101,
	R12M = 0x0102,
	R18M = 0x0103,
 	R24M = 0x0104,
	R36M = 0x0105,
	R48M = 0x0106,
	R54M = 0x0107,
	MCS0 = 0x0200,
	MCS1 = 0x0201,
	MCS2 = 0x0202,
	MCS3 = 0x0203,
	MCS4 = 0x0204,
	MCS5 = 0x0205,
	MCS6 = 0x0206,
	MCS7 = 0x0207,
	MCS8 = 0x0208,
	MCS9 = 0x0209,
	MCS10 = 0x0210,
	MCS11 = 0x0211,
	MCS12 = 0x0212,
	MCS13 = 0x0213,
	MCS14 = 0x0214,
	MCS15 = 0x0215,
	MCS32 = 0x0232,
};

extern int g_ltpt_testmode;
extern ltpt_rx_info g_ltpt_rxinfo;
extern ltpt_tx_info g_ltpt_txinfo;
extern unsigned char hed_rf65_txgainmap[32];
extern const unsigned int hed_bbp_value[];


void tls_tx_send_litepoint(void);

#endif
