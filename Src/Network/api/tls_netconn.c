/**************************************************************************
 * File Name                   : tls_netconn.c
 * Author                       :
 * Version                      :
 * Date                          :
 * Description                 :
 *
 * Copyright (c) 2014 Winner Microelectronics Co., Ltd. 
 * All rights reserved.
 *
 ***************************************************************************/
#include "wm_config.h"

#if TLS_CONFIG_SOCKET_RAW

#include "tls_netconn.h"
#include "wm_debug.h"
#include "wm_regs.h"
#include "lwip/sys.h"
#include "lwip/err.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "lwip/tcpip.h"
#include <string.h>

struct tls_netconn g_net_conn[TLS_MAX_NETCONN_NUM];
struct tls_netconn *p_net_conn[TLS_MAX_NETCONN_NUM];

//static err_t net_tcp_sent_cb(void *arg, struct tcp_pcb *pcb, u16_t len);
static void net_tcp_err_cb(void *arg, err_t err);
static err_t net_tcp_poll_cb(void *arg, struct tcp_pcb *pcb);
//static err_t net_do_writemore(struct tls_netconn *conn);

#define BUF_CIRC_CNT_TO_END(head,tail,size) \
	({int end = (size) - (tail); \
	  int n = ((head) + end) & ((size)-1); \
	  n < end ? n : end;})


u32 current_src_ip = 0;
void tls_net_set_sourceip(u32 ipvalue)
{
	current_src_ip = ipvalue;
}

u32 tls_net_get_sourceip(void)
{
	return 	current_src_ip;
}

static struct tls_netconn *net_alloc_socket(struct tls_netconn *conn, u8 socket_type)
{
    int i;
    u8 offset;
    u32 cpu_sr;
	struct tls_netconn * conn_t;
	
	if(socket_type == TLS_SOCKET_TYPE_CMD)
	{
		if(conn != NULL)
	    {
			offset = (conn->skt_num)* 4;
			for (i = 0; i < TLS_MAX_SOCKET_NUM; i++) {
			    cpu_sr = tls_os_set_critical();
			    if (!g_net_conn[i + offset].used) {
			        g_net_conn[i + offset].used = true;
			        tls_os_release_critical(cpu_sr);
					dl_list_init(&g_net_conn[offset + i].list);
			        return &g_net_conn[offset + i];
			    }
			    tls_os_release_critical(cpu_sr);
			}
	    }
		else
		{
			for (i = 0; i < TLS_MAX_SOCKET_NUM; i++) 
			{
			    cpu_sr = tls_os_set_critical();
			    if (!g_net_conn[i].used) {
			        g_net_conn[i].used       = true; 
			        tls_os_release_critical(cpu_sr);
					dl_list_init(&g_net_conn[i].list);
			        return &g_net_conn[i];
			    }
			    tls_os_release_critical(cpu_sr);
			}
		}
	}
	else
	{
		for (i = 0; i < TLS_MAX_NETCONN_NUM; i++) 
		{
			cpu_sr = tls_os_set_critical();
			if(p_net_conn[i] != NULL)
			{
				tls_os_release_critical(cpu_sr);
				continue;
			}
			conn_t = tls_mem_alloc(sizeof(struct tls_netconn));
			p_net_conn[i] = conn_t;
			tls_os_release_critical(cpu_sr);
			memset(conn_t, 0, sizeof(struct tls_netconn));
			conn_t->used = true;
			conn_t->state = NETCONN_STATE_NONE;
			conn_t->skt_num = TLS_MAX_NETCONN_NUM + i + 1;
			dl_list_init(&conn_t->list);
			TLS_DBGPRT_INFO("net_alloc_socket conn ptr = 0x%x\n", conn_t);
			return conn_t;
		}
	}
	
    return NULL;
}

static void net_free_socket(struct tls_netconn *conn)
{
	int index;
	u32 cpu_sr;
	if(conn == NULL || conn->used == false)
		return;
	TLS_DBGPRT_INFO("conn ptr = 0x%x\n", conn);
	conn->used = false;
	if(conn->client  && conn->list.prev != NULL && conn->list.prev != &conn->list)
	{
		TLS_DBGPRT_INFO("del from list.\n");
		cpu_sr = tls_os_set_critical();
	       dl_list_del(&conn->list);
		tls_os_release_critical(cpu_sr);			
	}
	if(conn->skt_num > TLS_MAX_NETCONN_NUM)
	{
		index = conn->skt_num - TLS_MAX_NETCONN_NUM - 1;
		tls_mem_free(conn);
		cpu_sr = tls_os_set_critical();
		conn = NULL;
		p_net_conn[index] = NULL;
		tls_os_release_critical(cpu_sr);
	}
}

static void net_send_event_to_hostif(struct tls_netconn *conn,
        int event)
{
    struct tls_socket_desc *skt_desc = conn->skd;
TLS_DBGPRT_INFO("skt_desc->state_changed: 0x%x, event=%d\n", skt_desc->state_changed, event);
    if(skt_desc->state_changed)
    {
        skt_desc->state_changed(conn->skt_num, event, conn->state);
    }
}

static void net_tcp_close_connect(struct tls_netconn *conn)
{
    err_t err;

    /* Set back some callback pointers */
    tcp_arg(conn->pcb.tcp, NULL);
    if (conn->pcb.tcp->state == LISTEN) {
        tcp_accept(conn->pcb.tcp, NULL);
    } else {
        /* some callbacks have to be reset if tcp_close is not successful */
        tcp_recv(conn->pcb.tcp, NULL);
        tcp_accept(conn->pcb.tcp, NULL);
        tcp_sent(conn->pcb.tcp, NULL);
        tcp_poll(conn->pcb.tcp, NULL, 4);
        tcp_err(conn->pcb.tcp, NULL);
    }
    err = tcp_close(conn->pcb.tcp);
    if (err)
        err = tcp_shutdown(conn->pcb.tcp, 1, 1);
    if (err == ERR_OK) {
        /* Closing succeeded */
        TLS_DBGPRT_INFO("tcp %d closed\n", conn->skt_num);
        conn->state = NETCONN_STATE_NONE;
        net_free_socket(conn);
        /* Set back some callback pointers as conn is going away */
        conn->pcb.tcp = NULL;
        /* Trigger select() in socket layer. Make sure everybody notices activity
           on the connection, error first! */
    } else {
        /* Closing failed, restore some of the callbacks */
        /* Closing of listen pcb will never fail! */
        LWIP_ASSERT("Closing a listen pcb may not fail!", (conn->pcb.tcp->state != LISTEN));
        //tcp_sent(conn->pcb.tcp, net_tcp_sent_cb);
        tcp_poll(conn->pcb.tcp, net_tcp_poll_cb, 4);
        tcp_err(conn->pcb.tcp, net_tcp_err_cb);
        tcp_arg(conn->pcb.tcp, conn);
        /* don't restore recv callback: we don't want to receive any more data */
    }
}

