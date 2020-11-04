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


#include "defs.h"
#include "log.h"
#include "utils.h"

#include <ctype.h>
#include <string.h>
#include <assert.h>

#ifndef msizeof
# define msizeof(s, m) sizeof(((s*)0)->m)
#endif


DEF *s_defs = NULL;
LOC s_loc;

/*** defs_init **************************************************************/
void defs_init() {
  
  strcpy( s_loc.file, "PREDEFINED" );
  s_loc.line_nr = 0;
  s_loc.next = 0;
}

/*** defs_add ***************************************************************/
void defs_add( char const *line, LOC const *loc ) {
  char const *s1 = line;
  char const *s2;
  size_t n;

  if(strncmp("#define", line, 6 )) {
    return;
  }

  s1 = skip_space( (char*)(s1+7) );
  s2 = s1;
  
  DEF *def = (DEF*)calloc( sizeof(DEF), 1 );

  if( !loc ) {
    loc = &s_loc;
  }
  memcpy( &def->loc, loc, sizeof(LOC));
  for( ; *s1; s1++ ) {
    if( isspace( *s1 )) {
      break;
    }
  }

  n = s1 - s2;
  assert( n < 64 );
  memcpy( def->name, s2, n );

  s2 = s1;
  s1 = skip_space( (char*)s1 );
  if( s1 - s2 > 0 && *s1 ) {
    n = strlen(s1);
    assert( n < msizeof( DEF, value ));
    n = n > msizeof( DEF, value )-1 ? msizeof( DEF, value )-1 : n;
    memcpy( def->value, s1, n );
  }

  DEF *d = defs_get( def->name );
  if( d ) {
    if( 0 == strcmp(def->value, d->value)) {
      log_printf( LogWarn, loc_cur(), "Define %s already exists with same value. See: %s:%d, %s:%d",
        def->name, def->loc.file, def->loc.line_nr, d->loc.file, d->loc.line_nr  );
    }
    else {
      log_printf( LogErr, loc_cur(), "Define %s already exists. See: %s:%d, %s:%d",
        def->name, def->loc.file, def->loc.line_nr, d->loc.file, d->loc.line_nr  );
    }
  }
  
  HASH_ADD_STR( s_defs, name, def );
}

/*** defs_get ***************************************************************/
DEF* defs_get( char const *name ) {
  DEF *def;

  HASH_FIND_STR( s_defs, name, def );
  return def;
}

void defs_iter( DEF_ITER *def ) {
  def->cur = s_defs;
}

int defs_next( DEF *el, DEF_ITER *def ) {

  if( !def->cur ) {
    return 0;
  }
  
  *el = *def->cur;
  def->cur = def->cur->hh.next;

  return 1;
}
