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

#include "utils.h"

#include <string.h>
#include <assert.h>
#include <ctype.h>

/***************************************************************************/
/**
 *
 */
void split(
    char*    s, /**< Pointer to string that should be separated */
    char     sep, /**< Character that separates the fields */
    char**   svec, /**< Pointer to a string array for the result */
    uint32_t  len, /**< Length of a field in the string array */
    uint32_t* u    /**< Number of fields in the string array */
  )
{
  uint32_t vcnt; // vector counter
  uint32_t cnt;
  uint32_t mark;
  char* buf;

  vcnt = 0;
  cnt = 0;
  buf = (char*) svec;

  while((*s != '\0') && (vcnt < *u)) {

    mark = 1;
    if( cnt < len ) {
      buf[cnt] = *s;
      mark = 0;
    }

    if( *s == sep ) {
      if( mark != 0 ) {
        cnt = len;
      }
      buf[cnt] = '\0';
      cnt = -1;
      vcnt++;
      buf = (char*) ((long) svec + (vcnt * len));
    }

    cnt++;
    s++;
  }
  *u = vcnt + 1;

  for( uint32_t n = 0; n < *u; n++ ) {
    char *p = (char*) ((long) svec + (n * len));
    strtrim( p, '"' );
  }
}

/***************************************************************************/
/**
 *
 */
char* srepeat(char c, uint16_t len)
{
  static char buf[65336];
  int i;
  for ( i = 0; i < len; i++ )
  {
    buf[i] = c;
  }
  buf[i] = '\0';

  return buf;
}

/***************************************************************************/
/**
 *
 */
char* strtrim(char* s, char c)
{
  int i;
  int nLen;
  char *p;

  assert( s != 0 );

  p = s;
  nLen = strlen( s );

  if( nLen == 0 ) {
    return s;
  }

  while((nLen > 0) && (p[nLen-1] == c)) {
    nLen--;
  }
  p[nLen] = '\0';

  i = 0;
  while( p && p[i] == c ) {
    i++;
  }

  if( i > 0 ) {
    memmove( s, &p[i], nLen );
  }

  return s;
}

char* skip_space( char *s ) {

  if( !s ) {
    return s;
  }

  while( *s && isspace( *s )) {
    s++;
  }
  return s;
}