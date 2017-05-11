#ifndef ROACH_HTTP_H
#define ROACH_HTTP_H

#include "./common.h"

#include <arpa/inet.h>
#include <inttypes.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFSIZE_URL_PATH    1024
#define HTTP_PROTO          "http"
#define DEFAULT_HTTP_PORT   "80"
#define TIMEOUT             10

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

typedef enum _connstate_t
{
    CONN_INIT,          // connection is ready to be started
    CONN_SUCCESS,       // connection is successful

    // Failures
    CONN_NO_URL,        // a proper url was not provided
    CONN_REFUSED,       // the target refused the connection
    CONN_TIMEOUT,       // the connection exceeded the timeout
    CONN_NXDOMAIN,      // the specified domain does not exist
} connstate_t;

typedef struct _http_client_t
{
    url_t *url;
    struct addrinfo *hints, *res;
    int fd;
    connstate_t connstate;
    atomic_bool complete;
} http_client_t;

http_client_t *http_client_create(void);
void http_client_destroy(http_client_t **clientPtr);

void http_client_set_url(http_client_t *client, const url_t *url);
status_t http_init_connection(http_client_t *client);

char * url_to_string(const url_t *url);
url_t * url_create(const char *uri);
void url_destroy(url_t **urlPtr);

#endif//ROACH_HTTP_H
