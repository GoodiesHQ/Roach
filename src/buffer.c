#include "../inc/roach/buffer.h"

buffer_t *buffer_create(void)
{
    return (buffer_t*)calloc(1, sizeof(buffer_t));
}

void buffer_append(buffer_t *buf, const void *data, size_t size)
{
    size_t orig_used = buf->used;
    buf->used += size;

    if(buf->allocated <= buf->used)
    {
        buf->allocated = ALIGN(buf->used, BUFFER_CHUNK);
        buf->data = realloc(buf->data, buf->allocated);
    }

    memcpy(buf->data + orig_used, data, size);
}

void buffer_append_str(buffer_t *buf, const char *str)
{
    buffer_append(buf, (const void*)str, strlen(str));
}

void buffer_destroy(buffer_t **bufferPtr)
{
    buffer_t *buffer = *bufferPtr;
    free(buffer->data);
    free(buffer);
}
