

#include "textio.h"

#include <string.h>
#include <errno.h>

struct TEXT_IO *s_tio = 0;
struct TEXT_IO *s_cur = 0;

int textio_open( char const * name, char const *args ) {
  struct TEXT_IO *t = s_tio;
  for(; t; t = t->next) {
    int n = strcmp( name, t->name );
    if( 0 == n ) {
      s_cur = t;
      break;
    }
  }

  if( s_cur ) {
    return t->open( args, t->priv );
  }

  return 0;
}

int textio_read( char *buf, int bufsz ) {
  if( !s_cur ) {
    return -EBADF;
  }

  return s_cur->read( buf, bufsz, s_cur->priv );
}

int textio_write( char const *buf, int bufsz ) {
  if( !s_cur ) {
    return -EBADF;
  }

  return s_cur->write( buf, bufsz, s_cur->priv );
}

int textio_ioctrl( int arg, void *argp, int argsz ) {
  if( !s_cur ) {
    return -EBADF;
  }

  return s_cur->ioctl( arg, argp, argsz, s_cur->priv );
}

int textio_register( struct TEXT_IO *tio ) {
  tio -> next = s_tio;
  s_tio = tio;

  return 0;
}
