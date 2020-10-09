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

#include "strpool.h"

#include "log.h"


void strpool_Init( StringPool *sp, int AllowDuplicate ) {
  sp->Head = NULL;
  sp->AllowDuplicate = AllowDuplicate;
  sp->MaxLen = 0;
}


StringItem *strpool_Add( StringPool *sp, char const *s ) {
  StringItem *str;
  size_t len;

  str = strpool_Get( sp, s );
  if( str && 0 == sp->AllowDuplicate ) {
    return str;
  } 

  len = strlen( s );
  if( len > sp->MaxLen ) {
    sp->MaxLen = len;
  }
  
  str = (StringItem*)calloc( sizeof(StringItem), 1 );
  if( !str ) {
    log_printf( LogErr, "No memory for new string." );
    return NULL;
  }

  strcpy( str->buf, s );
  str->len = len;
  str->offset = -1;
  HASH_ADD_STR( sp->Head, buf, str );

  return str;
}


StringItem *strpool_Get( StringPool *sp, char const *s ) {
  StringItem *item;
  HASH_FIND_STR( sp->Head, s, item );

  return item;
}

int strpool_MaxLen( StringPool *sp ) {
  return sp->MaxLen;
}