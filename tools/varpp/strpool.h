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

#pragma once

#include "uthash.h"


typedef struct _StringItem {
  char buf[1024];                /* key for hash */
  int len;
  int constant;
  int offset;
  void *priv;
  UT_hash_handle hh;         /* makes this structure hashable */
} StringItem;

enum {
  STRPOOL_DUP_FAIL = -1,
  STRPOOL_DUP_ALLOW = 1,
};

typedef struct _StringPool {
  StringItem *Head;
  int         DuplicatePolicy;
  size_t      MaxLen;
} StringPool;

typedef struct _strpool_iter {
  StringItem *cur;
} spool_iter;

void         strpool_Init( StringPool *, int );
StringItem  *strpool_Add( StringPool *, char const *, void * );
StringItem  *strpool_Get( StringPool *, char const * );

int          strpool_MaxLen( StringPool * );

void         strpool_iter( spool_iter *, StringPool * );
int          strpool_next( spool_iter *, StringItem * );
int          strpool_next2( spool_iter *iter, StringItem **el );

