/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\
  OFP_CODEC.C :

  Designed by THE on SEP 16, 2019
/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

#include 		<common.h>
#include		"ofpd.h"
#include		"ofp_codec.h"
#include        "ofp_fsm.h"
#include        "ofp_dbg.h"
#include 		"ofp_sock.h"

void OFP_encode_packet_in(void);

/*============================ DECODE ===============================*/

/*****************************************************
 * OFP_decode_frame
 * 
 * input : pArg - mail.param
 * output: imsg, event
 * return: session ccb
 *****************************************************/
STATUS OFP_decode_frame(tOFP_MBX *mail, tOFP_PORT *port_ccb)
{
    U16	mulen;
	U8	*mu;
	tOFP_MSG *msg;
	
	if (mail->len > ETH_MTU) {
	    DBG_OFP(DBGLVL1,0,"error! too large frame(%d)\n",mail->len);
	    return ERROR;
	}

	msg = (tOFP_MSG *)mail->refp;
	port_ccb->sockfd = msg->sockfd;
	mu = (U8 *)(msg->buffer);
	mulen = (mail->len) - (sizeof(int) + 1);


	if (msg->type == DRIV_DP) {
		OFP_encode_packet_in();
		port_ccb->event = E_PACKET_IN;
		return TRUE;
	}
    //PRINT_MESSAGE(mu,mulen);
	switch(((ofp_header_t *)mu)->type) {
	case OFPT_HELLO:
		printf("----------------------------------\nrecv hello msg\n");
		port_ccb->event = E_RECV_HELLO;
		memcpy(&(port_ccb->ofp_header),mu,sizeof(ofp_header_t));
		break;
	case OFPT_FEATURES_REQUEST:
		printf("----------------------------------\nrecv feature request\n");
		port_ccb->event = E_FEATURE_REQUEST;
		memcpy(&(port_ccb->ofp_header),mu,sizeof(ofp_header_t));
		break;
	case OFPT_MULTIPART_REQUEST:
		printf("----------------------------------\nrecv multipart\n");
		port_ccb->event = E_MULTIPART_REQUEST;
		memcpy(&(port_ccb->ofp_multipart),mu,sizeof(ofp_multipart_t));
		break;
	case OFPT_ECHO_REPLY:
		port_ccb->event = E_ECHO_REPLY;
		printf("----------------------------------\nrecv echo reply\n");
		break;
	case OFPT_FLOW_MOD:
		port_ccb->event = E_OTHERS;
		printf("----------------------------------\nrecv flow mod\n");
		PRINT_MESSAGE(mu, mulen);
		break;
	case OFPT_PACKET_OUT:
		port_ccb->event = E_OTHERS;
		printf("----------------------------------\nrecv packet out\n");
		PRINT_MESSAGE(mu, mulen);
		break;
	default:
		break;
	}
	
	return TRUE;
}


/*============================== ENCODING ===============================*/

void OFP_encode_packet_in(void) {
	printf("----------------------------------\nencode packet in\n");
}
#if 0
/*************************************************************************
 * OFP_encode_frame
 *
 * input : 
 *         omsg - ofp msg structure
 * output: mu - for buffer encoded message
 *         mulen - total omsg packet length after encoding.
 *************************************************************************/
void OFP_encode_hello(tOFP_MSG *omsg, U8 *mu, U16 *mulen)
{
	U8	*mp,i;
	
	mp = mu;
	memcpy(mp,ofp_da_mac,MAC_ADDR_LEN); /* from 2nd to 1st */
	mp += MAC_ADDR_LEN; /* DA */

	memcpy(mp,g_loc_mac,MAC_ADDR_LEN);
	mp += MAC_ADDR_LEN; /* SA = ??? */
	
	#if 0
	mp = ENCODE_U16(mp,0x8100); /* pre-2-bytes of VLAN TAG */
	mp = ENCODE_U16(mp,vid); /* post-2-bytes of VLAN TAG */
	#endif
	
	//ethernet type
	mp = ENCODE_U16(mp,(U16)ETH_TYPE_OFP);

	//ofp message	
	mp += ENCODE_OFP_TLVs(omsg->tlvs, mp);
	
	*mulen = mp - mu;
	if (*mulen < MIN_FRAME_SIZE){
		for(i=*mulen; i<MIN_FRAME_SIZE; i++)  *mp++ = 0;
		*mulen = MIN_FRAME_SIZE;  
	}
	//PRINT_MESSAGE(mu,*mulen);
}
#endif