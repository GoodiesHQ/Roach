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
    if(http_client_setattr(client, HTTP_ATTR_URL, purl) == SUCCESS)
    {
        if(http_init_connection(client) == SUCCESS)
        {
            if(http_connect(client) == SUCCESS)
            {
                http_response_t response = {0x00};
                if(http_get(client, &response) == SUCCESS)
                {
                    printf("Response Status: [%zd]\n", response.status_code);
                }
            }
        }
    }

    http_client_destroy(&client);
    url_destroy(&purl);
    return status;
}

int main(int argc, char **argv)
{
    doit("http://127.0.0.1:8000/test.txt");
    return 0;
}
