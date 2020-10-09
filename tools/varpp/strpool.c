
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