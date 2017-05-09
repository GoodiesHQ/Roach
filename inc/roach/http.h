#ifndef ROACH_HTTP_H
#define ROACH_HTTP_H

#include "./common.h"

#include <inttypes.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFSIZE_URL_PATH    1024
#define HTTP_PROTO          "http"
#define DEFAULT_HTTP_PORT   80
#define TIMEOUT             10
#define URL_PCHAR_CNT       4   // number of char* vars in the url_t struct

#define POSTFIX_PROTO       "://"
#define POSTFIX_PATH        "?"

#define INDEX_PROTO         0
#define INDEX_HOST          1
#define INDEX_PATH          2
#define INDEX_QUERY         3
#define INDEX_ADDR          4

typedef struct _url_t
{
    char *proto;        // only http is supported
    char *host;         // the fqdn of the host
    char *path;         // the path to acquire the file
    char *query;        // the query string
    char *addr;         // ip address
    uint16_t port;      // the port on which the HTTP server is running
} url_t;

typedef enum _connstate_t
{
    CONN_INIT,
    CONN_CONNECT,
    CONN_REFUSED,
    CONN_TIMEOUT,
    CONN_NXDOMAIN,
    CONN_SUCCESS,
} connstate_t;

typedef struct _state_http_t
{
    connstate_t connstate;
    atomic_bool complete;
} state_http_t;

char * url_to_string(const url_t *url);
url_t * url_create(const char *uri);
void url_destroy(url_t **urlPtr);

#endif//ROACH_HTTP_H
