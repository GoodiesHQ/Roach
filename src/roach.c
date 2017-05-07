#ifdef BUILD
#include <roach.h>
#else
// temporary guard so my buggy version of clang_complete won't complain about missing headers during development
#include "../inc/roach.h"
#endif

int main(int argc, char **argv)
{

    return 0;
}
