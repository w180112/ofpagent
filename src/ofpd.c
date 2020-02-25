/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\
  OFPD.C

    - purpose : for ofp detection
	
  Designed by THE on 30/09/'19
/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

#include        	<common.h>
#include 			<unistd.h>
#include        	"ofpd.h"
#include			"ofp_codec.h"
#include			"ofp_fsm.h"
#include        	"ofp_dbg.h"
#include 			"ofp_sock.h" 

BOOL				ofp_testEnable=FALSE;
U32					ofp_ttl;
U32					ofp_interval;
U16					ofp_init_delay;
U8					ofp_max_msg_per_query;

tOFP_PORT			ofp_ports[MAX_USER_PORT_NUM+1]; //port is 1's based

tIPC_ID 			ofpQid=-1;

extern int			ofp_io_fds[2];

pid_t ofp_cp_pid;
pid_t ofp_dp_pid;
pid_t tmr_pid;

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
int ofpdInit(void)
{
	U16  i;
	
	if (OFP_SOCK_INIT() < 0){ //must be located ahead of system init
		return -1;
	}
	
	ofp_interval = (U32)(10*SEC);
	tmr_pid = tmrInit();
    
    //--------- default of all ports ----------
	for(i=1; i<=MAX_USER_PORT_NUM; i++){
		ofp_ports[i].enable = FALSE;
		ofp_ports[i].query_cnt = 1;
		ofp_ports[i].state = S_CLOSED;
		ofp_ports[i].port = i;
		
		ofp_ports[i].imsg_cnt =
		ofp_ports[i].err_imsg_cnt =
		ofp_ports[i].omsg_cnt = 0;
		ofp_ports[i].head = NULL;
	}
	
	sleep(1);
	ofp_max_msg_per_query = MAX_OFP_QUERY_NUM;
	ofp_testEnable = TRUE; //to let driver ofp msg come in ...
	DBG_OFP(DBGLVL1,NULL,"============ ofp init successfully ==============\n");

	return 0;
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
	tOFP_PORT		*ccb;
	tMBUF   		mbuf;
	int				msize;
	//U16				port;
	U16				event;
	U16				ipc_type;
	
	if (ofpdInit() < 0)
		return -1;
	OFP_ipc_init();
	if ((ofp_cp_pid=fork()) == 0) {
   		ofp_sockd_cp();
    }
	if ((ofp_dp_pid=fork()) == 0) {
   		ofp_sockd_dp();
    }
    signal(SIGINT,OFP_bye);
    
	ofp_ports[0].sockfd = ofp_io_fds[0];
    OFP_FSM(&ofp_ports[0], E_START);
    
	for(;;) {
		//printf("\n===============================================\n");
		//printf("%s> waiting for ipc_rcv2() ...\n",CODE_LOCATION);
	    if (ipc_rcv2(ofpQid, &mbuf, &msize) == ERROR) {
	    	//printf("\n%s> ipc_rcv2 error ...\n",CODE_LOCATION);
	    	continue;
	    }
	    
	    ipc_type = *(U16*)mbuf.mtext;
	    //printf("ipc_type=%d\n",ipc_type);
		
		switch(ipc_type){
		case IPC_EV_TYPE_TMR:
			ipc_prim = (tIPC_PRIM*)mbuf.mtext;
			ccb = ipc_prim->ccb;
			event = ipc_prim->evt;
			OFP_FSM(ccb, event);
			break;
		
		case IPC_EV_TYPE_DRV:
			mail = (tOFP_MBX*)mbuf.mtext;
			//ofp_ports[port].port = TEST_PORT_ID;
			DBG_OFP(DBGLVL1,&ofp_ports[0],"<-- Rx ofp message\n");
			if (OFP_decode_frame(mail, &ofp_ports[0]) == ERROR)
				continue;
			else if (OFP_decode_frame(mail, &ofp_ports[0]) == RESTART) {
				if (ofpdInit() < 0)
					return -1;
				if ((ofp_cp_pid=fork()) == 0)
   					ofp_sockd_cp();
				if ((ofp_dp_pid=fork()) == 0)
   					ofp_sockd_dp();
				ofp_ports[0].sockfd = ofp_io_fds[0];
				OFP_FSM(&ofp_ports[0], E_START);
				puts("====================Restart connection.====================");
			}
			OFP_FSM(&ofp_ports[0], ofp_ports[0].event);
			break;
		
		//case IPC_EV_TYPE_CLI:
		default:
		    ;
		}
    }
}
