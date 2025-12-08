#include "video_stream_cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int video_stream_cache_load(video_stream_cache *v, const char *filename) {
    if (!v) return -1;
    FILE *f = fopen(filename, "rb");
    if (!f) return -1;
    v->frames = NULL;
    v->count = 0;

    while (1) {
        uint8_t size_hdr[5];
        if (fread(size_hdr, 1, 5, f) != 5) break;
        uint32_t size = (size_hdr[0] << 24) | (size_hdr[1] << 16) | (size_hdr[2] << 8) | (size_hdr[3]);
        size |= (uint32_t)size_hdr[4]; 
        if (size == 0) break;
        uint8_t *buf = malloc(size);
        if (!buf) { fclose(f); return -1; }
        if (fread(buf, 1, size, f) != size) { free(buf); break; }
        v->frames = realloc(v->frames, sizeof(vs_frame) * (v->count + 1));
        v->frames[v->count].data = buf;
        v->frames[v->count].size = size;
        v->count++;
    }
    fclose(f);
    return 0;
}

void video_stream_cache_free(video_stream_cache *v) {
    if (!v) return;
    for (size_t i = 0; i < v->count; i++) {
        free(v->frames[i].data);
    }
    free(v->frames);
    v->frames = NULL;
    v->count = 0;
}
