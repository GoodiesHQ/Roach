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
    if((gai_err = getaddrinfo(client->url->host, NULL, client->hints, &client->res)) != 0)
    {
        debugf("GetAddrInfo() Error(%d): %s\n", gai_err, gai_strerror(gai_err));
        freeaddrinfo(client->res);
        client->res = NULL;
        switch(gai_err)
        {
            case EAI_NONAME:
                client->connstate = CONN_NXDOMAIN;
                break;
        }
        return FAILURE;
    }
    client->connstate = CONN_INIT;
    return SUCCESS;
}

status_t http_connect(http_client_t *client)
{
    if(client->url == NULL)
    {
        client->connstate = CONN_NO_URL;
        return FAILURE;
    }
    return SUCCESS;
}

char * url_to_string(const url_t *url)
{
    const char * const *parts = (const char * const *)url;
    const size_t difference = URL_T_PARTS_COUNT - URL_T_PARTS_EXCLUDE;
    size_t sizes[difference], pfxlen;
    
    size_t total = 0, i;

    total += strlen(POSTFIX_PROTO);
    total += strlen(POSTFIX_PATH);

    for(i = 0; i < difference; ++i)
    {
        printf("Part %lu: %s\n", i, parts[i] ? parts[i] : "Pointer is Null");
        sizes[i] = strlen(parts[i]);
        total += sizes[i];
    }

    char *final = calloc(1, total);
    char *ptr = final;

    for(i = 0; i < difference; ++i)
    {
        strncpy(ptr, parts[i], sizes[i]);
        ptr += sizes[i];
        switch(i)
        {
            case INDEX_PROTO:
                pfxlen = strlen(POSTFIX_PROTO);
                strncpy(ptr, POSTFIX_PROTO, pfxlen);
                break;
            case INDEX_HOST:
                pfxlen = strlen(POSTFIX_HOST);
                strncpy(ptr, POSTFIX_HOST, pfxlen);
                break;
            case INDEX_PATH:
                pfxlen = strlen(POSTFIX_PATH);
                strncpy(ptr, POSTFIX_PATH, pfxlen);
                break;
            default:
                pfxlen = 0;
                break;
        }
        ptr += pfxlen;
    }
    debugf("URL: %s\n", final);
    return final;
}

// A likely very buggy URL parser, but functional for my purposes
url_t * url_create(const char *uri)
{
    size_t uriSize, tokenSize, protoSize;
    char *uriStr, *token, *tokenPtr, *tokenHost, *tokenPtrHost, *tmp;

    url_t *url = (url_t*)calloc(1, sizeof(url_t));

    uriSize = strlen(uri);
    uriStr = (char*)calloc(1, uriSize + 1);
    strncpy(uriStr, uri, uriSize);

    // Determine the protocol
    token = strtok_r(uriStr, ":", &tokenPtr);
    debugf("URL Protocol: %s\n", token);
    
    // Only HTTP is going to be supported to keep the binary small and monolithic
    if(strcmp(token, HTTP_PROTO) != 0)
    {
        debugf("Protocol '%s' is not supported\n", token);
        free(uriStr);
        url_destroy(&url);
        return NULL;
    }

    protoSize = strlen(token);
    url->proto = (char*)malloc(protoSize);
    strncpy(url->proto, token, protoSize);

    if((token = strtok_r(NULL, "/", &tokenPtr)) != NULL)
    {
        tokenSize = strlen(token);
        tokenHost = (char*)malloc(tokenSize + 1);
        strncpy(tokenHost, token, tokenSize);
    }
    else
    {
        free(uriStr);
        url_destroy(&url);
        return NULL;
    }

    if((token = strtok_r(tokenHost, ":", &tokenPtrHost)) != NULL)
    {
        tokenSize = strlen(token);
        url->host = (char*)malloc(tokenSize + 1);
        strncpy(url->host, token, tokenSize);
    }
    else 
    {
        free(tokenHost);
        url_destroy(&url);
        return NULL;
    }
    debugf("Host: %s\n", url->host);

    if((token = strtok_r(NULL, ":", &tokenPtrHost)) == NULL)
    {
        tokenSize = strlen(DEFAULT_HTTP_PORT);
        url->port = calloc(1, tokenSize + 1);
        strncpy(url->port, DEFAULT_HTTP_PORT, tokenSize);
    }
    else
    {
        /*unsigned short port = */ strtol(token, &tmp, 10);
        if(*tmp)
        {
            debugf("FAILURE: port is invalid: '%s'\n", token);
            free(tokenHost);
            free(uriStr);
            url_destroy(&url);
            return NULL;
        }
        tokenSize = strlen(token);
        url->port = calloc(1, tokenSize + 1);
        strncpy(url->port, token, tokenSize);
    }
    debugf("Port: %s\n", url->port);

    if((token = strtok_r(NULL, "?", &tokenPtr)) == NULL)
    {
        // There is no more to parse. Use default HTTP path.
        url->path = (char*)calloc(1, 2);
        strcpy(url->path, "/");
    }
    else
    {
        tokenSize = strlen(token);
        url->path = (char*)calloc(1, tokenSize + 2);
        strcpy(url->path, "/");
        strcpy(url->path + 1, token);
    }
    debugf("Path: %s\n", url->path);

    if((token = strtok_r(NULL, "?", &tokenPtr)) == NULL)
    {
        // There is no more to parse. Use empty HTTP query.
        url->query = (char*)calloc(1, 1);
    }
    else
    {
        tokenSize = strlen(token);
        url->query = (char*)calloc(1, tokenSize + 1);
        strncpy(url->query, token, tokenSize);
    }
    debugf("Query: %s\n", url->query);

    free(tokenHost);
    free(uriStr);
    return url;
}


void url_destroy(url_t **urlPtr)
{
    debugf("%s\n", "Freeing URL");
    url_t *url = *urlPtr;
    char **parts = (char**)url;
    size_t i;
    if(url)
    {
        for(i = 0; i < URL_T_PARTS_COUNT; ++i)
        {
            free(parts[i]);
        }
        free(url);
    }
}
