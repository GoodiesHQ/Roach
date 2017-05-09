#include "../inc/roach.h"

int doit(const char *uri)
{
    url_t *url = url_create(uri);
    char *str = url_to_string((const url_t*)url);
    printf("%s\n", str);
    free(str);
    url_destroy(&url);
    return 1;
}

int main(int argc, char **argv)
{
    doit("http://some.domain.com:8080/lol.out?arch=x64");
    return 0;
}
