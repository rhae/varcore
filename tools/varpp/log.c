
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "log.h"

enum { BufSize = 8192 };

static int s_nLogLevel = LogDebug;
static char s_Buf[BufSize];

static char* s_szPrefix[] = {
      "" ,
      "[ERROR]  " ,
      "[WARN]   " ,
      "[INFO]   " ,
      "[DEBUG]  " ,
      "\0"
};

void log_init( int nLevel )
{
    s_nLogLevel = nLevel;
}

/***************************************************************************/
/**
 *
 */
int log_printf( 
	int nLevel,
	const char *format, 
	...
    )
{
    int nPrint = 1;
    va_list ap;
    char *p;
    int nPos = 0;

    if ( (nLevel > s_nLogLevel) ) 
    {
	    nPrint = 0;
    }

    if ( nPrint )
    {
        p = s_Buf;
        memset( p, 0, BufSize );

        va_start( ap, format );
            nPos  = sprintf( p, "%s", s_szPrefix[nLevel] );
            nPos += vsprintf( &p[nPos], format, ap );
            fputs( p, stderr );
            fputs( "\n", stderr );
        va_end( ap );
    }

    return nPos;
}

