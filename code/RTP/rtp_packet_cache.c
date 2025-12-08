#include "rtp_packet_cache.h"
#include <pthread.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>

void rtp_cache_init(rtp_packet_cache_t *c) {
    c->head = NULL;
    pthread_mutex_init(&c->lock, NULL);
}

void rtp_cache_destroy(rtp_packet_cache_t *c) {
    pthread_mutex_lock(&c->lock);
    rtp_packet_cache_entry_t *it = c->head;
    while (it) {
        rtp_packet_cache_entry_t *n = it->next;
        if (it->packet) free(it->packet);
        free(it);
        it = n;
    }
    c->head = NULL;
    pthread_mutex_unlock(&c->lock);
    pthread_mutex_destroy(&c->lock);
}

int rtp_cache_store_packet(rtp_packet_cache_t *c, const uint8_t *pkt, size_t pkt_size) {
    if (pkt_size < sizeof(struct rtp_header_t)) return -1;
    struct rtp_header_t hdr;
    memcpy(&hdr, pkt, sizeof(hdr));
    uint16_t seq = get_rtp_seq(hdr); 

    rtp_packet_cache_entry_t *entry = malloc(sizeof(*entry));
    if (!entry) return -1;
    entry->seq = seq;
    entry->pkt_size = pkt_size;
    entry->packet = malloc(pkt_size);
    if (!entry->packet) { free(entry); return -1; }
    memcpy(entry->packet, pkt, pkt_size);
    entry->timestamp = get_rtp_timestamp(hdr);
    entry->next = NULL;

    pthread_mutex_lock(&c->lock);
    rtp_packet_cache_entry_t **pp = &c->head;
    while (*pp) {
        if ((*pp)->seq == seq) {
            rtp_packet_cache_entry_t *old = *pp;
            entry->next = old->next;
            free(old->packet);
            free(old);
            *pp = entry;
            pthread_mutex_unlock(&c->lock);
            return 0;
        }
        pp = &(*pp)->next;
    }
    entry->next = c->head;
    c->head = entry;
    pthread_mutex_unlock(&c->lock);
    return 0;
}

rtp_packet_cache_entry_t* rtp_cache_find(rtp_packet_cache_t *c, uint16_t seq) {
    pthread_mutex_lock(&c->lock);
    rtp_packet_cache_entry_t *it = c->head;
    while (it) {
        if (it->seq == seq) {
            rtp_packet_cache_entry_t *copy = malloc(sizeof(*copy));
            copy->seq = it->seq;
            copy->timestamp = it->timestamp;
            copy->pkt_size = it->pkt_size;
            copy->packet = malloc(it->pkt_size);
            memcpy(copy->packet, it->packet, it->pkt_size);
            copy->next = NULL;
            pthread_mutex_unlock(&c->lock);
            return copy;
        }
        it = it->next;
    }
    pthread_mutex_unlock(&c->lock);
    return NULL;
}

void rtp_cache_remove_older_than(rtp_packet_cache_t *c, size_t keep_count) {
    pthread_mutex_lock(&c->lock);
    rtp_packet_cache_entry_t *it = c->head;
    size_t i = 0;
    rtp_packet_cache_entry_t *last = NULL;
    while (it && i < keep_count) {
        last = it;
        it = it->next;
        i++;
    }
    if (last) last->next = NULL;
    while (it) {
        rtp_packet_cache_entry_t *n = it->next;
        free(it->packet);
        free(it);
        it = n;
    }
    pthread_mutex_unlock(&c->lock);
}

uint8_t* rtp_extract_jpeg_payload(const uint8_t *pkt, size_t pkt_size, size_t *out_size) {
    if (pkt_size <= sizeof(struct rtp_header_t)) { *out_size = 0; return NULL; }
    const uint8_t *p = pkt + sizeof(struct rtp_header_t);
    size_t left = pkt_size - sizeof(struct rtp_header_t);
    if (left < sizeof(struct jpeg_header_t)) {
        uint8_t *buf = malloc(left);
        memcpy(buf, p, left);
        *out_size = left;
        return buf;
    }
    struct jpeg_header_t jh;
    memcpy(&jh, p, sizeof(jh));
    size_t payload_offset = sizeof(struct rtp_header_t) + sizeof(struct jpeg_header_t);
    if (pkt_size <= payload_offset) { *out_size = 0; return NULL; }
    size_t payload_size = pkt_size - payload_offset;
    uint8_t *buf = malloc(payload_size);
    memcpy(buf, pkt + payload_offset, payload_size);
    *out_size = payload_size;
    return buf;
}
