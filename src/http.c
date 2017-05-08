#include "../inc/roach/http.h"

// A likely very buggy URL parser, but functional for my purposes
url_t * url_create(const char *uri)
{
    size_t uriSize, tokenSize, protoSize;
    char *uriStr, *token, *tokenPtr, *tokenHost, *tokenPtrHost, *tmp;

    url_t *url = (url_t*)malloc(sizeof(url_t));
    memset(url, 0x00, sizeof(url_t));

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
    } else
    {
        url_destroy(&url);
        return NULL;
    }
    debugf("Host_Port: %s\n", tokenHost);

    if((token = strtok_r(tokenHost, ":", &tokenPtrHost)) != NULL)
    {
        tokenSize = strlen(token);
        url->host = (char*)malloc(tokenSize + 1);
        strncpy(url->host, token, tokenSize);
    } else 
    {
        free(tokenHost);
        url_destroy(&url);
        return NULL;
    }
    debugf("Host: %s\n", url->host);

    if((token = strtok_r(NULL, ":", &tokenPtrHost)) == NULL)
    {
        url->port = DEFAULT_HTTP_PORT;
    } else {
        url->port = strtol(token, &tmp, 10);
        if(*tmp)
        {
            debugf("FAILURE: port is invalid: '%s'\n", token);
            free(tokenHost);
            url_destroy(&url);
            return NULL;
        }
    }
    debugf("Port: %d\n", url->port);

    if((token = strtok_r(NULL, "?", &tokenPtr)) == NULL)
    {
        // There is no more to parse. Use default HTTP path.
        url->path = (char*)calloc(1, 2);
        strcpy(url->path, "/");
    } else {
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
        strncpy(url->query, "", 1);
    } else {
        tokenSize = strlen(token);
        url->query = (char*)calloc(1, tokenSize + 1);
        strncpy(url->query, token, tokenSize);
    }
    debugf("Query: %s\n", url->query);

    return url;
}

void url_destroy(url_t **urlPtr)
{
    url_t *url = *urlPtr;
    if(url)
    {
        free(url->proto);
        free(url->host);
        free(url->path);
        free(url->query);
        free(url->addr);
        free(url);
    }
}
