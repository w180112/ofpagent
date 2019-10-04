/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\
  ofp_fsm.c
  
     State Transition Table 

  Designed by THE
  Date: 30/09/2019
/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

#include 		<common.h>
#include		"ofpd.h"
#include		"ofp_codec.h"
#include    	"ofp_sock.h"
#include    	"ofp_fsm.h"
#include		"ofp_dbg.h"

char 			*OFP_state2str(U16 state);

static STATUS 	A_send_hello(tOFP_PORT*, void*);
static STATUS 	A_send_echo_request(tOFP_PORT*, void*);
static STATUS   A_send_feature_reply(tOFP_PORT*, void*);
static STATUS   A_send_multipart_reply(tOFP_PORT*, void*);
static STATUS   A_start_timer(tOFP_PORT*, void*);

tOFP_STATE_TBL  ofp_fsm_tbl[] = { 
/*//////////////////////////////////////////////////////////////////////////////////
  	STATE   		EVENT           		NEXT-STATE        HANDLER       
///////////////////////////////////////////////////////////////////////////////////\*/
{ S_CLOSED,			E_START,     			S_HELLO_WAIT,	{ A_send_hello, 0 }},

{ S_HELLO_WAIT,		E_RECV_HELLO,     		S_FEATURE_WAIT,	{ 0 }},

{ S_FEATURE_WAIT,	E_FEATURE_REQUEST,     	S_ESTABLISHED,	{ A_send_feature_reply, A_send_echo_request, A_start_timer, 0 }},

{ S_ESTABLISHED,	E_MULTIPART_REQUEST,    S_ESTABLISHED,	{ A_send_multipart_reply, 0 }},

{ S_ESTABLISHED,	E_OTHERS,    			S_ESTABLISHED,	{ 0 }},

{ S_INVALID, 0 }
};
 
/***********************************************************************
 * OFP_FSM
 *
 * purpose : finite state machine.
 * input   : tnnl - tunnel pointer
 *           event -
 *           arg - signal(primitive) or pdu
 * return  : error status
 ***********************************************************************/
STATUS OFP_FSM(tOFP_PORT *port_ccb, U16 event, void *arg)
{	
    register int  	i,j;
    STATUS			retval;
    char 			str1[30],str2[30];

	if (!port_ccb){
        DBG_OFP(DBGLVL1,port_ccb,"Error! No port found for the event(%d)\n",event);
        return FALSE;
    }
    
    /* Find a matched state */
    for(i=0; ofp_fsm_tbl[i].state!=S_PORT_INVLD; i++)
        if (ofp_fsm_tbl[i].state == port_ccb->state)
            break;

    if (ofp_fsm_tbl[i].state == S_PORT_INVLD){
        DBG_OFP(DBGLVL1,port_ccb,"Error! unknown state(%d) specified for the event(%d)\n",
        	port_ccb->state,event);
        return FALSE;
    }

    /*
     * Find a matched event in a specific state.
     * Note : a state can accept several events.
     */
    for(;ofp_fsm_tbl[i].state==port_ccb->state; i++)
        if (ofp_fsm_tbl[i].event == event)
            break;
    
    if (ofp_fsm_tbl[i].state != port_ccb->state){ /* search until meet the next state */
        DBG_OFP(DBGLVL1,port_ccb,"error! invalid event(%d) in state(%s)\n",
            event, OFP_state2str(port_ccb->state));
  		return TRUE; /* still pass to endpoint */
    }
    
    /* Correct state found */
    if (port_ccb->state != ofp_fsm_tbl[i].next_state){
    	strcpy(str1,OFP_state2str(port_ccb->state));
    	strcpy(str2,OFP_state2str(ofp_fsm_tbl[i].next_state));
        DBG_OFP(DBGLVL1,port_ccb,"state changed from %s to %s\n",str1,str2);
        port_ccb->state = ofp_fsm_tbl[i].next_state;
    }
    
	for(j=0; ofp_fsm_tbl[i].hdl[j]; j++){
       	retval = (*ofp_fsm_tbl[i].hdl[j])(port_ccb,(void*)arg);
       	if (!retval)  return TRUE;
    }
    return TRUE;
}

