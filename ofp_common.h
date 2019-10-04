#ifndef _OPENFLOW_COMMON_H_
#define _OPENFLOW_COMMON_H_

#include <stdint.h>

#define OFP_ETH_ALEN 6
#define OFP_MAX_PORT_NAME_LEN 16
#define OFP_ASSERT(EXPR)                                                \
        extern int (*build_assert(void))[ sizeof(struct {               \
                    unsigned int build_assert_failed : (EXPR) ? 1 : -1; })]
enum ofp_version {
    OFP10_VERSION = 0x01,
    OFP11_VERSION = 0x02,
    OFP12_VERSION = 0x03,
    OFP13_VERSION = 0x04,
    OFP14_VERSION = 0x05,
    OFP15_VERSION = 0x06
};

typedef struct ofp_header {
	uint8_t version; /* OFP_VERSION. */
	uint8_t type; /* One of the OFPT_ constants. */
	uint16_t length; /* Length including this ofp_header. */
	uint32_t xid; /* Transaction id associated with this packet.
	            Replies use the same id as was in the request to facilitate pairing. */
}ofp_header_t;
OFP_ASSERT(sizeof(struct ofp_header) == 8);

struct ofp_multipart { 
	struct ofp_header ofp_header; 
	uint16_t type;
	uint16_t flags;
	uint8_t pad[4]; 
	uint8_t body[0];
}ofp_multipart_t;
OFP_ASSERT(sizeof(struct ofp_multipart) == 8);

struct ofp_port {
    uint32_t port_no;
    uint8_t pad[4];
    uint8_t hw_addr[OFP_ETH_ALEN]; 
    uint8_t pad2[2]; /* Align to 64 bits. */
    char name[OFP_MAX_PORT_NAME_LEN]; /* Null-terminated */
    uint32_t config; /* Bitmap of OFPPC_* flags. */
    uint32_t state; /* Bitmap of OFPPS_* flags. */
    /* Bitmaps of OFPPF_* that describe features. All bits zeroed if * unsupported or unavailable. */
    uint32_t curr; /* Current features. */
    uint32_t advertised; /* Features being advertised by the port. */
    uint32_t supported; /* Features supported by the port. */
    uint32_t peer; /* Features advertised by peer. */
    uint32_t curr_speed; /* Current port bitrate in kbps. */
    uint32_t max_speed; /* Max port bitrate in kbps */
};
OFP_ASSERT(sizeof(struct ofp_port) == 64);

enum ofp_multipart_types {
    OFPMP_DESC = 0,
    OFPMP_FLOW = 1,
    OFPMP_AGGREGATE = 2,
    OFPMP_TABLE = 3,
    OFPMP_PORT_STATS = 4,
    OFPMP_QUEUE = 5,
    OFPMP_GROUP = 6,
    OFPMP_GROUP_DESC = 7,
    OFPMP_GROUP_FEATURES = 8,
    OFPMP_METER = 9,
    OFPMP_METER_CONFIG = 10,
    OFPMP_METER_FEATURES = 11,
    OFPMP_TABLE_FEATURES = 12,
    OFPMP_PORT_DESC = 13,
    OFPMP_EXPERIMENTER = 0xffff 
};


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

/* Capabilities supported by the datapath. */ 
enum ofp_capabilities {
    OFPC_FLOW_STATS = 1 << 0, /* Flow statistics. */
    OFPC_TABLE_STATS = 1 << 1, /* Table statistics. */
    OFPC_PORT_STATS  = 1 << 2, /* Port statistics. */
    OFPC_GROUP_STATS  = 1 << 3, /* Group statistics. */
    OFPC_IP_REASM  = 1 << 5, /* Can reassemble IP fragments. */
    OFPC_QUEUE_STATS  = 1 << 6, /* Queue statistics. */
    OFPC_PORT_BLOCKED = 1 << 8, /* Switch will block looping ports. */
};

#endif