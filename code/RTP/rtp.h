#ifndef __RTP_H
#define __RTP_H

#include <stdint.h>
#include <arpa/inet.h>

// --------------------------------------------------
// RTP HEADER (FOLLOW RTP 1889)
// --------------------------------------------------

struct rtp_header_t {
	uint8_t v_p_x_cc;
	uint8_t m_pt;

	uint16_t seq;
	uint32_t timestamp;
	uint32_t ssrc;
	// no csrc
} __attribute__((packed));


/*
 * HELPER FUNCTIONS FOR RTP_HEADER_T
 */

static inline void set_rtp_v(struct rtp_header_t* h, unsigned char v) {
	h->v_p_x_cc = (h->v_p_x_cc & 0x3f) | ((v & 0x03) << 6);
}
static inline unsigned char get_rtp_v(const struct rtp_header_t h) {
	return (h.v_p_x_cc >> 6) & 0x03;
}

static inline void set_rtp_p(struct rtp_header_t* h, unsigned char p) {
	h->v_p_x_cc = (h->v_p_x_cc & 0xdf) | ((p & 0x01) << 5);
}
static inline unsigned char get_rtp_p(const struct rtp_header_t h) {
	return (h.v_p_x_cc >> 5) & 0x01;
}

static inline void set_rtp_x(struct rtp_header_t* h, unsigned char x) {
	h->v_p_x_cc = (h->v_p_x_cc & 0xef) | ((x & 0x01) << 4); 
}
static inline unsigned char get_rtp_x(const struct rtp_header_t h) {
	return (h.v_p_x_cc >> 4) & 0x01;
}

static inline void set_rtp_cc(struct rtp_header_t* h, unsigned char cc) {
	h->v_p_x_cc = (h->v_p_x_cc & 0xf0) | (cc & 0x0f);
}
static inline unsigned char get_rtp_cc(const struct rtp_header_t h) {
	return h.v_p_x_cc & 0x0f;
}


static inline void set_rtp_m(struct rtp_header_t* h, unsigned char m) {
	h->m_pt = (h->m_pt & 0x7f) | ((m & 0x01) << 7);
}
static inline unsigned char get_rtp_m(const struct rtp_header_t h) {
	return (h.m_pt >> 7) & 0x01;
}

static inline void set_rtp_pt(struct rtp_header_t* h, unsigned char pt) {
	h->m_pt = (h->m_pt & 0x80) | (pt & 0x7f);
}
static inline unsigned char get_rtp_pt(const struct rtp_header_t h) {
	return h.m_pt & 0x80;
}


static inline void set_rtp_seq(struct rtp_header_t* h, uint16_t seq) {
	h->seq = ntohs(seq);
}
static inline uint16_t get_rtp_seq(const struct rtp_header_t h) {
	return htons(h.seq);
}

static inline void set_rtp_timestamp(struct rtp_header_t* h, uint32_t ts) {
	h->timestamp = ntohl(ts);
}
static inline uint32_t get_rtp_timestamp(const struct rtp_header_t h) {
	return htonl(h.timestamp);
}

static inline void set_rtp_ssrc(struct rtp_header_t* h, uint32_t ssrc) {
	h->ssrc = ntohl(ssrc);
}
static inline uint32_t get_rtp_ssrc(const struct rtp_header_t h) {
	return htonl(h.ssrc);
}


#endif
