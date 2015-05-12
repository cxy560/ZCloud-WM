
#ifndef TLS_SYS_H
#define TLS_SYS_H
#include "wm_type_def.h"
#include "wm_params.h"

struct tls_one_shot_cfg{
	u8	ssid[32];
	u8	key[65];
	u8	key_len;
	u16 channel;	
};

#define SYS_MSG_NET_UP            1
#define SYS_MSG_NET_DOWN          2
#define SYS_MSG_CONNECT_FAILED	  3	
#define SYS_MSG_AUTO_MODE_RUN     4




void tls_os_timer_init(void);
int tls_sys_init(void);
void tls_auto_reconnect(void);


#endif /* end of TLS_SYS_H */
