#ifndef VIDEO_STREAM_CACHE_H
#define VIDEO_STREAM_CACHE_H

#include <stdint.h>
#include <stddef.h>

typedef struct vs_frame {
    uint8_t *data;
    size_t size;
} vs_frame;

typedef struct video_stream_cache {
    vs_frame *frames;
    size_t count;
} video_stream_cache;

int video_stream_cache_load(video_stream_cache *v, const char *filename);
void video_stream_cache_free(video_stream_cache *v);

#endif