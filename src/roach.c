#include "../inc/roach.h"

int main(int argc, char **argv)
{
    char *uri1 = "http://some.domain.com/lol.out";
    char *uri2 = "http://some.domain.com:8080/lol.out?arch=x64";
    char *uri3 = "http://some.domain.com:8080";

    url_t *url1 = url_create(uri1);
    url_t *url2 = url_create(uri2);
    url_t *url3 = url_create(uri3);

    printf("1: %s\n", url1 == NULL ? "Fail" : "Success");
    printf("2: %s\n", url2 == NULL ? "Fail" : "Success");
    printf("3: %s\n", url3 == NULL ? "Fail" : "Success");

    url_destroy(&url1);
    url_destroy(&url2);
    url_destroy(&url3);

    return 0;
}
