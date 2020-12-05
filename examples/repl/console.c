

#include "console.h"

#include "common.h"
#include "textio.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

#define GET_DATA( p ) struct CONSOLE_DATA *d = (struct CONSOLE_DATA *)p;

struct CONSOLE_DATA {
  FILE *fd_in;
  FILE *fd_out;
};

struct CONSOLE_DATA data;

int console_open( char const *args, void *priv ) {
  UNUSED_PARAM( args );
  UNUSED_PARAM( priv );

  return 0;
}

int console_read( char *buf, int bufsz, void *priv ) {
  GET_DATA( priv );

  char *s = fgets( buf, bufsz, d->fd_in );
  if( s == buf ) {
    return strlen( buf );
  }

  if( feof( d->fd_in )) {
    return EOF;
  }
  return feof( d->fd_in );
}

int console_write( char const *buf, int bufsz, void *priv ) {
  UNUSED_PARAM( bufsz );
  GET_DATA( priv );

  return fputs( buf, d->fd_out );
}

int console_ioctl( int arg, void *argp, int argsz, void *priv ) {
  UNUSED_PARAM( arg );
  UNUSED_PARAM( argp );
  UNUSED_PARAM( argsz );
  UNUSED_PARAM( priv );
  return 0;
}

struct TEXT_IO console = {
  .name = "CONSOLE",
  .open = console_open,
  .read = console_read,
  .write = console_write,
  .ioctl = console_ioctl,
  .priv = (void *) &data
};

void console_init() {
  data.fd_in = stdin;
  data.fd_out = stdout;

  textio_register( &console );
}
