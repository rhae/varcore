/*
 *  Copyright (c) 2020, Ruediger Haertel
 *  All rights reserved.
 *  
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  
 *  1. Redistributions of source code must retain the above copyright notice, this
 *     list of conditions and the following disclaimer.
 *  
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *  
 *  3. Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *  
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "loc.h"
#include "utlist.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static LOC *s_cur = NULL;

void loc_init( void )
{
  s_cur = NULL;
}

LOC *loc_cur() {
  return s_cur;
}

void loc_push( const char *file, int line_nr )
{
  LOC *loc = (LOC*) calloc( sizeof(LOC), 1 );

  strcpy( loc->file, file );
  loc->line_nr = line_nr;

  LL_PREPEND( s_cur, loc );
}

void loc_pop() {
  LL_DELETE( s_cur, s_cur );
}

void loc_set( int line_nr ) {
  assert( s_cur );

  s_cur->line_nr = line_nr;
}

int loc_fmt( char *buf, size_t buf_size, LOC const* loc ) {
  if( !loc ) {
    loc = s_cur;
  }

  return snprintf( buf, buf_size, "[%s:%d]", loc->file, loc->line_nr );
}