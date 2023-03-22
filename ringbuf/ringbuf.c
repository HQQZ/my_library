#include "ringbuf.h"
#include <string.h>

#ifndef DYNAMIC_MALLOC
static size_t head_sum = 0;
static ringbuf_t rb_head[RINGBUF_HEAD];
static size_t buff_sum = 0;
static uint8_t rb_buff[RINGBUF_SIZE];
#endif

/**
 * @brief 创建一个新的ringbuf
 *
 * @param length: 缓冲区长度
 * @return ringbuf_t 创建的ringbuf指针
 */
ringbuf_t * ringbuf_new(size_t length)
{
    #ifdef DYNAMIC_MALLOC
    ringbuf_t *rb = MALLOC(sizeof(struct ringbuf_s));
    #else
    if(head_sum > RINGBUF_HEAD)
        return NULL;
    ringbuf_t *rb = rb_head + head_sum;
    head_sum++;
    #endif
    if(rb)
    {
        rb->size = length;
        #ifdef DYNAMIC_MALLOC
        rb->buf = MALLOC(rb->size);
        #else
        if((length + buff_sum) > RINGBUF_SIZE)
            return NULL;
        rb->buf = rb_buff + buff_sum;
        buff_sum += rb->size;
        #endif
        if (rb->buf)
            ringbuf_reset(rb);
        else {
            #ifdef DYNAMIC_MALLOC
            free(rb);
            #endif
            return NULL;
        }
    }
    return rb;
}

size_t ringbuf_buffer_size(const ringbuf_t *rb)
{
    return rb->size;
}

void ringbuf_reset(ringbuf_t *rb)
{
    rb->head = rb->tail = rb->buf;
}

void ringbuf_free(ringbuf_t *rb)
{
    if(!rb)
        return;
    #ifdef DYNAMIC_MALLOC
    FREE(rb->buf);
    FREE(rb);
    #endif
}

static const uint8_t * ringbuf_end(const ringbuf_t *rb)
{
    return (rb->buf + (ringbuf_buffer_size(rb) - 1));
}

size_t ringbuf_bytes_free(const ringbuf_t *rb)
{
    if (rb->head > rb->tail)
        return (rb->head - rb->tail);
    else
        return (ringbuf_buffer_size(rb) - (rb->tail - rb->head));
}

size_t ringbuf_bytes_used(const ringbuf_t *rb)
{
    return (ringbuf_buffer_size(rb) - ringbuf_bytes_free(rb));
}

int ringbuf_is_full(const ringbuf_t *rb)
{
    return ringbuf_bytes_free(rb) == 0;
}

int ringbuf_is_empty(const ringbuf_t *rb)
{
    return (ringbuf_bytes_free(rb) == ringbuf_buffer_size(rb));
}

const void * ringbuf_tail(const ringbuf_t *rb)
{
    return rb->tail;
}

const void * ringbuf_head(const ringbuf_t *rb)
{
    return rb->head;
}

uint8_t ringbuf_write(ringbuf_t *rb, const uint8_t *buf, size_t length)
{
    size_t bytes_free, sequential_bytes;

    if(rb == NULL || ringbuf_is_full(rb))
        return 0;

    bytes_free = ringbuf_bytes_free(rb);
    if(length > bytes_free)
        length = bytes_free;

    if(rb->head > rb->tail)
    {
        memcpy(rb->tail, buf, length);
        rb->tail += length;
    }
    else
    {
        sequential_bytes = ringbuf_end(rb) - rb->tail + 1;
        if(sequential_bytes >= length)
        {
            memcpy(rb->tail, buf, length);
            rb->tail += length;
        }
        else
        {
            memcpy(rb->tail, buf, sequential_bytes);
            rb->tail = rb->buf;
            memcpy(rb->tail, buf + sequential_bytes, length - sequential_bytes);
            rb->tail += (length - sequential_bytes);
        }
    }
    return 1;
}

uint8_t ringbuf_read(ringbuf_t *rb, uint8_t *buf, size_t length)
{
    size_t bytes_used, sequential_bytes;

    if(rb == NULL || ringbuf_is_empty(rb))
        return 0;

    bytes_used = ringbuf_bytes_used(rb);
    if(length > bytes_used)
        length = bytes_used;

    if(rb->head >= rb->tail)
    {
        sequential_bytes = ringbuf_end(rb) - rb->head + 1;
        if(sequential_bytes >= length)
        {
            memcpy(buf, rb->head, length);
            rb->head += length;
        }
        else
        {
            memcpy(buf, rb->head, sequential_bytes);
            rb->head = rb->buf;
            memcpy(buf + sequential_bytes, rb->head, length - sequential_bytes);
            rb->head += (length - sequential_bytes);
        }
    }
    else
    {
        memcpy(buf, rb->head, length);
        rb->head += length;
    }
    return 1;
}