/*********************************************************************
 * A_check_up_port_cfg: 
 *
 *********************************************************************/
STATUS A_send_hello(tOFP_PORT *port_ccb, void *m)	
{
	unsigned char buffer[256];
	struct ofp_header of_header;

	of_header.version = 0x04;
	of_header.type = OFPT_HELLO;
	uint16_t length = of_header.length = sizeof(struct ofp_header);
	of_header.length = htons(of_header.length);
	of_header.xid = 0x5;

	drv_xmit(buffer, length);

	return TRUE;
}

/*********************************************************************
 * A_config_port: 
 *
 *********************************************************************/
STATUS A_send_echo_request(tOFP_PORT *port_ccb, void *m)	
{
	unsigned char buffer[256];
	struct ofp_switch_features_t ofp_switch_features;

	of_header.version = 0x04;
	of_header.type = OFPT_ECHO_REQUEST;
	uint16_t length = of_header.length = sizeof(struct ofp_header);
	of_header.length = htons(of_header.length);
	of_header.xid = 0;

	drv_xmit(buffer, length);

	return TRUE;
}

/*********************************************************************
 * A_send_feature_reply: 
 *
 *********************************************************************/
STATUS A_send_feature_reply(tOFP_PORT *port_ccb, void *m)	
{
	unsigned char buffer[256];
	ofp_switch_features_t ofp_switch_features;
	struct ifaddrs *ifaddr, *ifa;

	ofp_switch_features.ofp_header.version = 0x04;
	ofp_switch_features.ofp_header.type = OFPT_FEATURES_REPLY;
	ofp_switch_features.ofp_header.length = sizeof(struct ofp_header);
	ofp_switch_features.ofp_header.xid = 0;

	strcpy(ofp_port_desc.name,ifa.ifa_name);
	int fd;
    struct ifreq ifr;
	uint64_t tmp;

	if (getifaddrs(&ifaddr) == -1) {
    	perror("getifaddrs");
    	return -1;
  	}
    fd = socket(AF_INET, SOCK_DGRAM, 0);
 	ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name,ifa.ifa_name,IFNAMSIZ-1);
 	ioctl(fd,SIOCGIFHWADDR,&ifr);
    close(fd);
	ofp_switch_features.datapath_id = 0x0;
	for(int i=5; i>=0; i--) {
        tmp = ifr.ifr_hwaddr.sa_data[i];
        ofp_switch_features.datapath_id += tmp << (i*8);
    }
	printf("%llx\n", ofp_switch_features.datapath_id);
	ofp_switch_features.datapath_id = bitswap64(ofp_switch_features.datapath_id);
	ofp_switch_features.n_buffers = htonl(256);
	ofp_switch_features.n_tables = 254;
	ofp_switch_features.auxiliary_id = 0;
	ofp_switch_features.capabilities = OFPC_FLOW_STATS | OFPC_PORT_STATS | OFPC_QUEUE_STATS;
	ofp_switch_features.capabilities = htonl(ofp_switch_features.capabilities);
	ofp_switch_features.ofp_header.length += sizeof(ofp_switch_features_t);
	length = ofp_switch_features.ofp_header.length;
	ofp_switch_features.ofp_header.length = htons(ofp_switch_features.ofp_header.length);

	drv_xmit(buffer, length);

	return TRUE;
}

/*********************************************************************
 * A_start_timer: 
 *
 *********************************************************************/
STATUS A_start_timer(tOFP_PORT *port_ccb, void *m)
{
	DBG_OFP(DBGLVL1,port_ccb,"start query timer(%ld secs)\n",ofp_interval/SEC);
	OSTMR_StartTmr(ofpQid, port_ccb, ofp_interval, "ofp:txT", Q_ofp_query_expire);
	
	
	return TRUE;
}


#if 0
/*********************************************************************
 * A_stop_query_tmr: 
 *
 *********************************************************************/
