#include "../inc/roach/http.h"

url_t * url_create(const char *uri)
{
    size_t uriSize, tokenSize;
    char *urlstr, *token, *tokenPtr=NULL;

    url_t *url = malloc(sizeof(url_t));
    memset(url, 0x00, sizeof(url_t));

    // TODO: parse urls

    return url;
}
