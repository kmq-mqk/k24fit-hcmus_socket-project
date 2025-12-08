#define _POSIX_C_SOURCE 199309L
#include "frame_cache.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

static long timeval_to_ms(const struct timeval *tv) {
    return tv->tv_sec * 1000 + tv->tv_usec / 1000;
}

void frame_cache_init(frame_cache_t *c, int jitter_ms) {
    c->head = NULL;
    pthread_mutex_init(&c->lock, NULL);
    pthread_cond_init(&c->cond, NULL);
    c->count = 0;
    if (jitter_ms < MIN_JITTER_MS) jitter_ms = MIN_JITTER_MS;
    if (jitter_ms > MAX_JITTER_MS) jitter_ms = MAX_JITTER_MS;
    c->jitter_ms = jitter_ms;
    c->last_frame_on_pause = NULL;
    c->paused = 0;
}

void frame_cache_destroy(frame_cache_t *c) {
    pthread_mutex_lock(&c->lock);
    frame_t *it = c->head;
    while (it) {
        frame_t *n = it->next;
        frame_free(it);
        it = n;
    }
    if (c->last_frame_on_pause) frame_free(c->last_frame_on_pause);
    c->head = NULL;
    c->count = 0;
    pthread_mutex_unlock(&c->lock);
    pthread_mutex_destroy(&c->lock);
    pthread_cond_destroy(&c->cond);
}

void frame_free(frame_t *f) {
    if (!f) return;
    if (f->data) free(f->data);
    free(f);
}

int frame_cache_push(frame_cache_t *c, uint16_t seq, uint32_t timestamp, const uint8_t *data, size_t size) {
    frame_t *node = malloc(sizeof(frame_t));
    if (!node) return -1;
    node->seq = seq;
    node->timestamp = timestamp;
    node->data = malloc(size);
    if (!node->data) { free(node); return -1; }
    memcpy(node->data, data, size);
    node->size = size;
    gettimeofday(&node->arrival, NULL);
    node->next = NULL;

    pthread_mutex_lock(&c->lock);
    frame_t **pp = &c->head;
    while (*pp && (int16_t)((*pp)->seq - seq) < 0) { 
        pp = &(*pp)->next;
    }
    node->next = *pp;
    *pp = node;
    c->count++;
    pthread_cond_signal(&c->cond);
    pthread_mutex_unlock(&c->lock);
    return 0;
}

frame_t* frame_cache_pop_playable(frame_cache_t *c) {
    pthread_mutex_lock(&c->lock);
    while (c->head == NULL && !c->paused) {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_nsec += 50 * 1000000;
        if (ts.tv_nsec >= 1000000000) { ts.tv_sec += 1; ts.tv_nsec -= 1000000000; }
        pthread_cond_timedwait(&c->cond, &c->lock, &ts);
        if (c->head == NULL) {
            pthread_mutex_unlock(&c->lock);
            return NULL;
        }
    }

    if (c->paused) {
        frame_t *copy = NULL;
        if (c->last_frame_on_pause) {
            copy = malloc(sizeof(frame_t));
            memcpy(copy, c->last_frame_on_pause, sizeof(frame_t));
            copy->data = malloc(c->last_frame_on_pause->size);
            memcpy(copy->data, c->last_frame_on_pause->data, c->last_frame_on_pause->size);
            copy->next = NULL;
        }
        pthread_mutex_unlock(&c->lock);
        return copy;
    }

    struct timeval now;
    gettimeofday(&now, NULL);
    long arrived_ms = timeval_to_ms(&c->head->arrival);
    long now_ms = timeval_to_ms(&now);
    if (now_ms - arrived_ms >= c->jitter_ms) {
        frame_t *f = c->head;
        c->head = f->next;
        f->next = NULL;
        c->count--;
        pthread_mutex_unlock(&c->lock);
        return f;
    } else {
        pthread_mutex_unlock(&c->lock);
        return NULL;
    }
}

void frame_cache_set_pause(frame_cache_t *c, int pause) {
    pthread_mutex_lock(&c->lock);
    if (pause && !c->paused) {
        if (c->last_frame_on_pause) { frame_free(c->last_frame_on_pause); c->last_frame_on_pause = NULL; }
        if (c->head) {
            frame_t *f = c->head;
            frame_t *copy = malloc(sizeof(frame_t));
            memcpy(copy, f, sizeof(frame_t));
            copy->data = malloc(f->size);
            memcpy(copy->data, f->data, f->size);
            copy->next = NULL;
            c->last_frame_on_pause = copy;
        }
        c->paused = 1;
    } else if (!pause && c->paused) {
        c->paused = 0;
    }
    pthread_mutex_unlock(&c->lock);
}
