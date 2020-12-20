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

#include "strpool.h"

#include "log.h"


void strpool_Init( StringPool *sp, int DuplicatePolicy ) {
  sp->Head = NULL;
  sp->DuplicatePolicy = DuplicatePolicy;
  sp->MaxLen = 0;
}


StringItem *strpool_Add( StringPool *sp, char const *s, void *priv ) {
  StringItem *str;
  size_t len;

  str = strpool_Get( sp, s );
  if( str ) {
    switch( sp->DuplicatePolicy ) {
      case STRPOOL_DUP_ALLOW:
        return str;
      case STRPOOL_DUP_FAIL:
        return 0;
    }
  }

  len = strlen( s );
  if( len > sp->MaxLen ) {
    sp->MaxLen = len;
  }
  
  str = (StringItem*)calloc( sizeof(StringItem), 1 );
  if( !str ) {
    log_printf( LogErr, 0, "No memory for new string." );
    return NULL;
  }

  strcpy( str->buf, s );
  str->len = len;
  str->offset = -1;
  str->priv = priv;
  HASH_ADD_STR( sp->Head, buf, str );

  return str;
}


StringItem *strpool_Get( StringPool *sp, char const *s ) {
  StringItem *item;
#ifdef STRPOOL_SEARCH_ITER
  for( item = sp->Head; item ; item = item->hh.next ) {
    if( 0 == strcmp( s, item->buf )) {
      break;
    }
  }
#else
  HASH_FIND_STR( sp->Head, s, item );
#endif

  return item;
}

size_t strpool_MaxLen( StringPool *sp ) {
  return sp->MaxLen;
}


void strpool_iter( spool_iter *iter, StringPool *sp ) {
  iter->cur = sp->Head;
}

int strpool_next( spool_iter *iter, StringItem *el ) {
  return strpool_next2( iter, &el );
}

int strpool_next2( spool_iter *iter, StringItem **el ) {
  
  if( !iter->cur ) {
    return 0;
  }
  
  *el = iter->cur;
  iter->cur = iter->cur->hh.next;

  return 1;
}

#ifdef STRPOOL_TEST
#include <stdio.h>
int main(int argc, char** argv) {
  (void) argc;
  (void) argv;

#ifndef countof
# define countof(x) (sizeof(x)/sizeof(x[0]))
#endif

  static const char *names[] = {
    "Hello", "World", "Moon", "Sun", "Jupiter", "Mars", "Mars2"
  };

  StringPool sp;
  strpool_Init( &sp, 1 );

  for( int i = 0; i < countof(names); i++ ) {
    strpool_Add( &sp, names[i], 0 );
  }
  
  for( int i = countof(names)-1; i > 0 ; i-- ) {
    StringItem *si;
    si = strpool_Get( &sp, names[i] );
    if( !si ) {
      printf( "failed to find %s (%d)", names[i], i );
      continue;
    }
    printf("% 3d: %s\n", i, names[i] );
  }

  return 0;
}
#endif
