#include "../inc/roach.h"

int doit(const char *uri)
{
    url_t url, *purl = &url;
    url_create_ptr(&purl, uri);

    if(!purl)
    {
        fprintf(stderr, "Invalid URL\n");
        return -1;
    }

    http_client_t *client = http_client_create();
    http_client_set_url(client, &url);

    char *str = url_to_string((const url_t*)client->url);
    printf("%s\n", str);
    free(str);
    http_client_destroy(&client);
    return 1;
}

int main(int argc, char **argv)
{
    doit("http://some.domain.com:8080/lol.out?arch=x64");
    return 0;
}