static err_t net_tcp_poll_cb(void *arg, struct tcp_pcb *pcb)
{
	err_t err_ret = ERR_OK;
    struct tls_netconn *conn = (struct tls_netconn *)arg;
    struct tls_netconn *server_conn = NULL; 
    //TLS_DBGPRT_INFO("net_tcp_poll_cb start\n");

    if (conn == NULL || pcb == NULL) {
        return ERR_CLSD;
    }

    if (conn->write_state && (conn->state == NETCONN_STATE_CONNECTED)) {
        //net_do_writemore(conn);
    } else if (conn->state == NETCONN_STATE_CLOSED) {
        net_tcp_close_connect(conn); 
    } else {
        if ((pcb->state == CLOSE_WAIT) || (pcb->state == CLOSED)) {
            if (conn->pcb.tcp != NULL) {
                tcp_arg(conn->pcb.tcp, NULL);
                tcp_accept(conn->pcb.tcp, NULL);
                tcp_recv(conn->pcb.tcp, NULL);      
                tcp_sent(conn->pcb.tcp, NULL);
                tcp_poll(conn->pcb.tcp, NULL, 4);
                tcp_err(conn->pcb.tcp, NULL);
                tcp_close(conn->pcb.tcp);       
                conn->state = NETCONN_STATE_NONE;
                net_send_event_to_hostif(conn, NET_EVENT_TCP_DISCONNECT);
                conn->pcb.tcp = NULL;
                net_free_socket(conn);
            }
        }
    }
	if(conn->client && conn->idle_time > 0)
	{
		TLS_DBGPRT_INFO("conn->skt_num=%d, conn->client=%d\n", conn->skt_num, conn->client);
		server_conn = dl_list_first(&conn->list, struct tls_netconn, list);
		if(server_conn)
		{
			--conn->idle_time;
			if(conn->idle_time == 0)
			{
				tcp_abort(pcb);
				//tls_socket_close(conn->skt_num);
				TLS_DBGPRT_INFO("update conn->idle_time %d\n", conn->idle_time);
				return ERR_ABRT;
			}
		}
	}
	if(((conn)->skd) != NULL && (((conn)->skd)->pollf) != NULL)
	{
		err_ret = conn->skd->pollf(conn->skt_num);
		if(err_ret == ERR_ABRT)
			tcp_abort(pcb);
		return err_ret;
	}

    return ERR_OK;

}

static void net_tcp_err_cb(void *arg, err_t err)
{
    struct tls_netconn *conn = (struct tls_netconn *)arg;
    struct tcp_pcb *pcb = conn->pcb.tcp;  
    u8 event = NET_EVENT_TCP_CONNECT_FAILED;
    TLS_DBGPRT_INFO("tcp err = %d\n", err);

    if (pcb) {
        tcp_arg(pcb, NULL);
        tcp_sent(pcb, NULL);
        tcp_recv(pcb, NULL);
        tcp_err(pcb, NULL);
        //tcp_close(pcb);
	
	if (!conn->client) {
	    tcp_accept(pcb, NULL);
	}
	if(conn->state != NETCONN_STATE_NONE)
	{
           conn->state = NETCONN_STATE_NONE;
           event = NET_EVENT_TCP_DISCONNECT;
	}
        net_free_socket(conn);
	net_send_event_to_hostif(conn, event);
	if(conn->skd->errf != NULL)
	{
		conn->skd->errf(conn->skt_num, err);
	}
    }
    conn->pcb.tcp = NULL;
}

#if (RAW_SOCKET_USE_CUSTOM_PBUF)
static struct raw_sk_pbuf_custom*
raw_sk_alloc_pbuf_custom(void)
{
  return (struct raw_sk_pbuf_custom*)mem_malloc(sizeof(struct raw_sk_pbuf_custom));
}


static void
raw_sk_free_pbuf_custom(struct raw_sk_pbuf_custom* p)
{
	if(p != NULL)
	{
		mem_free(p);
		p = NULL;
	}
}

static void
raw_sk_free_pbuf_custom_fn(struct pbuf *p)
{
	struct raw_sk_pbuf_custom *pcr = (struct raw_sk_pbuf_custom*)p;

	if(p != NULL)
	{
	//	printf("\nraw_sk_free_pbuf_custom_fn,len=%d\n",p->tot_len);
		tcp_recved((struct tcp_pcb *)pcr->param, p->tot_len);

		if (pcr->original != NULL) {
	//		printf("\ngo to pbuf_free,original pbuf=%x\n",pcr->original);
			pbuf_free(pcr->original);
		}
		raw_sk_free_pbuf_custom(pcr);
	}
}
	
#endif


/**
 * Receive callback function for TCP connect.
 */
