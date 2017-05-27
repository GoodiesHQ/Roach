#ifndef ROACH_BUFFER_H
#define ROACH_BUFFER_H

#include "./common.h"

#include <stdlib.h>
#include <string.h>

#ifndef BUFFER_CHUNK
#define BUFFER_CHUNK 256
#endif

typedef struct _buffer_t
{
    char *data;
    size_t allocated, used;
} buffer_t;

buffer_t *buffer_create(void);
char *buffer_to_str(const buffer_t *buf);
status_t buffer_append(buffer_t *buf, const void *data, const size_t size);
status_t buffer_append_str(buffer_t *buf, const char *str);
void buffer_destroy(buffer_t **bufferPtr);

#endif//ROACH_BUFFER_H
