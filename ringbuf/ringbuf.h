#ifndef __RINGBUF_H__
#define __RINGBUF_H__

#include "main.h"
#include <stdlib.h>

// #define DYNAMIC_MALLOC

#ifdef DYNAMIC_MALLOC
#define MALLOC  malloc
#define FREE    free
#else
#define RINGBUF_HEAD    10
#define RINGBUF_SIZE    1000
#endif

struct ringbuf_s
{
    uint8_t *buf;
    uint8_t *head, *tail;
    size_t size;
};
typedef struct ringbuf_s ringbuf_t;

ringbuf_t * ringbuf_new(size_t length);
size_t ringbuf_buffer_size(const ringbuf_t *rb);
void ringbuf_reset(ringbuf_t *rb);
size_t ringbuf_bytes_free(const ringbuf_t *rb);
size_t ringbuf_bytes_used(const ringbuf_t *rb);
int ringbuf_is_full(const ringbuf_t *rb);
int ringbuf_is_empty(const ringbuf_t *rb);
const void * ringbuf_tail(const ringbuf_t *rb);
const void * ringbuf_head(const ringbuf_t *rb);
uint8_t ringbuf_write(ringbuf_t *rb, const uint8_t *buf, size_t length);
uint8_t ringbuf_read(ringbuf_t *rb, uint8_t *buf, size_t length);

#endif /* __RINGBUF_H__ */