static err_t net_tcp_recv_cb(void *arg, 
        struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
    struct tls_netconn *conn;
    struct tls_netconn *server_conn = NULL;
    err_t err_ret = ERR_OK;
	u16 datalen = 0;
#if (RAW_SOCKET_USE_CUSTOM_PBUF)
	struct raw_sk_pbuf_custom *pcr = NULL;
	struct pbuf *newpbuf = NULL;
#endif

 //   TLS_DBGPRT_INFO("net_tcp_recv_cb err=%d\n", err);

    LWIP_UNUSED_ARG(pcb);
    LWIP_ASSERT("must have a pcb argument", pcb != NULL);
    LWIP_ASSERT("must have an argument", arg != NULL);
    conn = (struct tls_netconn *)arg;
    LWIP_ASSERT("tcp recv for wrong pcb!", conn->pcb.tcp == pcb);

    if (err) { 
        /* tcp is disconnect */
        TLS_DBGPRT_INFO("err code = %d\n", err);
		err_ret = err;
#if 0
        if (conn->pcb.tcp != NULL) {
            tcp_arg(conn->pcb.tcp, NULL);
            tcp_accept(conn->pcb.tcp, NULL);
            tcp_recv(conn->pcb.tcp, NULL);      
            tcp_sent(conn->pcb.tcp, NULL);
            tcp_poll(conn->pcb.tcp, NULL, 4);
            tcp_err(conn->pcb.tcp, NULL);
            tcp_close(conn->pcb.tcp);     
            conn->state = NETCONN_STATE_NONE;         
            net_send_event_to_hostif(conn, NET_EVENT_TCP_DISCONNECT);
            conn->pcb.tcp = NULL;
            net_free_socket(conn);
        }
#endif
	if(conn->skd != NULL && conn->skd->recvf != NULL)
	{
		TLS_DBGPRT_INFO("conn->skd->recvf to call.");
		err_ret = conn->skd->recvf(conn->skt_num, p, err);
		if(err_ret == ERR_ABRT)
			tcp_abort(pcb);
	}
        if (p)
            pbuf_free(p);
        return err_ret;
    }

    if (pcb == NULL ||conn == NULL) {//p == NULL ||  
        TLS_DBGPRT_INFO("pcb = 0x%x, p = 0x%x\n", pcb ,p);
        return ERR_VAL;
    }

    if(p == NULL)
    {
        TLS_DBGPRT_ERR("received 0\n");
        tcp_recved(pcb, 0);
        tcp_abort(pcb);
        return ERR_ABRT;
    }
#if (RAW_SOCKET_USE_CUSTOM_PBUF)
	pcr = raw_sk_alloc_pbuf_custom();
	if(pcr == NULL)
	{
		return ERR_MEM;
	}
	newpbuf = pbuf_alloced_custom(PBUF_RAW, p->tot_len, PBUF_REF, &pcr->pc, p->payload, p->tot_len);
      if (newpbuf == NULL) {
		raw_sk_free_pbuf_custom(pcr);
		return ERR_MEM;
      }
	pcr->original = p;
	pcr->param = pcb;
	pcr->pc.custom_free_function = raw_sk_free_pbuf_custom_fn;
#else
    tcp_recved(pcb, p->tot_len);
#endif
	
	if(conn->client && conn->idle_time > 0)
	{
		TLS_DBGPRT_INFO("conn->skt_num=%d, conn->client=%d\n", conn->skt_num, conn->client);
		server_conn = dl_list_first(&conn->list, struct tls_netconn, list);
		TLS_DBGPRT_INFO("server_conn=%p\n", server_conn);
		if(server_conn)
		{
			conn->idle_time = server_conn->idle_time;
			TLS_DBGPRT_INFO("update conn->idle_time %d\n", conn->idle_time);
		}
	}
    if(conn->skd->recvf != NULL)
    {
    	tls_net_set_sourceip(pcb->remote_ip.addr);
		datalen = p->tot_len;
#if (RAW_SOCKET_USE_CUSTOM_PBUF)
	conn->skd->recvf(conn->skt_num, newpbuf, ERR_OK);
#else
	conn->skd->recvf(conn->skt_num, p, ERR_OK);
#endif
		if (conn->skd->recvwithipf != NULL){
			conn->skd->recvwithipf(conn->skt_num, datalen, (u8 *)(&(pcb->remote_ip.addr)), pcb->remote_port, ERR_OK);
		}
    }
    else
    {
#if (RAW_SOCKET_USE_CUSTOM_PBUF)
	raw_sk_free_pbuf_custom(pcr);
#endif
        if (p)
            pbuf_free(p);
    }

    return err_ret;
}

#if 0
static err_t net_tcp_sent_cb(void *arg, struct tcp_pcb *pcb, u16_t len)
{
    struct tls_netconn *conn = (struct tls_netconn *)arg;    

    //TLS_DBGPRT_INFO("s\n");
    if (conn->write_state && (conn->state == NETCONN_STATE_CONNECTED)) {
        net_do_writemore(conn);
    } else if (conn->state == NETCONN_STATE_CLOSED) {
        net_tcp_close_connect(conn);
    }

    return ERR_OK;
}
#endif

/**
 * tcp connnect callback
 */
static err_t net_tcp_connect_cb(void *arg, struct tcp_pcb *pcb, err_t err)
{
    struct tls_netconn *conn;
	err_t  err_ret = ERR_OK;


    conn = (struct tls_netconn *)arg;
	pcb = conn->pcb.tcp;
    if ((conn->proto == TLS_NETCONN_TCP) && (err == ERR_OK)) {
	TLS_DBGPRT_INFO("net_tcp_connect_cb =====> state : %d\n", pcb->state);
        
       conn->state = NETCONN_STATE_CONNECTED;
      	net_send_event_to_hostif(conn, NET_EVENT_TCP_CONNECTED);
    } else {
        TLS_DBGPRT_INFO("the err is =%d\n", err);
    } 
	if(conn->skd != NULL && conn->skd->connf != NULL)
	{
		err_ret = conn->skd->connf(conn->skt_num, err);
		if(err_ret == ERR_ABRT)
			tcp_abort(pcb);
		return err_ret;
	}

    return err;
}

/**
 * Accept callback function for TCP netconns.
 */
static err_t net_tcp_accept_cb(void *arg, 
        struct tcp_pcb *newpcb, err_t err)
{
    err_t err_ret = ERR_OK;
    struct tls_netconn *conn = (struct tls_netconn *)arg;
    struct tls_netconn *newconn;
    struct tcp_pcb *pcb;
    u32 cpu_sr;

    //TLS_DBGPRT_INFO("=====>\n");
#if 0
    if (conn->tcp_connect_cnt >= TLS_MAX_SOCKET_NUM) {
        return ERR_ABRT;
    }
#endif

