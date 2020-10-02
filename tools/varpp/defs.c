
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
      log_printf( LogWarn, "Define %s already exists with same value. See: %s:%d, %s:%d",
        def->name, def->loc.file, def->loc.line_nr, d->loc.file, d->loc.line_nr  );
    }
    else {
      log_printf( LogErr, "Define %s already exists. See: %s:%d, %s:%d",
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
