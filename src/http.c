#include "../inc/roach/http.h"


http_client_t *http_client_create(void)
{
    http_client_t *client = (http_client_t*)calloc(1, sizeof(http_client_t));
    client->hints = (struct addrinfo*)calloc(1, sizeof(struct addrinfo));
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
        debugf("%s\n", "Releasing addrinfo");
        freeaddrinfo(client->res);
    }
    if(client->url)
    {
        debugf("%s\n", "Releasing client url");
        url_destroy(&client->url);
    }
    if(client->fd != DEFAULT_SOCKET_FD)
    {
        if(fcntl(client->fd, F_GETFL) < 0 && errno == EBADF)
        {
            close(client->fd);
        }
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
        case CONN_FAILURE:
            return "An unspecified error has occured.";
        default:
            return "Invalid State";
    }
}

void http_client_set_url(http_client_t *client, const url_t *url)
{
    if(client->url)
    {
        url_destroy(&client->url);
    }
    size_t i, len;
    char **copy;
    const char * const * orig = (const char * const *)url;

    if(client->url)
    {
        debugf("%s\n", "URL Already Exists");
    }
    client->url = calloc(1, sizeof(url_t));
    copy = (char**)client->url;
    for(i = 0; i < URL_T_PARTS_COUNT; ++i)
    {
        if(orig[i] == NULL)
        {
            copy[i] = NULL;
            continue;
        }
        len = strlen(orig[i]);
        copy[i] = calloc(1, len + 1);
        memmove(copy[i], orig[i], len);
    }
}

status_t http_init_connection(http_client_t *client)
{
    int gai_err;
    if(client->url == NULL)
    {
        client->connstate = CONN_NO_URL;
        debugf("%s\n", "Cannot create connection without a URL");
        return FAILURE;
    }
    if((gai_err = getaddrinfo(client->url->host, client->url->port, client->hints, &client->res)) != 0)
    {
        debugf("GetAddrInfo() Error(%d): %s\n", gai_err, gai_strerror(gai_err));
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
        debugf("%s\n", "HTTP client is not in the initialized state");
        return FAILURE;
    }

    // Iterate over the results from the previous getaddrinfo output
    for(iter = client->res, i = 1; iter != NULL; iter=iter->ai_next, ++i)
    {
        // Create a socket. If this goes awry, there are bigger issues
        if((sockfd = socket(iter->ai_family, iter->ai_socktype, iter->ai_protocol)) == -1)
        {
            debugf("Try #%d: %s\n", i, "Creating the socket failed");
            client->connstate = CONN_FAILURE;
            continue;
        }
        debugf("SUCCESS: %s\n", "Socket Created");

        // Set the send and recv timeout for the socket
        if(setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout)) < 0)
        {
            debugf("Try #%d: %s\n", i, "Socket SND Timeout Failed");
            continue;
        }

        if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) < 0)
        {
            debugf("Try #%d: %s\n", i, "Socket RCV Timeout Failed");
            continue;
        }

        // Establish a connection
        if(connect(sockfd, iter->ai_addr, iter->ai_addrlen) == -1)
        {
            debugf("Try #%d: %s\n", i,"Connecting to the socket failed");
            client->connstate = CONN_FAILURE;
            continue;
        }

        debugf("SUCCESS: %s\n", "Connection Created");

        client->connstate = CONN_SUCCESS;
        break;
    }

    switch(client->connstate)
    {
        case CONN_SUCCESS:
            client->fd = sockfd;
            return SUCCESS;
        default:
            debugf("%s\n", "No valid connections could be formed");
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
            debugf("%s\n", "Could not send buffer.");
            return FAILURE;
        }
        debugf("SUCCESS: sent %zd bytes\n", sent);
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
            debugf("FAILURE: recv() = %zd\n", received);
            return FAILURE;
        }
        if(received == 0)
        {
            debugf("%s\n", "Receive Complete");
            break;
        }

        debugf("recv() received %zd bytes\n", received);
        receivedTotal += received;
        if(buffer_append(buf, tmp, received) == FAILURE)
        {
            return FAILURE;
        }
    }

    return SUCCESS;
}

http_response_t *http_get(http_client_t *client)
{
    buffer_t *req = http_get_buffer(client), *res = buffer_create();
    if(!req)
    {
        debugf("%s\n", "GET buffer failed to be created");
        return NULL;
    }

    if(http_send_buffer(client, req) == FAILURE)
    {
        client->connstate = CONN_FAILURE;
        goto fail;
    }

    if(http_recv_buffer(client, res) == FAILURE)
    {
        client->connstate = CONN_FAILURE;
        goto fail;
    }

    write(1, res->data, res->used);

fail:
    buffer_destroy(&req);
    buffer_destroy(&res);
    return NULL;
}