    newconn = net_alloc_socket(conn, TLS_SOCKET_TYPE_CMD);

    if (!newconn) {
        /* connection aborted */
        /* not happen */
		tcp_abort(newpcb);
        return ERR_ABRT;
    }

    //conn->tcp_connect_cnt++;

    newconn->used = true;
    newconn->pcb.tcp = newpcb;
    newconn->client = true;
    newconn->idle_time = conn->idle_time;
    newconn->proto = TLS_NETCONN_TCP;
    newconn->state = NETCONN_STATE_CONNECTED;
    pcb = newconn->pcb.tcp;
    ip_set_option(newconn->pcb.tcp, SOF_KEEPALIVE);
    newconn->skd = conn->skd;
    tcp_arg(pcb, newconn);
    tcp_recv(pcb, net_tcp_recv_cb);
    //tcp_sent(pcb, net_tcp_sent_cb);
    tcp_poll(pcb, net_tcp_poll_cb, 2);
    tcp_err(pcb, net_tcp_err_cb);
	cpu_sr = tls_os_set_critical();
	dl_list_add_tail(&conn->list, &newconn->list);
	tls_os_release_critical(cpu_sr);
    newconn->port = host_to_le16(pcb->remote_port);
    ip_addr_set(&newconn->addr, &pcb->remote_ip);
    net_send_event_to_hostif(newconn, NET_EVENT_TCP_JOINED);
    if(conn->skd->acceptf != NULL)
	{
		err_ret =conn->skd->acceptf(newconn->skt_num, err);
		if(err_ret == ERR_ABRT)
			tcp_abort(pcb);
	} 
    //conn->state = NETCONN_STATE_CONNECTED;

    return err_ret;
}

/**
 * Receive callback function for UDP netconns.
 */
static void net_udp_recv_cb(void *arg, struct udp_pcb *pcb, 
        struct pbuf *p, ip_addr_t *srcaddr, u16_t port)
{
    struct tls_netconn *conn;
	u32 datalen = 0;

    //TLS_DBGPRT_INFO("=====>\n");
    LWIP_UNUSED_ARG(pcb); /* only used for asserts... */
    LWIP_ASSERT("recv_udp must have a pcb argument", pcb != NULL);
    LWIP_ASSERT("recv_udp must have an argument", arg != NULL);
    conn = (struct tls_netconn *)arg;
    LWIP_ASSERT("recv_udp: recv for wrong pcb!", conn->pcb.udp == pcb);

    if (conn == NULL || pcb == NULL) {
	TLS_DBGPRT_INFO("if (conn == NULL || pcb == NULL) \n");
        if (p != NULL)
            pbuf_free(p);
        return;
    }

    if(conn->skd->recvf != NULL)
    {
    	datalen = p->tot_len;
        conn->skd->recvf(conn->skt_num, p, ERR_OK);

		tls_net_set_sourceip(srcaddr->addr);
		if (conn->skd->recvwithipf != NULL){
			conn->skd->recvwithipf(conn->skt_num, datalen, (u8 *)(&(srcaddr->addr)), port, ERR_OK);
		}
    }
    else
    {
        if (p)
            pbuf_free(p);
    }

    return; 
}


/**
 * Start create TCP connection
 */
static err_t net_tcp_start(struct tls_netconn *conn)
{
    struct tcp_pcb *pcb;
    err_t  err;
    u16 localportnum = conn->localport;

	
    //TLS_DBGPRT_INFO("=====>\n");
    conn->pcb.tcp = tcp_new();
    pcb = conn->pcb.tcp;

    if (pcb == NULL) {
        TLS_DBGPRT_INFO("could not allocate tcp pcb\n");
        return ERR_VAL;
    }

    tcp_arg(pcb, conn);

    if (conn->client) {
        TLS_DBGPRT_INFO("pcb = 0x%x, conn->addr = 0x%x, port = %d\n",
                pcb, conn->addr.addr, conn->port);
        tcp_err(pcb, net_tcp_err_cb);
        tcp_recv(pcb, net_tcp_recv_cb);
        //tcp_nagle_disable(pcb);
		if (pcb->recv != NULL) {
			TLS_DBGPRT_INFO("pcb->recv != NULL\n");
		}
	    //tcp_sent(pcb, net_tcp_sent_cb);
	    tcp_poll(pcb, net_tcp_poll_cb, 4);
		ip_set_option(pcb, SOF_KEEPALIVE);
		if(localportnum > 0 && localportnum <= 0xFFFF)
		{
			err = tcp_bind(pcb, IP_ADDR_ANY, localportnum);
			if (err != ERR_OK) 
			{
				TLS_DBGPRT_INFO("tcp bind failed %d\n", err);
	            return err;
			}
		}
        err = tcp_connect(pcb, &conn->addr, conn->port, net_tcp_connect_cb);
        if (err != ERR_OK) {
            TLS_DBGPRT_INFO("tcp connect failed %d\n", err);
            return err;
        }
		if (conn->localport == 0){
			conn->localport = pcb->local_port;
		}
    } else {
        TLS_DBGPRT_INFO("pcb ptr = 0x%x, conn->port = %d\n",
                pcb, conn->port);
        err = tcp_bind(pcb, IP_ADDR_ANY, conn->port);
	 	if (err != ERR_OK) {
            TLS_DBGPRT_INFO("tcp bind failed %d\n", err);
            return err;
        }
        conn->pcb.tcp = tcp_listen(pcb);
        //ip_set_option(conn->pcb.tcp, SOF_KEEPALIVE);
        if (conn->pcb.tcp == NULL) {
            /* create tcp sever failed */
            TLS_DBGPRT_INFO("tcp listen failed\n");
            return ERR_VAL;
        }
        tcp_accept(conn->pcb.tcp, net_tcp_accept_cb);
    }

    return ERR_OK;
}

/**
 * Start create UDP connection
 */
