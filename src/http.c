#include "../inc/roach/roach.h"

url_t * url_create(const char *uri)
{
    size_t uriSize, tokenSize;
    char *urlstr, *token, *tokenPtr=NULL;
    char *host_port;

    url_t *url = malloc(sizeof(url_t));
    memset(url, 0x00, sizeof(url_t));

    uriSize = strlen(uri);
    urlstr = malloc(uriSize + 1);
    strcpy(urlstr, uri);

    // PROTO
    token = strtok_r(urlstr, ":", &tokenPtr);
    tokenSize = strlen(token);
    url->proto = malloc(tokenSize + 1);
    strncpy(url->proto, token, tokenSize);

    // HOST:PORT
    token = strtok_r(NULL, "/", &tokenPtr);
    if(token != NULL)
    {
        tokenSize = strlen(token);
        host_port = malloc(tokenSize);
        strncpy(host_port, token, tokenSize);
    }else{
        host_port = malloc(1);
        strncpy(host_port, "", 1);
    }

    free(urlstr);
    return url;
}
