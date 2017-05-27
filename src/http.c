#include "../inc/roach/http.h"


http_client_t *http_client_create(void)
{
    http_client_t *client = NULL;
    
    if((client = calloc(1, sizeof(http_client_t))) == NULL)
    {
        debugf(DBG_CRIT, "%s\n", "calloc() failure");
        return NULL;
    }

    if((client->hints = calloc(1, sizeof(struct addrinfo))) == NULL)
    {
        debugf(DBG_CRIT, "%s\n", "calloc() failure");
        free(client);
        return NULL;
    }
    client->headers = NULL;
    client->hints->ai_family = AF_UNSPEC;
    client->hints->ai_socktype = SOCK_STREAM;
    client->complete = ATOMIC_VAR_INIT(false);
    client->fd = DEFAULT_SOCKET_FD;
    return client;
}

void http_client_destroy(http_client_t **clientPtr)
{
    http_client_t *client = *clientPtr;
    if(client->res)
    {
        debugf(DBG_INFO, "%s\n", "Releasing addrinfo");
        freeaddrinfo(client->res);
    }
    if(client->url)
    {
        debugf(DBG_INFO, "%s\n", "Releasing client url");
        url_destroy(&client->url);
    }
    if(client->fd != DEFAULT_SOCKET_FD)
    {
        if(fcntl(client->fd, F_GETFL) < 0 && errno == EBADF)
        {
            close(client->fd);
        }
    }
    if(client->headers)
    {
        //http_header_destroy(&client->headers);
    }
    free(client->hints);
    free(client);
}

const char *http_state_str(connstate_t state)
{
    switch(state)
    {
        case CONN_INIT:
            return "Domain is valid and the client is initialized and ready to connect";
        case CONN_SUCCESS:
            return "The connection has connected successfully.";
        case CONN_NO_URL:
            return "No URL has been provided.";
        case CONN_REFUSED:
            return "The target has refused the connection.";
        case CONN_TIMEOUT:
            return "The connection has timed out.";
        case CONN_NXDOMAIN:
            return "The specified domain does not exist.";
        case CONN_IN_USE:
            return "The connection is already in use.";
        case CONN_INV_RESP:
            return "The server has given an invalid response.";
        case CONN_FAILURE:
            return "An unspecified error has occured.";
        default:
            return "Invalid State";
    }
}

status_t http_client_setattr(http_client_t *client, http_client_attr_t attr, ...)
{
    va_list valist;
    va_start(valist, attr);

    switch(attr)
    {
        case HTTP_ATTR_URL:
        {
            if(client->url)
            {
                url_destroy(&client->url);
            }

            const url_t *url = va_arg(valist, const url_t*);

            if((client->url = calloc(1, sizeof(url_t))) == NULL)
            {
                debugf(DBG_CRIT, "%s\n", "calloc() failure");
                return FAILURE;
            }

            return url_copy(client->url, url);
        }
        case HTTP_ATTR_HEADER:
        {
            break;
        }
        default:
            debugf(DBG_WARN, "%s\n", "Invalid Attribute");
            return FAILURE;
    }
    return SUCCESS;
}

status_t http_init_connection(http_client_t *client)
{
    int gai_err;
    if(client->url == NULL)
    {
        client->connstate = CONN_NO_URL;
        debugf(DBG_MAJOR, "%s\n", "Cannot create connection without a URL");
        return FAILURE;
    }
    if((gai_err = getaddrinfo(client->url->host, client->url->port, client->hints, &client->res)) != 0)
    {
        debugf(DBG_MAJOR, "GetAddrInfo() Error(%d): %s\n", gai_err, gai_strerror(gai_err));
        freeaddrinfo(client->res);
        client->res = NULL;
        switch(gai_err)
        {
            case EAI_NONAME:
                client->connstate = CONN_NXDOMAIN;
                break;
            default:
                client->connstate = CONN_FAILURE;
        }
        return FAILURE;
    }
    client->connstate = CONN_INIT;
    return SUCCESS;
}

status_t http_connect(http_client_t *client)
{
    int sockfd, i;
    struct addrinfo *iter;
    struct timeval timeout;

    timeout.tv_sec = TIMEOUT;
    timeout.tv_usec = 0;

    // No URL has been set
    if(client->url == NULL)
    {
        client->connstate = CONN_NO_URL;
        return FAILURE;
    }

    // Connection is not in INIT state
    if(client->connstate != CONN_INIT)
    {
        debugf(DBG_MAJOR, "%s\n", "HTTP client is not in the initialized state");
        return FAILURE;
    }

    // Iterate over the results from the previous getaddrinfo output
    for(iter = client->res, i = 1; iter != NULL; iter=iter->ai_next, ++i)
    {
        // Create a socket. If this goes awry, there are bigger issues
        if((sockfd = socket(iter->ai_family, iter->ai_socktype, iter->ai_protocol)) == -1)
        {
            debugf(DBG_WARN, "Try #%d: %s\n", i, "Creating the socket failed");
            client->connstate = CONN_FAILURE;
            continue;
        }
        debugf(DBG_INFO, "Successfully: %s\n", "Created Socket");

        // Set the send and recv timeout for the socket
        if(setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout)) < 0)
        {
            debugf(DBG_WARN, "Try #%d: %s\n", i, "Socket SND Timeout Failed");
            continue;
        }

        if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) < 0)
        {
            debugf(DBG_WARN, "Try #%d: %s\n", i, "Socket RCV Timeout Failed");
            continue;
        }

        // Establish a connection
        if(connect(sockfd, iter->ai_addr, iter->ai_addrlen) == -1)
        {
            debugf(DBG_WARN, "Try #%d: %s\n", i,"Connecting to the socket failed");
            client->connstate = CONN_FAILURE;
            continue;
        }

        debugf(DBG_INFO, "Successfully: %s\n", "Connection Created");

        client->connstate = CONN_SUCCESS;
        break;
    }

    switch(client->connstate)
    {
        case CONN_SUCCESS:
            client->fd = sockfd;
            return SUCCESS;
        default:
            debugf(DBG_MAJOR, "%s\n", "No valid connections could be formed");
            return FAILURE;
    }
}