static err_t net_udp_start(struct tls_netconn *conn)
{
	struct udp_pcb *udppcb;
	u16 localportnum = conn->localport;

	if(localportnum == 0)
	{
		localportnum = conn->port;
	}
	udppcb = udp_new();

    if(udppcb == NULL) {
        return ERR_MEM;
    }	
	conn->pcb.udp = udppcb;
    if (udp_bind(udppcb, IP_ADDR_ANY, localportnum)!= ERR_OK){
        TLS_DBGPRT_INFO("udp connect failed\n");
        return ERR_CONN;
    }	
	
    /* Set a receive callback for the upcb */
    udp_recv(udppcb, net_udp_recv_cb, conn);

    return ERR_OK;
}
#if 0
static err_t net_do_writemore(struct tls_netconn *conn)
{
    err_t err = ERR_OK;
    void *dataptr;
    u16 len, available;
    u8 write_finished = 0;
    u32 diff;
    u8 apiflags = TCP_WRITE_FLAG_COPY;
    u8 dontblock = 0;    

    dataptr = (u8_t*)conn->current_msg->dataptr + conn->current_msg->write_offset;
    diff = conn->current_msg->len - conn->current_msg->write_offset;
    if (diff > 0xffffUL) { /* max_u16_t */
        len = 0xffff;
        apiflags |= TCP_WRITE_FLAG_MORE;
    } else {
        len = (u16_t)diff;
    }
    available = tcp_sndbuf(conn->pcb.tcp);
    if (available < len) {
        /* don't try to write more than sendbuf */
        len = available;
        if (dontblock){ 
            if (!len) {
                err = ERR_WOULDBLOCK;
                goto err_mem;
            }
        } else {
            apiflags |= TCP_WRITE_FLAG_MORE;
        }
    }
    err = tcp_write(conn->pcb.tcp, dataptr, len, apiflags);

err_mem:
    if (err == ERR_OK) {
        conn->current_msg->write_offset += len;
        if ((conn->current_msg->write_offset == conn->current_msg->len) || dontblock) {
            /* everything was written */
            write_finished = 1;
            conn->current_msg->write_offset = 0;
        }
        tcp_output(conn->pcb.tcp);
    } else if ((err == ERR_MEM) && !dontblock) {
        /* If ERR_MEM, we wait for sent_tcp or poll_tcp to be called
           we do NOT return to the application thread, since ERR_MEM is
           only a temporary error! */

        /* tcp_write returned ERR_MEM, try tcp_output anyway */
        tcp_output(conn->pcb.tcp);

    } else {
        /* On errors != ERR_MEM, we don't try writing any more but return
           the error to the application thread. */
        write_finished = 1;
        conn->current_msg->len = 0;
    }
    if (write_finished) {
        /* everything was written: set back connection state
           and back to application task */
        conn->current_msg->err = err;
        conn->write_state = false;

        tls_mem_free(conn->current_msg);
        conn->current_msg = NULL;
    }
    return ERR_OK;
}
#endif 

static err_t net_skt_tcp_send(struct tls_net_msg *net_msg)
{
	struct tcp_pcb *pcb = net_msg->conn->pcb.tcp;
	err_t err;

	/* 
		When tcp error occured, lwip will delete the pcb and sometimes GSKT.
		This function maybe registered by GSKT_TimerSend, so we must check if GSKT has been delted!!! 
	*/
	err = tcp_write(pcb, net_msg->dataptr, net_msg->len, TCP_WRITE_FLAG_COPY);
	if (err == ERR_OK){
		//sys_sem_signal(net_msg->conn->op_completed);
		tcp_output(pcb);
	}
	else
	{
		TLS_DBGPRT_INFO("err:%d\n", err);
	}
	return err;
}


/**
 * Send data on a UDP pcb 
 */
static void net_do_send(void *ctx)
{
    struct tls_net_msg *msg = (struct tls_net_msg *)ctx;
    struct tls_netconn *conn = msg->conn;
    struct pbuf *p;

    //TLS_DBGPRT_INFO("=====>\n"); 

    p = msg->p;
#if LWIP_CHECKSUM_ON_COPY
    if (ip_addr_isany(&msg->addr)) {
        msg->err = udp_send_chksum(conn->pcb.udp, p,
                0, 0);
    } else {
        msg->err = udp_sendto_chksum(conn->pcb.udp, p,
                &msg->addr, msg->port,
                0, 0);
    }
#else /* LWIP_CHECKSUM_ON_COPY */
    if (ip_addr_isany(&msg->addr)) {
        msg->err = udp_send(conn->pcb.udp, p);
    } else {
        msg->err = udp_sendto(conn->pcb.udp, p, 
                &msg->addr, msg->port);
    }
#endif /* LWIP_CHECKSUM_ON_COPY */

    pbuf_free(p);
    sys_sem_signal(conn->op_completed);
}

/**
 * Send data on a TCP pcb 
 */
static void net_do_write(void *ctx)
{
    struct tls_net_msg *net_msg = (struct tls_net_msg *)ctx;
    struct tls_netconn *conn = net_msg->conn;
	struct tls_netconn *server_conn = NULL;

    //TLS_DBGPRT_INFO("s=%d,p=0x%x\n", conn->state, conn->pcb.tcp);

#if 0
    if (ERR_IS_FATAL(conn->last_err)) {
        net_msg->err = conn->last_err;
    } 
#endif
    if (conn->proto == TLS_NETCONN_TCP) {
#if LWIP_TCP
        if (conn->state != NETCONN_STATE_CONNECTED) {
            /* netconn is connecting, closing or in blocking write */
            net_msg->err = ERR_INPROGRESS;
        } else if (conn->pcb.tcp != NULL) {
            conn->write_state = true;
            //conn->write_offset = 0;
            net_msg->err = net_skt_tcp_send(net_msg);
            /* for both cases: if do_writemore was called, don't ACK the APIMSG
               since do_writemore ACKs it! */
        } else {
            net_msg->err = ERR_CONN;
            TLS_DBGPRT_INFO("==>err=%d\n", net_msg->err);
        }
	if(conn->client && conn->idle_time > 0)
	{
		TLS_DBGPRT_INFO("conn->skt_num=%d, conn->client=%d\n", conn->skt_num, conn->client);
		server_conn = dl_list_first(&conn->list, struct tls_netconn, list);
		TLS_DBGPRT_INFO("server_conn=%p\n", server_conn);
		if(server_conn)
		{
			conn->idle_time = server_conn->idle_time;
			TLS_DBGPRT_INFO("update conn->idle_time %d\n", conn->idle_time);
		}
	}
#else /* LWIP_TCP */
        net_msg->err = ERR_VAL;
#endif /* LWIP_TCP */
#if (LWIP_UDP || LWIP_RAW)
    } else {
        net_msg->err = ERR_VAL;
#endif /* (LWIP_UDP || LWIP_RAW) */
    }
        //tls_mem_free(net_msg->dataptr);
    //if(net_msg->err != ERR_OK)
    {
        //TLS_DBGPRT_INFO("conn->proto=%d, err=%d\n", conn->proto, net_msg->err);
        //TLS_DBGPRT_INFO("free net_msg->dataptr=%p\n", net_msg->dataptr);
        sys_sem_signal(conn->op_completed);
    }
#if 0
    tls_mem_free(net_msg);
#endif
}

