
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