buffer_t *http_get_buffer(http_client_t *client)
{
    buffer_t *buf = buffer_create();
    buffer_append_str(buf, "GET ");
    buffer_append_str(buf, client->url->path);
    buffer_append_str(buf, " HTTP/" HTTP_VER "\r\nHost: ");
    buffer_append_str(buf, client->url->host);
    buffer_append_str(buf, "\r\nConnection: close\r\nUser-Agent: " HTTP_UA "\r\n\r\n");
    return buf;
}

status_t http_send_buffer(http_client_t *client, buffer_t *buf)
{
    ssize_t sent = 0, sentTotal = 0;
    while(sentTotal < buf->used)
    {
        if((sent = send(client->fd, buf->data + sentTotal, buf->used - sentTotal, 0)) == -1)
        {
            debugf(DBG_WARN, "%s\n", "Could not send buffer.");
            return FAILURE;
        }
        debugf(DBG_INFO, "Successfully sent %zd bytes\n", sent);
        sentTotal += sent;
    }
    return SUCCESS;
}

status_t http_recv_buffer(http_client_t *client, buffer_t *buf)
{
    ssize_t received = 0, receivedTotal = 0;
    char tmp[BUFFER_CHUNK];

    while(true)
    {
        received = recv(client->fd, tmp, BUFFER_CHUNK, 0);
        if(received < 0)
        {
            debugf(DBG_WARN, "FAILURE: recv() = %zd\n", received);
            return FAILURE;
        }

        if(received == 0)
        {
            debugf(DBG_INFO, "%s\n", "Receive Complete");
            break;
        }

        debugf(DBG_INFO, "recv() received %zd bytes\n", received);
        receivedTotal += received;
        if(buffer_append(buf, tmp, received) == FAILURE)
        {
            return FAILURE;
        }
    }

    return SUCCESS;
}

status_t http_parse_headers(const char *header_begin, size_t size, http_response_t *response)
{
    size_t headerCnt = 0;
    const char *tmp = header_begin;
    while((tmp = strstr(tmp, CRLF)) != NULL && tmp < header_begin + size)
    {
        headerCnt++;
        tmp++;
    }
    debugf(DBG_INFO, "Found %zd headers\n", headerCnt);
    return SUCCESS;
}

status_t http_parse_response(buffer_t *buf, http_response_t *response)
{
    char *token, *tokenPtr, *tmp, *headerStr;
    size_t headerSize;

    if(response->buf != NULL)
    {
        debugf(DBG_WARN, "%s\n", "The provided buffer for the HTTP response is not empty. Destroying.");
        buffer_destroy(&response->buf);
    }

    token = strstr(buf->data, CRLF_SEP);
    if(token == NULL)
    {
        debugf(DBG_MAJOR, "%s\n", "HTTP '\\r\\n\\r\\n' was not found");
        return FAILURE;
    }
    headerSize = (size_t)(token - buf->data);
    /*if((headerStr = calloc(1, headerSize)) == NULL)
    {
        debugf(DBG_CRIT, "%s\n", "calloc() failure");
        return FAILURE;
    }*/

    if(http_parse_headers(buf->data, headerSize, response) == FAILURE)
    {

    }
}

status_t http_get(http_client_t *client, http_response_t *response)
{
    status_t status = FAILURE;
    buffer_t *req = http_get_buffer(client);
    buffer_t *buf = buffer_create();

    if(!req || !buf)
    {
        debugf(DBG_CRIT, "%s\n", "GET buffer failed to be created");
        free(req);
        free(buf);
        return FAILURE;
    }

    if(!buf)
    {
        buffer_destroy(&req);
        debugf(DBG_CRIT, "%s\n", "GET buffer failed to be created");
        return FAILURE;
    }

    if(http_send_buffer(client, req) == FAILURE)
    {
        client->connstate = CONN_FAILURE;
        goto fail;
    }

    if(http_recv_buffer(client, buf) == FAILURE)
    {
        client->connstate = CONN_FAILURE;
        goto fail;
    }

    if(http_parse_response(buf, response) == FAILURE)
    {
        client->connstate = CONN_INV_RESP;
        goto fail;
    }

    status = SUCCESS;

    //write(1, buf->data, buf->used);

fail:
    buffer_destroy(&req);
    buffer_destroy(&buf);
    return status;
}