static void do_create_connect(void *ctx)
{
    struct tls_net_msg *net_msg = (struct tls_net_msg *)ctx;
    struct tls_netconn *conn;
	err_t  err;

    //TLS_DBGPRT_INFO("=====>\n");

    conn = net_msg->conn;
    TLS_DBGPRT_INFO("conn ptr = 0x%x, conn->skt_num=%d\n", conn, conn->skt_num);

    switch (conn->proto) {
        case TLS_NETCONN_UDP:
            err = net_udp_start(conn);
			if (err != ERR_OK){
				conn->state = NETCONN_STATE_NONE;
				net_send_event_to_hostif(conn, NET_EVENT_UDP_START_FAILED);
				net_free_socket(conn);				
			}else{
				conn->state = NETCONN_STATE_CONNECTED;
				net_send_event_to_hostif(conn, NET_EVENT_UDP_START); 
			}
            break;
        case TLS_NETCONN_TCP:
            err = net_tcp_start(conn);
			if (err != ERR_OK){
				conn->state = NETCONN_STATE_NONE;
				net_send_event_to_hostif(conn, NET_EVENT_TCP_CONNECT_FAILED);
				net_free_socket(conn);
			}else{
				if (!conn->client){
					net_send_event_to_hostif(conn, NET_EVENT_TCP_CONNECTED);
				}
				//conn->state = NETCONN_STATE_CONNECTED;
			}
            break;
        default:
            /* Unsupported netconn type, e.g. protocol disabled */
            break;
    }
    tls_mem_free(net_msg);
    return;
}

static void do_close_connect(void *ctx)
{
    struct tls_net_msg *net_msg = (struct tls_net_msg *)ctx;
    struct tls_netconn *conn = net_msg->conn;
    struct tls_netconn *client_conn;
    int i;
    int sktNums[2*TLS_MAX_NETCONN_NUM] = {-1};

    if (conn->used == false) 
        return;

    switch (conn->proto) {
        case TLS_NETCONN_UDP:
            if (conn->pcb.udp != NULL) {
                conn->pcb.udp->recv_arg = NULL;
                udp_remove(conn->pcb.udp);
                conn->pcb.udp = NULL;
            }
	     conn->state = NETCONN_STATE_CLOSED;
            break;
        case TLS_NETCONN_TCP:
            if (!conn->client) {
                /* it's a server, close connected client */
		i=0;
		dl_list_for_each(client_conn, &conn->list, struct tls_netconn, list){
                    if (client_conn->used) {
                        client_conn->state = NETCONN_STATE_CLOSED;
				sktNums[i++] = client_conn->skt_num;
                    }
                }
		  while(i-->0)
		  {
		  	client_conn = tls_net_get_socket(sktNums[i]);
			net_tcp_close_connect(client_conn); 
		  }
            }
            conn->state = NETCONN_STATE_CLOSED;
            net_tcp_close_connect(conn);
            break;
        default:
            break;
    }
    net_free_socket(conn);
    tls_mem_free(net_msg);
}

int tls_socket_create_inline(struct tls_socket_desc * skd, u8 socket_type)
{
    struct tls_netconn *conn = NULL;
    struct tls_net_msg *net_msg;
    err_t err = ERR_OK;

    //TLS_DBGPRT_INFO("=====>socket_type=%d, skd->protocol=%d\n", socket_type, skd->protocol);

    if ((skd->protocol == SOCKET_PROTO_TCP) || (skd->protocol == SOCKET_PROTO_UDP)) {
        conn = net_alloc_socket(NULL, socket_type);

        if (conn) {
            conn->client = (skd->cs_mode == SOCKET_CS_MODE_CLIENT);
            conn->proto = skd->protocol;
            if (conn->client){
                MEMCPY((char *)&conn->addr.addr, (char *)&skd->ip_addr, 4);
                conn->state = NETCONN_STATE_NONE;
            }
            else {
                ip_addr_set(&conn->addr, IP_ADDR_ANY);  
				conn->state = NETCONN_STATE_WAITING;
            }
            conn->socket_type = socket_type;
            conn->port = skd->port;
	     	conn->localport = skd->localport;
            conn->idle_time = skd->timeout;
	     	conn->skd = skd;
            //conn->cmd_mode = cmd_mode;

            net_msg = tls_mem_alloc(sizeof(*net_msg));
            if (net_msg == NULL) {
                net_free_socket(conn);
                return -1;
            }
            memset(net_msg, 0, sizeof(*net_msg));
            net_msg->conn = conn;
            err = tcpip_callback_with_block(do_create_connect, net_msg, 0);
            if (err) {
                net_free_socket(conn);
                conn->state = NETCONN_STATE_NONE;
                conn = NULL;
                tls_mem_free(net_msg);
            }
        } else {
            return 0;
        }
    } else {
        err = ERR_VAL;
    }

    if (conn)
        return conn->skt_num; 
    else 
        return -1;
}
int tls_socket_create(struct tls_socket_desc * skd)
{
    return tls_socket_create_inline(skd, TLS_SOCKET_TYPE_CMD);
}

