#include "../inc/roach.h"

status_t doit(const char *uri)
{
    status_t status = SUCCESS;

    url_t *purl = url_create(uri);

    if(!purl)
    {
        fprintf(stderr, "Invalid URL\n");
        return -1;
    }

    http_client_t *client = http_client_create();
    http_client_set_url(client, purl);

    if(http_init_connection(client) == FAILURE)
    {
        debugf("%s\n", "Connection Failed!");
        status = FAILURE;
        goto cleanup;
    }
    
    // other stuff

cleanup:
    http_client_destroy(&client);
    url_destroy(&purl);
    return status;
}

int main(int argc, char **argv)
{
    doit("http://httpbin.org/ip");
    return 0;
}

