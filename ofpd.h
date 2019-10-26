/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\
  OFPD.H

     For ofp detection

  Designed by Dennis Tseng on Apr 26, 2016
/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

#include "ofp_common.h"

#define OFP_Q_KEY				0x0b00
#define ETH_MTU					1500
#define TEST_PORT_ID			1

#define	MIN_FRAME_SIZE			64
#define	MAX_QUE_EVT_CNT			(MBOX_QUE_SIZE/2)
#define _PBM(port)				(1<<(port-1))

#define MAX_USER_PORT_NUM		44
#define MAX_OFP_QUERY_NUM		10
#define DEF_QUERY_INTERVAL		2
#define	OFP_MAX_OPT_TLV_NUM	10
#define	OFP_MAX_SUB_OPT_TLV_NUM 20

#define FWD_STD_802_1Q			1
#define FWD_REFLECTIVE_RELAY	1
#define CAP_VSI_DISCOV_PROTO	1
#define CAP_802_1X_AUTH_REQ		1

typedef struct {
	U8		subt;
	U16		len;
	U8		value[255];
} tSUB_VAL;

//========= system capability ===========
typedef struct {
	U16		cap_map;
	U16		en_map;
} tSYS_CAP;

//========= management address ===========
typedef struct {
	U8		addr_strlen; //addr_subt + addr[]
	U8		addr_subt;
	U8		addr[31];
	
	U8		if_subt;
	U32		if_no;
	
	U8		oid_len;
	U32		oids[128];
} tMNG_ADDR;

//========= The structure of EVB TLV ===========
typedef struct _EVB_CAP {
	struct _FWD_MODE { /*_LIT_ENDIAN */  
		U8	rsvd: 6;
		U8	rr  : 1;
		U8	std : 1;
	} fwd_mode;
	
	struct _CAP {
		U8	vsi_discov : 1;
		U8	auth: 2;
		U8	rsvd: 5;
	} cap;
} EVB_CAP_t;
		 
typedef struct _OFP_EVB_TLV {
	EVB_CAP_t	evb_cap;
	EVB_CAP_t	evb_cur_config;
	U16 		vsi_num_support;
	U16 		vsi_num_config;
} EVB_VSI_t;

//========= The structure of port ===========
typedef struct {
	BOOL		enable;
	U8 			state;
	U8			query_cnt;
	U16			port;

	U32			imsg_cnt;
	U32			omsg_cnt;
	U32			err_imsg_cnt;	
	
	tSUB_VAL	port_id;
		
	U32			ttl;
	char		port_desc[80];
	char		sys_name[80];
	char		sys_desc[255];
	
	tSYS_CAP	sys_cap;
	tMNG_ADDR  	mng_addr;
	EVB_VSI_t	evb;

	int 		sockfd;
	U16			event;
	ofp_header_t ofp_header;
	ofp_multipart_t ofp_multipart;
} tOFP_PORT;

extern BOOL			ofp_opt_tlvs[];
extern BOOL			ofp_sub_opt_tlvs[];
extern U8	 		g_loc_mac[]; //system mac addr -- global variable
extern tOFP_PORT	ofp_ports[];
extern tIPC_ID 		ofpQid;
extern U32			ofp_interval;
extern U8			ofp_max_msg_per_query;

/*-----------------------------------------
 * Queue between IF driver and daemon
 *----------------------------------------*/
typedef struct {
	U16  			type;
	U8          	refp[ETH_MTU];
	int	        	len;
}tOFP_MBX;

/*-----------------------------------------
 * msg from dp
 *----------------------------------------*/
typedef struct {
	U8  			type;
	int 			sockfd;
	char          	buffer[ETH_MTU];
}tOFP_MSG;

typedef enum {
	/* Immutable messages. */
	OFPT_HELLO,
	OFPT_ERROR,
	OFPT_ECHO_REQUEST,
	OFPT_ECHO_REPLY,
	OFPT_EXPERIMENTER,
	/* Switch configuration messages */
	OFPT_FEATURES_REQUEST,
	OFPT_FEATURES_REPLY,
	OFPT_GET_CONFIG_REQUEST,
	OFPT_GET_CONFIG_REPLY, 
	OFPT_SET_CONFIG,
	/* Asynchronous messages. */
	OFPT_PACKET_IN,
	OFPT_FLOW_REMOVED, 
	OFPT_PORT_STATUS,
	/* Controller command messages. */
	OFPT_PACKET_OUT,
	OFPT_FLOW_MOD,
	OFPT_GROUP_MOD,
	OFPT_PORT_MOD,
	OFPT_TABLE_MOD,
	/* Multipart messages. */ 
	OFPT_MULTIPART_REQUEST, 
	OFPT_MULTIPART_REPLY,
	/* Barrier messages. */ 
	OFPT_BARRIER_REQUEST,
	OFPT_BARRIER_REPLY,
	/* Queue Configuration messages. */
	FPT_QUEUE_GET_CONFIG_REQUEST,
	OFPT_QUEUE_GET_CONFIG_REPLY,
	/* Controller role change request messages. */
	OFPT_ROLE_REQUEST,
	OFPT_ROLE_REPLY,
	/* Asynchronous message configuration. */
	OFPT_GET_ASYNC_REQUEST,
	OFPT_GET_ASYNC_REPLY,
	OFPT_SET_ASYNC,
	/* Meters and rate limiters configuration messages. */
	OFPT_METER_MOD,
}OFPT_t;