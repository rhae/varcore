

#pragma once

#include <stdarg.h>

enum {
    LogNone  = 0,
    LogErr   = 1,
    LogWarn  = 2,
    LogInfo  = 3,
    LogDebug = 4,
};

void log_init( int nLevel );
int  log_printf( int nLevel, const char *format, ... );

