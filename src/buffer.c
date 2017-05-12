#include "../inc/roach/buffer.h"

char *buffer_to_str(buffer_t *buf)
{
    char *ret = calloc(1, buf->used + 1);
    if(!ret)
    {
        debugf("%s\n", "calloc() error");
        return NULL;
    }
    memmove(ret, buf->data, buf->used);
    return ret;
}

buffer_t *buffer_create(void)
{
    return (buffer_t*)calloc(1, sizeof(buffer_t));
}

status_t buffer_append(buffer_t *buf, const void *data, size_t size)
{
    const size_t orig_used = buf->used;
    buf->used += size;

    if(buf->allocated <= buf->used)
    {
        buf->allocated = ALIGN(buf->used, BUFFER_CHUNK);
        buf->data = realloc(buf->data, buf->allocated);
        if(!buf->data)
        {
            debugf("%s\n", "realloc() error.");
            return FAILURE;
        }
    }
    memmove(buf->data + orig_used, data, size);
    return SUCCESS;
}

status_t buffer_append_str(buffer_t *buf, const char *str)
{
    return buffer_append(buf, (const void*)str, strlen(str));
}

void buffer_destroy(buffer_t **bufferPtr)
{
    buffer_t *buffer = *bufferPtr;
    if(buffer)
    {
        free(buffer->data);
        free(buffer);
    }
}
