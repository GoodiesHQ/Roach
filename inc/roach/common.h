#ifndef ROACH_COMMON_H
#define ROACH_COMMON_H

#include <stdio.h>

#ifdef DEBUG
#define debugf(fmt, ...) do { if (DEBUG) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, __VA_ARGS__); } while (0)
#else
#define debugf(...)
#endif

#define STR_(x) #x
#define STR(x) STR_(x)

#endif//ROACH_COMMON_H
