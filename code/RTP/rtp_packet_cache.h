#ifndef RTP_PACKET_CACHE_H
#define RTP_PACKET_CACHE_H

#include <stdint.h>
#include <stdlib.h>
#include "rtp.h"
#include "jpeg_rtp.h"

typedef struct rtp_packet_cache_entry {
    uint16_t seq;
    uint32_t timestamp;
    uint8_t *packet; 
    size_t   pkt_size;
    struct rtp_packet_cache_entry *next;
} rtp_packet_cache_entry_t;

typedef struct rtp_packet_cache {
    rtp_packet_cache_entry_t *head;
    pthread_mutex_t lock;
} rtp_packet_cache_t;

void rtp_cache_init(rtp_packet_cache_t *c);
void rtp_cache_destroy(rtp_packet_cache_t *c);
int rtp_cache_store_packet(rtp_packet_cache_t *c, const uint8_t *pkt, size_t pkt_size);
rtp_packet_cache_entry_t* rtp_cache_find(rtp_packet_cache_t *c, uint16_t seq); 
void rtp_cache_remove_older_than(rtp_packet_cache_t *c, size_t keep_count);

uint8_t* rtp_extract_jpeg_payload(const uint8_t *pkt, size_t pkt_size, size_t *out_size);

#endif
