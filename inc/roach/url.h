#ifndef ROACH_URL_H
#define ROACH_URL_H

#include "./common.h"
#include "./buffer.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define HTTP_PROTO          "http"
#define DEFAULT_HTTP_PORT   "80"

#define POSTFIX_PROTO       "://"
#define POSTFIX_HOST        ":"
#define POSTFIX_PATH        "?"

#define INDEX_PROTO         0
#define INDEX_HOST          1
#define INDEX_PORT          2
#define INDEX_PATH          3
#define INDEX_QUERY         4
#define INDEX_ADDR          5

typedef struct _url_t url_t;
#define URL_T_PARTS_COUNT   sizeof(url_t) / sizeof(char*)
#define URL_T_PARTS_EXCLUDE 1

typedef struct _url_t
{
    char *proto;        // only http is supported
    char *host;         // the fqdn of the host
    char *port;         // string representation of the port
    char *path;         // the path to acquire the file
    char *query;        // the query string
    char *addr;         // ip address
} url_t;

status_t copy_url(url_t *dst, const url_t *src);
char * url_to_string(const url_t *url);
url_t * url_create(const char *uri);
void url_destroy(url_t **urlPtr);

#endif//ROACH_HTTP_H
