#ifndef FRAME_CACHE_H
#define FRAME_CACHE_H

#include <stdint.h>
#include <stddef.h>
#include <sys/time.h>
#include <pthread.h>

#define MAX_JITTER_MS 200
#define MIN_JITTER_MS 50

typedef struct frame_t {
    uint16_t seq;          
    uint32_t timestamp;   
    uint8_t *data;        
    size_t   size;
    struct timeval arrival;
    struct frame_t *next;
} frame_t;

typedef struct frame_cache_t {
    frame_t *head;   
    pthread_mutex_t lock;
    pthread_cond_t  cond;
    size_t count;
    int jitter_ms; 
    frame_t *last_frame_on_pause; 
    int paused;
} frame_cache_t;

void frame_cache_init(frame_cache_t *c, int jitter_ms);
void frame_cache_destroy(frame_cache_t *c);

int frame_cache_push(frame_cache_t *c, uint16_t seq, uint32_t timestamp, const uint8_t *data, size_t size);

frame_t* frame_cache_pop_playable(frame_cache_t *c);

void frame_cache_set_pause(frame_cache_t *c, int pause);

void frame_free(frame_t *f);

#endif