int tls_socket_get_status(u8 socket, u8 *buf, u32 bufsize)
{
    struct tls_skt_status_t *skt_status;
    struct tls_skt_status_ext_t *skts_ext;
    struct tls_netconn *conn;
    struct tls_netconn *client_conn;
    struct tls_netconn *server_conn;
    int remain_len;

    skt_status = (struct tls_skt_status_t *)buf;

    if (bufsize < sizeof(struct tls_skt_status_t))
        return ERR_VAL; 

    if (socket < 1 || socket > 20)
        return ERR_VAL;

    memset(buf, 0, bufsize);

    conn = tls_net_get_socket(socket);
    remain_len = bufsize - sizeof(u32);
    skts_ext = (struct tls_skt_status_ext_t *)(buf + sizeof(u32));
    skts_ext->protocol = (enum tls_socket_protocol)conn->proto;
//TLS_DBGPRT_INFO("conn->client=%d, conn->state=%d, conn->used=%d\n", conn->client, conn->state,conn->used);
    if (conn->client) {
        skt_status->socket_cnt = 1;
        if (conn->used) {
            MEMCPY(skts_ext->host_ipaddr, &conn->addr.addr, 4);
            if (socket < 4) {
                skts_ext->local_port = conn->localport;
                skts_ext->remote_port = conn->port;
            } else {
                server_conn = dl_list_first(&conn->list, struct tls_netconn, list);
				TLS_DBGPRT_INFO("server_conn->skt_num = %d\n", server_conn->skt_num);
                skts_ext->local_port = server_conn->localport;
                skts_ext->remote_port = conn->port; 
            }
            skts_ext->status = conn->state; /* connected */
            skts_ext->socket = conn->skt_num; 
        } else {
            skts_ext->status = conn->state;   /* disconnect */ 
            skts_ext->socket = conn->skt_num;
        }
//TLS_DBGPRT_INFO("skts_ext->socket=%d, skts_ext->status=%d, conn->state=%d\n", skts_ext->socket, skts_ext->status, conn->state);
    } else {
    	struct tls_ethif * ethif;
	ethif = tls_netif_get_ethif();
        skt_status->socket_cnt = 1;
        if (conn->used) {
            skts_ext->status = conn->state; /* listen */
            //MEMCPY(skts_ext->host_ipaddr, &conn->addr.addr, 4);
            //MEMCPY(skts_ext->host_ipaddr, (char *)&sys->ethif->nif->ip_addr.addr, 4);
            //MEMCPY(&conn->addr.addr, (char *)&sys->ethif->nif->ip_addr.addr, 4);
            MEMCPY(skts_ext->host_ipaddr, (char *)&ethif->ip_addr.addr, 4);
            skts_ext->local_port = conn->port;
            skts_ext->remote_port = 0;
            skts_ext->socket = conn->skt_num;

            remain_len -= sizeof(struct tls_skt_status_ext_t);
            
	     	dl_list_for_each(client_conn, &conn->list, struct tls_netconn, list){
                if (remain_len < sizeof(struct tls_skt_status_ext_t))
                    break;
                if (client_conn->used) {
                    skts_ext++;
                    skts_ext->status = client_conn->state; /* connect */
                    skt_status->socket_cnt++;
                    MEMCPY(skts_ext->host_ipaddr, &client_conn->addr.addr, 4);
                    skts_ext->local_port = conn->port;
                    skts_ext->remote_port = client_conn->port; 
                    skts_ext->socket = client_conn->skt_num;
                    skts_ext->protocol = (enum tls_socket_protocol)client_conn->proto;
                }
            }
        } else {
            skts_ext->status = conn->state;
            skts_ext->socket = conn->skt_num;
        }
    }

    return 0;
}

struct tls_netconn * tls_net_get_socket(u8 socket)
{
    struct tls_netconn *conn;

    if (socket < 1 || socket > TLS_MAX_NETCONN_NUM * 2)
        return NULL;


	if(socket <= TLS_MAX_NETCONN_NUM)
	{
    		conn = &g_net_conn[socket-1];
	}
	else
	{
		conn = p_net_conn[socket - TLS_MAX_NETCONN_NUM -1];
    }
	//TLS_DBGPRT_INFO("conn->skt_num = %d\n", conn->skt_num);
    return conn;
}

int tls_socket_close(u8 socket)
{
    struct tls_net_msg *net_msg;
    struct tls_netconn *conn;
    err_t err;
//TLS_DBGPRT_INFO("socket=%d.\n", socket);
    if (socket < 1 || socket > TLS_MAX_NETCONN_NUM*2)
        return -1;

    conn = tls_net_get_socket(socket);

	if(conn == NULL || !conn->used)
	{
		return -1;
	}
    net_msg = tls_mem_alloc(sizeof(*net_msg));
    if (net_msg == NULL) {
        return -1;
    }
    memset(net_msg, 0, sizeof(*net_msg));
    net_msg->conn = conn;
    err = tcpip_callback_with_block(do_close_connect, net_msg, 0);
    if (err) {
        tls_mem_free(net_msg);
    }

    return err;

}

int tls_socket_udp_sendto(u16 localport, u8  *ip_addr, u16 port, void *pdata, u16 len)
{
    struct tls_netconn *conn;
    struct tls_net_msg *net_msg = NULL;
    struct pbuf *p;
    int i;
    int err = 0;
    bool found = false;

        /* find match socket */
        for (i = 0; i<TLS_MAX_NETCONN_NUM * 2; i++) {
            conn = tls_net_get_socket(i);
            if (conn != NULL && conn->used && 
                    (localport== conn->port) &&
                    conn->proto == TLS_NETCONN_UDP) {
                found = true;
                break;
            }
        }
        if (!found) {
            err = -1;
            goto out;
        }
       net_msg = tls_mem_alloc(sizeof(struct tls_net_msg));
	if(!net_msg)
		return ERR_MEM;
	net_msg->len = len;
	net_msg->dataptr = pdata;

        /* udp */
        while (1) {
            p = pbuf_alloc(PBUF_TRANSPORT, net_msg->len, PBUF_REF);
            if (p != NULL) {
                break;
            } else {
                /* delay 1 ticks */
                tls_os_time_delay(1);
            } 
        }
        p->payload = (void*)net_msg->dataptr;
        p->len = p->tot_len = net_msg->len;

        net_msg->err = ERR_VAL;/* for debug : catch not set err */
        net_msg->p = p;
        net_msg->port = port;
        net_msg->conn = conn;
        MEMCPY(&net_msg->addr.addr, ip_addr, 4);
        err = tcpip_callback_with_block(net_do_send, net_msg, 0);
out:
    if(err != ERR_OK && net_msg)
        tls_mem_free(net_msg);
    return err;
}

