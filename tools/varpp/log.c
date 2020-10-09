/*
 *  Copyright (c) 2020, Ruediger Haertel
 *  All rights reserved.
 *  
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *  
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *  
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  
 *  3. Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *  
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 *  TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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

    if( nLevel > s_nLogLevel ) {
	    nPrint = 0;
    }

    if( nPrint )
    {
        p = s_Buf;
        memset( p, 0, BufSize );

        va_start( ap, format );
            if( s_nLogLevel > LogInfo || nLevel < LogInfo) {
                nPos  = sprintf( p, "%s", s_szPrefix[nLevel] );
            }
            nPos += vsprintf( &p[nPos], format, ap );
            fputs( p, stderr );
            fputs( "\n", stderr );
        va_end( ap );
    }

    return nPos;
}

