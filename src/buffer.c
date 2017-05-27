#include "../inc/roach/buffer.h"

// converts a buffer to a string and returns a copy. Does not free the buffer.
char *buffer_to_str(const buffer_t *buf)
{
    char *ret;
    if((ret = calloc(1, buf->used + 1)) == NULL)
    {
        debugf(DBG_CRIT, "%s\n", "calloc() error");
        return NULL;
    }
    memmove(ret, buf->data, buf->used);
    return ret;
}

// creates a buffer object. Returns NULL on failure;
buffer_t *buffer_create(void)
{
    buffer_t *buf;
    if((buf = calloc(1, sizeof(buffer_t))) == NULL)
    {
        debugf(DBG_CRIT, "%s\n", "calloc() error");
        return NULL;
    }
    return buf;
}

// appends data to a buffer with a given size. Buffer dynamically grows as needed.
status_t buffer_append(buffer_t *buf, const void *data, const size_t size)
{
    char *tmp;
    const size_t orig_used = buf->used, orig_allocated = buf->allocated;
    buf->used += size;

    if(buf->allocated <= buf->used)
    {
        buf->allocated = ALIGN(buf->used, BUFFER_CHUNK); // aligns to the nearest multiple of BUFFER_CHUNK
        if((tmp = realloc(buf->data, buf->allocated)) == NULL)
        {
            buf->allocated = orig_allocated;
            debugf(DBG_CRIT, "%s\n", "realloc() error.");
            return FAILURE;
        }
        buf->data = tmp;
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
        //*bufferPtr = NULL; // not sure if this is best practice.
    }
}
