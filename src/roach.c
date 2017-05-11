#include "../inc/roach.h"

status_t doit(const char *uri)
{
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
        http_client_destroy(&client);
        url_destroy(&purl);
        return FAILURE;
    }
    http_client_destroy(&client);
    url_destroy(&purl);
    return SUCCESS;
}

int main(int argc, char **argv)
{
    doit("http://google.com/");
    return 0;
}
