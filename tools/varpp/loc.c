
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