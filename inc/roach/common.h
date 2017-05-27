#ifndef ROACH_COMMON_H
#define ROACH_COMMON_H

#include <stdio.h>

#define DBG_INFO    1
#define DBG_WARN    2
#define DBG_MAJOR   3
#define DBG_CRIT    4

#define MSG_INFO    "INFO: "        // Verbose general information
#define MSG_WARN    "WARNING: "     // Hey, seriously you should check this out.
#define MSG_MAJOR   "MAJOR: "       // Yo, something is really fucked up, fix it.
#define MSG_CRIT    "CRITICAL: "    // YOU FUCKED UP EVERYTHING

#ifdef DEBUG
#define debugf(DBG_LVL, fmt, ...) \
    do { \
        char *_dbg_msg; \
        switch(DBG_LVL) \
        { \
            case DBG_INFO: \
                _dbg_msg = MSG_INFO; \
                break; \
            case DBG_WARN: \
                _dbg_msg = MSG_WARN; \
                break; \
            case DBG_MAJOR: \
                _dbg_msg = MSG_MAJOR; \
                break; \
            case DBG_CRIT: \
                _dbg_msg = MSG_CRIT; \
                break; \
        } \
        if (_dbg_msg && DEBUG <= DBG_LVL) \
        { \
            fprintf(stderr, "%s:%d:%s(): %s" fmt, __FILE__, __LINE__, __func__, _dbg_msg, __VA_ARGS__); \
        } \
    } while (0)
#else
#define debugf(...)
#endif


#define STR_(x) #x
#define STR(x) STR_(x)
#define ALIGN(V, B) V >= 0 ? ((V + B - 1) / B) * B : (V / B) * B
#define ROACH_VERSION   "0.1"

typedef enum _status_t
{
    SUCCESS,
    FAILURE,
} status_t;

#endif//ROACH_COMMON_H
