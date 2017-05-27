#ifndef ROACH_HTTP_H
#define ROACH_HTTP_H

#include "./common.h"
#include "./url.h"
#include "./buffer.h"

#include <stdarg.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFSIZE_URL_PATH    1024
#define CRLF                "\r\n"
#define CRLF_SEP            "\r\n\r\n"
#define HTTP_PROTO          "http"
#define HTTP_PROTO_RESP     "HTTP/"
#define HTTP_VER            "1.1"
#define HTTP_UA             "roach/" ROACH_VERSION
#define DEFAULT_HTTP_PORT   "80"
#ifndef TIMEOUT
#define TIMEOUT             1
#endif
#define DEFAULT_SOCKET_FD   -1

// May want to be used for a single varargs function like :
//
// http_client_setattr(HTTP_ATTR_URL, "http://researcher.com/file.bin")

typedef enum _http_client_attr_t
{
#define HTTP_ATTR_URL_NARGS 1
#define HTTP_ATTR_HEADER_NARGS 1
    HTTP_ATTR_URL,
    HTTP_ATTR_HEADER,
} http_client_attr_t;

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
    CONN_INV_RESP,      // the server has sent an invalid response
    CONN_FAILURE,
} connstate_t;

typedef struct _http_header_t
{
    char **key;
    char **value;
    size_t size;
} http_header_t;

typedef struct _http_response_t
{
    unsigned status_code;
    http_header_t *headers;
    size_t headerCnt;
    buffer_t *buf;
} http_response_t;

typedef struct _http_client_t
{
    url_t *url;
    struct addrinfo *hints, *res;
    int fd;
    http_header_t *headers;
    connstate_t connstate;
    atomic_bool complete;
} http_client_t;

// insitialization and destruction of objects
http_client_t *http_client_create(void);
void http_client_destroy(http_client_t **clientPtr);
http_header_t *http_header_create(void);
void http_header_destroy(http_header_t **headerPtr);

// status
const char *http_state_str(connstate_t state);

// set attributes on the client
status_t http_client_setattr(http_client_t *client, http_client_attr_t attr, ...);

// controls the connection of the client
status_t http_init_connection(http_client_t *client);
status_t http_connect(http_client_t *client);

// making http requests
status_t http_get(http_client_t *client, http_response_t *response);

// parsing and buffer management
status_t http_parse_response(buffer_t *buf, http_response_t *response);
status_t http_parse_headers(const char *header_begin, size_t size, http_response_t *response);
buffer_t *http_get_buffer(http_client_t *client);

#endif//ROACH_HTTP_H
