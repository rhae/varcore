
#pragma once

#include "uthash.h"
#include "../../lib/varcore.h"

typedef struct _StringItem {
  STRBUF buf;                /* key for hash */
  int len;
  int constant;
  int offset;
  UT_hash_handle hh;         /* makes this structure hashable */
} StringItem;

typedef struct _StringPool {
  StringItem *Head;
  int         AllowDuplicate;
  size_t      MaxLen;
} StringPool;

void         strpool_Init( StringPool *, int );
StringItem  *strpool_Add( StringPool *, char const * );
StringItem  *strpool_Get( StringPool *, char const * );

int          strpool_MaxLen( StringPool * );
