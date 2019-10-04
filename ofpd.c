/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\
  OFPD.C

    - purpose : for ofp detection
	
  Designed by THE on 30/09/'19
/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

#include        	<common.h>
#include        	"ofpd.h"
#include			"ofp_codec.h"
#include			"ofp_fsm.h"
#include        	"ofp_dbg.h"
#include 			"ofp_sock.h" 

BOOL				ofp_testEnable=FALSE;
BOOL				ofp_opt_tlvs[OFP_MAX_OPT_TLV_NUM];
U32					ofp_ttl;
U32					ofp_interval;
U16					ofp_init_delay;
U8					ofp_max_msg_per_query;

tOFP_PORT			ofp_ports[MAX_USER_PORT_NUM+1]; //port is 1's based

tIPC_ID 			ofpQid=-1;

/*---------------------------------------------------------
 * ofp_bye : signal handler for INTR-C only
 *--------------------------------------------------------*/
void OFP_bye()
{
    printf("ofp> delete Qid(0x%x)\n",ofpQid);
    DEL_MSGQ(ofpQid);
    printf("bye!\n");
	exit(0);
}

/*---------------------------------------------------------
 * OFP_ipc_init
 *--------------------------------------------------------*/
STATUS OFP_ipc_init(void)
{
	if (ofpQid != -1){
		printf("ofp> Qid(0x%x) is already existing\n",ofpQid);
		return TRUE;
	}
	
	if (GET_MSGQ(&ofpQid,OFP_Q_KEY) == ERROR){
	   	printf("ofp> can not create a msgQ for key(0x0%x)\n",OFP_Q_KEY);
	   	return ERROR;
	}
	printf("ofp> new Qid(0x%x)\n",ofpQid);
	return TRUE;
}

/**************************************************************
 * ofpdInit: 
 *
 **************************************************************/
void ofpdInit(void)
{
	U16  i;
	
	if (OFP_SOCK_INIT() < 0){ //must be located ahead of system init
		return;
	}
	
	ofp_interval = (U32)(3*SEC);
	tmrInit();
	OFP_ipc_init();
    
    //--------- default of all ports ----------
	for(i=1; i<=MAX_USER_PORT_NUM; i++){
		ofp_ports[i].enable = FALSE;
		ofp_ports[i].query_cnt = 1;
		ofp_ports[i].state = S_CLOSED;
		ofp_ports[i].port = i;
		
		ofp_ports[i].imsg_cnt =
		ofp_ports[i].err_imsg_cnt =
		ofp_ports[i].omsg_cnt = 0;
	}
	ofp_ports[1].enable = TRUE;
	
	for(i=0; i<OFP_MAX_OPT_TLV_NUM; i++){
		ofp_opt_tlvs[i] = FALSE;
	}
    
	sleep(1);
	ofp_testEnable = TRUE; //to let driver ofp msg come in ...
	DBG_OFP(DBGLVL1,NULL,"============ ofp init successfully ==============\n");
}
            
/***************************************************************
 * ofpd : 
 *
 ***************************************************************/
int main(int argc, char **argv)
{
	extern STATUS	OFP_FSM(tOFP_PORT *port_ccb, U16 event);
	tIPC_PRIM		*ipc_prim;
    tOFP_MBX		*mail;
	tOFP_PORT		*ccb, ofp_header;
	tMBUF   		mbuf;
	int				msize;
	U16				port;
	U16				event;
	U16				ipc_type;
	
	ofpdInit();
	
	if (fork() == 0){
   		ofp_sockd();
    }
    signal(SIGINT,OFP_bye);
    
    OFP_FSM(&ofp_ports[TEST_PORT_ID], Q_link_up, 0);
    
	for(;;){
		//printf("\n===============================================\n");
		//printf("%s> waiting for ipc_rcv2() ...\n",CODE_LOCATION);
	    if (ipc_rcv2(ofpQid, &mbuf, &msize) == ERROR){
	    	//printf("\n%s> ipc_rcv2 error ...\n",CODE_LOCATION);
	    	continue;
	    }
	    
	    ipc_type = *(U16*)mbuf.mtext;
	    //printf("ipc_type=%d\n",ipc_type);
		
		switch(ipc_type){
		case IPC_EV_TYPE_TMR:
			if (ipc_type == IPC_EV_TYPE_TMR){
				ipc_prim = (tIPC_PRIM*)mbuf.mtext;
				ccb = ipc_prim->ccb;
				event = ipc_prim->evt;
				OFP_FSM(ccb, event, 0);
			}
			break;
		
		case IPC_EV_TYPE_DRV:
			mail = (tOFP_MBX*)mbuf.mtext;
			ofp_ports[port].port = TEST_PORT_ID;
			DBG_OFP(DBGLVL1,&ofp_ports[TEST_PORT_ID],"<-- Rx ofp message\n");
			if (OFP_decode_frame(mail, &ofp_ports[0]) == ERROR) {
				ofp_ports[port].err_imsg_cnt++;
				continue;
			}
			OFP_FSM(&ofp_ports[TEST_PORT_ID], event);
			break;
		
		//case IPC_EV_TYPE_CLI:
		default:
		    ;
		}
		
		/*
		get_msgQ(ofpmsgQ, &mail);
		
		switch(mail.type){
		case MT_ofp_peer:
			port = mail.port;
			if (port < 1 || port > MAX_USER_PORT_NUM)  continue;	
			if (!ofp_ports[port].enable)  continue;
			
			if (OFP_decode_frame(&mail, &imsg) == ERROR){
				ofp_ports[port].err_imsg_cnt++;
				continue;
			}
			
			event = mail.evt;
			DBG_OFP(DBGLVL1,"[%d] <-- Rx ofp message\n",port);
			OFP_FSM(&ofp_ports[port], event, &imsg);
			break;
			
		case MT_ofp_crt:
			port = mail.port;
			event = mail.evt;
			OFP_FSM(&ofp_ports[port], event, ofp_data);
			break;
			
		case MT_ofp_tmr:
			port = ((tOFP_PORT*)mail.ccb)->port;
			if (port < 1 || port > MAX_USER_PORT_NUM)  continue;	
			if (!ofp_ports[port].enable)  continue;
			
			event = mail.evt;
			OFP_FSM(mail.ccb, event, ofp_data);
			break;
		
		case MT_ofp_link:
			OFP_FSM(&ofp_ports[mail.port], mail.evt, ofp_data);
			break;
			
		default:
			;
		}*/
    }
}
