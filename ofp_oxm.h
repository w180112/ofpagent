
#ifndef _OFP_OXM_H_
#define _OFP_OXM_H_

enum ofp_oxm_class {
    OFPXMC_NXM_0 = 0x0000, /* Backward compatibility with NXM */
    OFPXMC_NXM_1 = 0x0001, /* Backward compatibility with NXM */
    OFPXMC_OPENFLOW_BASIC = 0x8000, /* Basic class for OpenFlow */
    OFPXMC_EXPERIMENTER = 0xFFFF, /* Experimenter class */
};

typedef struct ofp_oxm_header {
    uint16_t oxm_class;
    union oxm_header {
		uint16_t oxm_value;
		struct oxm_bit {
			#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
            uint16_t oxm_length:8;
            uint16_t oxm_hasmask:1;
			uint16_t oxm_field:7;
			#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
			uint16_t oxm_field:7;
			uint16_t oxm_hasmask:1;
			uint16_t oxm_length:8;
			#endif
		}oxm_struct;
	}oxm_union;
}ofp_oxm_header_t;

#endif