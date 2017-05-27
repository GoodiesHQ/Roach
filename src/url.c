#include "../inc/roach/url.h"

status_t url_copy(url_t *dst, const url_t *src)
{
    size_t i, len;
    char **target = (char**)dst;
    const char * const * orig = (const char * const *)src;

    for(i = 0; i < URL_T_PARTS_COUNT; ++i)
    {
        if(orig[i] == NULL)
        {
            target[i] = NULL;
            continue;
        }

        len = strlen(orig[i]);
        if((target[i] = calloc(1, len + 1)) == NULL)
        {
            debugf(DBG_CRIT, "%s\n", "calloc() failure");
            return FAILURE;
        }
        memmove(target[i], orig[i], len);
    }
    return SUCCESS;
}


char * url_to_string(const url_t *url)
{
    if(!url)
    {
        debugf(DBG_WARN, "%s\n", "url was NULL");
        return NULL;
    }

    char *tmp;
    buffer_t *buffer;
    const char * const *parts = (const char * const *)url;
    const size_t difference = URL_T_PARTS_COUNT - URL_T_PARTS_EXCLUDE;
    size_t i = 0;

    if((buffer = buffer_create()) == NULL)
    {
        debugf(DBG_CRIT, "%s\n", "buffer was not created");
        return NULL;
    }

    for(i = 0; i < difference; ++i)
    {
        if(buffer_append_str(buffer, parts[i]) == FAILURE)
        {
            buffer_destroy(&buffer);
            return NULL;
        }

        switch(i)
        {
            case INDEX_PROTO:
                tmp = POSTFIX_PROTO;
                break;
            case INDEX_HOST:
                tmp = POSTFIX_HOST;
                break;
            case INDEX_PATH:
                if(strlen(parts[INDEX_QUERY]))
                {
                    tmp = POSTFIX_PATH;
                }
                break;
            default:
                tmp = NULL;
        }

        if(tmp && buffer_append_str(buffer, tmp) == FAILURE)
        {
            buffer_destroy(&buffer);
            return NULL;
        }
    }

    tmp = buffer_to_str(buffer);
    buffer_destroy(&buffer);
    return tmp;
}

// A likely very buggy URL parser, but functional for my purposes
url_t * url_create(const char *uri)
{
    const size_t uriSize = strlen(uri); // length of the original URI
    size_t tokenSize, protoSize; // string lengths
    char *uriStr, *token, *tokenPtr, *tokenHost, *tokenPtrHost, *tmp; // tokens for parsing
    url_t *url;

    if((url = (url_t*)calloc(1, sizeof(url_t))) == NULL)
    {
        debugf(DBG_CRIT, "%s\n", "calloc() failure");
        return NULL;
    }

    if((uriStr = (char*)calloc(1, uriSize + 1)) == NULL)
    {
        url_destroy(&url);
        debugf(DBG_CRIT, "%s\n", "calloc() failure");
        return NULL;
    }

    memmove(uriStr, uri, uriSize);

    // Determine the protocol
    token = strtok_r(uriStr, ":", &tokenPtr);
    debugf(DBG_INFO, "URL Protocol: %s\n", token);
    
    // Only HTTP is going to be supported to keep the binary small and monolithic
    if(strcmp(token, HTTP_PROTO) != 0)
    {
        free(uriStr);
        url_destroy(&url);
        debugf(DBG_MAJOR, "Protocol '%s' is not supported\n", token);
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
        debugf(DBG_MAJOR, "%s\n", "The token was NULL.");
        return NULL;
    }

    if((token = strtok_r(tokenHost, ":", &tokenPtrHost)) != NULL)
    {
        printf("LOL 1: %s\n", token);
        tokenSize = strlen(token);
        url->host = (char*)malloc(tokenSize + 1);
        strncpy(url->host, token, tokenSize);
    }
    else 
    {
        free(tokenHost);
        url_destroy(&url);
        debugf(DBG_MAJOR, "%s\n", "The token was NULL.");
        return NULL;
    }

    debugf(DBG_INFO, "Host: %s\n", url->host);

    if((token = strtok_r(NULL, ":", &tokenPtrHost)) == NULL)
    {
        tokenSize = strlen(DEFAULT_HTTP_PORT);
        url->port = calloc(1, tokenSize + 1);
        strncpy(url->port, DEFAULT_HTTP_PORT, tokenSize);
    }
    else
    {
        strtol(token, &tmp, 10);
        if(*tmp)
        {
            debugf(DBG_MAJOR, "'%s' is an invalid port\n", token);
            free(tokenHost);
            free(uriStr);
            url_destroy(&url);
            return NULL;
        }
        tokenSize = strlen(token);
        url->port = calloc(1, tokenSize + 1);
        strncpy(url->port, token, tokenSize);
    }
    debugf(DBG_INFO, "Port: %s\n", url->port);

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
    debugf(DBG_INFO, "Path: %s\n", url->path);

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

    debugf(DBG_INFO, "Query: %s\n", url->query);
    free(tokenHost);
    free(uriStr);
    return url;
}

void url_destroy(url_t **urlPtr)
{
    debugf(DBG_INFO, "%s\n", "Freeing URL");
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
