#include "../inc/roach/http.h"


char * url_to_string(const url_t *url)
{
    const char * const *parts = (const char * const *)url;
    const size_t difference = URL_T_SIZE - URL_T_PARTS_EXCLUDE;
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
                printf("LOL %s LOL\n", POSTFIX_HOST);
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
    debugf("Host_Port: %s\n", tokenHost);

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
        url->port = malloc(tokenSize + 1);
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
        url->port = malloc(tokenSize + 1);
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
        //strncpy(url->query, "", 1); // necessary? I think not.
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
    url_t *url = *urlPtr;
    char **parts = (char**)url;
    size_t i;
    if(url)
    {
        for(i = 0; i < URL_T_SIZE; ++i)
        {
            free(parts[i]);
        }
        free(url);
    }
}