STATUS A_stop_query_tmr(tOFP_PORT *port_ccb, void *m)
{
	DBG_OFP(DBGLVL1,port_ccb,"stop query timer\n");
	OSTMR_StopXtmr(port_ccb,Q_ofp_query_expire);
	return TRUE;
} 

/*********************************************************************
 * A_query_tmr_expire: 
 *
 *********************************************************************/
STATUS A_query_tmr_expire(tOFP_PORT *port_ccb, void *m)
{
	DBG_OFP(DBGLVL1,port_ccb,"query timer expired\n");
	//port_ccb->query_cnt++; remove this line to keep query forever ... until receive peer's ofp msg
	return TRUE;
}
#endif

/*********************************************************************
 * A_send_multipart_reply: 
 *
 *********************************************************************/
STATUS A_send_multipart_reply(tOFP_PORT *port_ccb, void *m)
{
	ofp_multipart_t ofp_multipart;
	struct ofp_port ofp_port_desc;
	struct ifaddrs *ifaddr, *ifa;
	U8 buf[256];
	uintptr_t buf_ptr;

	if (getifaddrs(&ifaddr) == -1) {
    	perror("getifaddrs");
    	return -1;
  	}

	ofp_multipart = port_ccb->ofp_multipart;
	ofp_multipart.ofp_header.type = OFPT_MULTIPART_REPLY;
	buf_ptr = buf + sizeof(ofp_multipart_t);
	ofp_multipart.ofp_header.length = sizeof(ofp_multipart_t);

	memset(&ofp_port_desc,0,sizeof(struct ofp_port));
	for(int i=0,ifa=ifaddr; ifa != NULL; i++, ifa=ifa->ifa_next) {
		if (ifa->ifa_addr == NULL || ifa->ifa_addr->sa_family != AF_PACKET) 
			continue;
		ofp_port_desc.port_no = i;
		strcpy(ofp_port_desc.name,ifa.ifa_name);
		int fd;
    	struct ifreq ifr;
 
    	fd = socket(AF_INET, SOCK_DGRAM, 0);
 		ifr.ifr_addr.sa_family = AF_INET;
    	strncpy(ifr.ifr_name,ifa.ifa_name,IFNAMSIZ-1);
 		ioctl(fd,SIOCGIFHWADDR,&ifr);
    	close(fd);
    	memcpy(ofp_port_desc.hw_addr,(unsigned char *)ifr.ifr_hwaddr.sa_data,OFP_ETH_ALEN);
		memcpy(buf+buf_ptr,&ofp_port_desc,sizeof(ofp_port));
		buf_ptr += sizeof(ofp_port);
		ofp_multipart.ofp_header.length += sizeof(ofp_port);
	}
	uint16_t length = ofp_multipart.ofp_header.length;
	ofp_multipart.ofp_header.length = htons(ofp_multipart.ofp_header.length);
	memcpy(buf,ofp_multipart,sizeof(ofp_multipart_t));
	drv_xmit(buf,length);
	freeifaddrs(ifaddr);
	return TRUE;
}

//============================== others =============================

/*-------------------------------------------------------------------
 * OFP_state2str
 *
 * input : state
 * return: string of corresponding state value
 *------------------------------------------------------------------*/
char *OFP_state2str(U16 state)
{
	static struct {
		OFP_STATE	state;
		char		str[10];
	} ofp_state_desc_tbl[] = {
    	{ S_CLOSED,  		"CLOSED  " },
    	{ S_HELLO_WAIT,  	"WAIT_HELLO    " },
    	{ S_FEATURE_WAIT,  	"WAIT_FEATURE" },
    	{ S_ESTABLISHED,	"ESTABLOSHED" },
    	{ S_INVALID,  		"INVALID " },
	};

	U8  i;
	
	for(i=0; ofp_state_desc_tbl[i].state != S_INVLD; i++){
		if (ofp_state_desc_tbl[i].state == state)  break;
	}
	if (ofp_state_desc_tbl[i].state == S_INVLD){
		return NULL;
	}
	return ofp_state_desc_tbl[i].str;
}
