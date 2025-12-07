#ifndef __JPEG_RTP_H
#define __JPEG_RTP_H


#include <stdint.h>
#include <arpa/inet.h>

// --------------------------------------------------
// JPEG RTP PAYLOAD HEADER (FOLLOW RFC 2435)
// --------------------------------------------------

struct jpeg_header_t {
	uint8_t type_specific;
	uint8_t fragment_offset[3];
	
	uint8_t type;
	uint8_t q;
	uint8_t width;
	uint8_t height;
} __attribute__((packed));


/*
 * HELPER FUNCTIONS FOR JPEG_HEADER_T
 */

static inline void set_jpeg_type_specific(struct jpeg_header_t* h, uint8_t val) {
	h->type_specific = val;
}
static inline uint8_t get_jpeg_type_specific(const struct jpeg_header_t h) {
	return h.type_specific;
}

static inline void set_jpeg_fragment_offset(struct jpeg_header_t* h, uint32_t offset) {
	// store little-endian value of 'offset' as big-endian
	h->fragment_offset[0] = (offset >> 16) & 0xff;
	h->fragment_offset[1] = (offset >> 8) & 0xff;
	h->fragment_offset[2] = offset & 0xff;
}
static inline uint32_t get_jpeg_fragment_offset(const struct jpeg_header_t h) {
	// return little-endian value
	return ((uint32_t)h.fragment_offset[0] << 16) |
		((uint32_t)h.fragment_offset[1] << 8) |
		((uint32_t)h.fragment_offset[2]);
}

static inline void set_jpeg_type(struct jpeg_header_t* h, uint8_t type) {
	h->type = type;
}
static inline uint8_t get_jpeg_type(const struct jpeg_header_t h) {
	return h.type;
}

static inline void set_jpeg_q(struct jpeg_header_t* h, uint8_t q) {
	h->q = q;
}
static inline uint8_t get_jpeg_q(const struct jpeg_header_t h) {
	return h.q;
}

static inline void set_jpeg_width(struct jpeg_header_t* h, uint8_t width) {
	h->width = width;
}
static inline uint8_t get_jpeg_width(const struct jpeg_header_t h) {
	return h.width;
}

static inline void set_jpeg_height(struct jpeg_header_t* h, uint8_t height) {
	h->height = height;
}
static inline uint8_t get_jpeg_height(const struct jpeg_header_t h) {
	return h.height;
}

#endif
