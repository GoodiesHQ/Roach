#ifndef ROACH_HTTP_H
#define ROACH_HTTP_H

#include <string.h>
#include <stdatomic.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>

#define BUFSIZE_URL_PATH    1024
#define HTTP_PROTO          "http"
#define DEFAULT_HTTP_PORT   "80"
#define TIMEOUT             10

typedef struct _url_t
{
    char *proto;  // only http is supported
    char *host;   // the fqdn of the host
    short port;   // the port on which the HTTP server is running
    char *path;   // the path to acquire the file
    char *query;  //
    char *addr;   // ip address
} url_t;

typedef enum _connstate_t
{
    INITIALIZED,
    CONNECTED,
    CONN_REFUSED,
    CONN_TIMEOUT,
    UNKNOWN_HOST,
    SUCCESS,
} connstate_t;

typedef struct _state_http_t
{
    connstate_t connstate;
    atomic_bool complete;
} state_http_t;

url_t * url_create(const char *uri);
void url_destroy(url_t *url);

#endif//ROACH_HTTP_H