static err_t netconn_msg(tcpip_callback_fn function, struct tls_net_msg *net_msg, u8_t block)
{
	err_t err = ERR_OK;
	struct tls_netconn *conn = net_msg->conn;
#if 1
	err = tcpip_callback_with_block(function, net_msg, block);
	if(err)
	{
		//TLS_DBGPRT_INFO("free net_msg->dataptr=%p\n", net_msg->dataptr);
		//tls_mem_free(net_msg->dataptr);
		TLS_DBGPRT_INFO("callback end, err=%d\n", err);
		tls_mem_free(net_msg);
		return err;
	}
	sys_arch_sem_wait(conn->op_completed, 0);
	err = net_msg->err;
	return err;
#else
	struct tls_net_msg *net_msg_item;
	if(conn->current_msg == NULL)
	{
		conn->current_msg = net_msg;
	}
	else
	{
		dl_list_add_tail(&conn->current_msg->list, &net_msg->list);
	}
	//TLS_DBGPRT_INFO("callback\n");
	err = tcpip_callback_with_block(function, conn->current_msg, block);
	if(err)
	{
		TLS_DBGPRT_INFO("callback end, err=%d\n", err);
		return err;
	}
	sys_arch_sem_wait(conn->op_completed, 0);
	//TLS_DBGPRT_INFO("callback end\n");
	if(conn->current_msg->err)
		return conn->current_msg->err;
		
	dl_list_for_each(net_msg_item, &conn->current_msg->list, struct tls_net_msg, list){
		err = tcpip_callback_with_block(function, net_msg_item, block);
		if(err)
			break;
		sys_arch_sem_wait(conn->op_completed, 0);
		if(net_msg_item->err)
			break;
		dl_list_del(&net_msg_item->list);
		tls_mem_free(net_msg_item);
	}
	if(dl_list_empty(&conn->current_msg->list))
	{
		tls_mem_free(conn->current_msg);
		conn->current_msg = NULL;
	}
	else
	{
		tls_mem_free(conn->current_msg);
		conn->current_msg = net_msg_item;
	}
	return ERR_OK;
#endif
}

int tls_socket_send(u8 skt_num, void *pdata, u16 len)
{
    struct tls_net_msg *net_msg = NULL;
    struct tls_netconn *conn;
    struct tls_netconn *client_conn;
    struct pbuf *p;
    err_t err = ERR_OK;
    u32 socket;

    socket = skt_num;
    net_msg = tls_mem_alloc(sizeof(struct tls_net_msg));
	if(!net_msg)
	{
		TLS_DBGPRT_INFO("alloc net_msg err.\n");
		return ERR_MEM;
	}
	memset(net_msg, 0, sizeof(struct tls_net_msg));
	dl_list_init(&net_msg->list);
	net_msg->len = len;
	net_msg->dataptr = pdata;
        if (socket < 1 || socket > TLS_MAX_NETCONN_NUM*2)
        {
            tls_mem_free(net_msg);
            return ERR_VAL;
        }
        conn = tls_net_get_socket(socket);

        if (conn == NULL || conn->used == false)
        {
            tls_mem_free(net_msg);
            return ERR_VAL;
        }
        if (conn->proto == TLS_NETCONN_UDP) {
	        net_msg->conn = conn;
	        net_msg->err = ERR_VAL;/* for debug : catch not set err */
            do{
                p = pbuf_alloc(PBUF_TRANSPORT, net_msg->len, PBUF_REF);
                if (p != NULL) {
                    break;
                } else {
                    TLS_DBGPRT_INFO("pbuf_alloc err.\n");
                    tls_mem_free(net_msg);
                    return ERR_VAL;
                } 
            }while (0);
            p->payload = (void*)net_msg->dataptr;
            p->len = p->tot_len = net_msg->len;

            net_msg->p = p;
            net_msg->port = conn->port;
            ip_addr_set(&net_msg->addr, &conn->addr);

            err = netconn_msg(net_do_send, net_msg, 0);			
        } else if (conn->proto == TLS_NETCONN_TCP) {
        	if (conn->client){
	            net_msg->conn = conn;
		        net_msg->err = ERR_VAL;/* for debug : catch not set err */
	            err = netconn_msg(net_do_write, net_msg, 0);
        	}else{
        		dl_list_for_each(client_conn, &conn->list, struct tls_netconn, list){
				if (client_conn->used){
					net_msg->conn = client_conn;
		        	net_msg->err = ERR_VAL;/* for debug : catch not set err */
	            	err = netconn_msg(net_do_write, net_msg, 0);
				}
        		}
				
        	}
        } else {
            err = -1;
        }
	tls_mem_free(net_msg);
    return err;
}

int tls_net_init()
{
    int i;
    memset(p_net_conn, 0, sizeof(struct tls_netconn *) * TLS_MAX_NETCONN_NUM);
    memset(g_net_conn, 0, sizeof(struct tls_netconn) * TLS_MAX_NETCONN_NUM);
    for (i = 0; i < TLS_MAX_NETCONN_NUM; i++) {
        g_net_conn[i].used = 0;
        g_net_conn[i].state = NETCONN_STATE_NONE;
        g_net_conn[i].skt_num = i+1;
        dl_list_init(&g_net_conn[i].list);
        if (sys_sem_new(&g_net_conn[i].op_completed, 0) != ERR_OK) {
            return ERR_MEM;
        }
	TLS_DBGPRT_INFO("tls_net_init conn ptr = 0x%x\n", &g_net_conn[i]);
    }

    return 0;
}

#endif //TLS_CONFIG_SOCKET_RAW

