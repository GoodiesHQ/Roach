#ifndef ROACH_HTTP_H
#define ROACH_HTTP_H

#include "./common.h"
#include "./url.h"
#include "./buffer.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFSIZE_URL_PATH    1024
#define HTTP_PROTO          "http"
#define DEFAULT_HTTP_PORT   "80"
#ifndef TIMEOUT
#define TIMEOUT             1
#endif
#define DEFAULT_SOCKET_FD   -1

typedef enum _connstate_t
{
    CONN_INIT,          // connection is ready to be started
    CONN_SUCCESS,       // connection is successful
    // Failures
    CONN_NO_URL,        // a proper url was not provided
    CONN_REFUSED,       // the target refused the connection
    CONN_TIMEOUT,       // the connection exceeded the timeout
    CONN_NXDOMAIN,      // the specified domain does not exist
    CONN_IN_USE,        // the client has an unclosed file descriptor
    CONN_FAILURE,
} connstate_t;

typedef struct _http_client_t
{
    url_t *url;
    struct addrinfo *hints, *res;
    int fd;
    connstate_t connstate;
    atomic_bool complete;
} http_client_t;

typedef struct _http_response_t
{
    unsigned status_code;
    buffer_t *buf;
} http_response_t;

http_client_t *http_client_create(void);
void http_client_destroy(http_client_t **clientPtr);
const char *http_state_str(connstate_t state);

void http_client_set_url(http_client_t *client, const url_t *url);
status_t http_init_connection(http_client_t *client);
status_t http_connect(http_client_t *client);

http_response_t *http_get(http_client_t *client);

#endif//ROACH_HTTP_H
