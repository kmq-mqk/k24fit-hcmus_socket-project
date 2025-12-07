#ifndef __RTP_H
#define __RTP_H

#include <stdint.h>
#include <arpa/inet.h>

struct rtp_header_t {
//	uint8_t version:2;
//	uint8_t padding:1;
//	uint8_t extension:1;
//	uint8_t csrc_count:4;	// will be 0
	uint8_t v_p_x_cc;

//	uint8_t marker:1;
//	uint8_t payload_type:7;
	uint8_t m_pt;

	uint16_t seq;
	uint32_t timestamp;
	uint32_t ssrc;
	// no csrc
} __attribute__((packed));

struct jpeg_header_t {
	uint8_t type_specific;
	uint8_t fragment_offset[3];
	
	uint8_t type;
	uint8_t q;
	uint8_t width;
	uint8_t height;
} __attribute__((packed));

#endif
