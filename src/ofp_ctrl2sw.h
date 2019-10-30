#ifndef _OFP_CTRL2SW_H_
#define _OFP_CTRL2SW_H_

#include "ofp_common.h"

typedef struct ofp_multipart { 
	struct ofp_header ofp_header; 
	uint16_t type;
	uint16_t flags;
	uint8_t pad[4]; 
	uint8_t body[0];
}ofp_multipart_t;
OFP_ASSERT(sizeof(struct ofp_multipart) == 16);

/* Send packet (controller -> datapath). */ 
typedef struct ofp_packet_out {
    struct ofp_header header; 
    uint32_t buffer_id; /* ID assigned by datapath (OFP_NO_BUFFER if none). */
    uint32_t in_port; /* Packet's input port or OFPP_CONTROLLER. */
    uint16_t actions_len; /* Size of action array in bytes. */
    uint8_t pad[6];
    struct ofp_action_header actions[0]; /* Action list. */
    /* uint8_t data[0]; */ /* Packet data. The length is inferred
                                from the length field in the header. 
                                (Only meaningful if buffer_id == -1.) */
}ofp_packet_out_t;
OFP_ASSERT(sizeof(struct ofp_packet_out) == 24);

/* Switch features. */ 
typedef struct ofp_switch_features {
    struct ofp_header ofp_header;
    uint64_t datapath_id; /* Datapath unique ID. The lower 48-bits are for a MAC 
        address, while the upper 16-bits are implementer-defined. */
    uint32_t n_buffers; /* Max packets buffered at once. */
    uint8_t n_tables; /* Number of tables supported by datapath. */
    uint8_t auxiliary_id; /* Identify auxiliary connections */
    uint8_t pad[2]; /* Align to 64-bits. */
    /* Features. */
    uint32_t capabilities; /* Bitmap of support "ofp_capabilities". */
    uint32_t reserved;
}ofp_switch_features_t;
OFP_ASSERT(sizeof(struct ofp_switch_features) == 32);

#endif