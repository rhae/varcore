

#include "console.h"
#include "textio.h"
#include "vars.h"

#include "common.h"

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct SCPI {
  int    Chan;
  int    ChanAvail;
  STRBUF Scpi;
  STRBUF Ext;
  STRBUF Value;
  int    Request;
};

void  repl_run(char const *prompt);
int   repl_eval( char*, int, char const *, int );
char *skip_space( char* );
int   isscpi( char );
int   parse_scpi( struct SCPI *, char const *line );
void  test_parse_scpi();

int main(int argc, char **argv ) {

  test_parse_scpi();


  UNUSED_PARAM(argc);
  UNUSED_PARAM(argv);

  console_init();
  telnet_init();
  vars_init();

  textio_open( "TELNET", "port=8023" );

  repl_run( "vars> " );

  textio_write( "ENDE!\n", 6 );

  return 0;
}

void repl_run( char const *prompt ) {

  int prompt_len = strlen( prompt );

  textio_write( "varcore repl!\n", 13 );
  textio_write( "Press <CTL-D> to exit.\n", 24 );

  for(;;) {
    textio_write( prompt, prompt_len );
    char buf_in[256];
    char buf_out[512];

    int n = textio_read( buf_in, sizeof(buf_in));
    if( n <= 0) {
      break;
    }

    n = repl_eval( buf_out, sizeof(buf_out), buf_in, n );

    textio_write( buf_out, n );
    textio_write( "\n", 1 );
  }
}

int repl_eval( char *resp, int respsz, char const *req, int reqsz ) {
  struct SCPI S;
  int         ret;

  UNUSED_PARAM( reqsz );

  memset( &S, 0, sizeof(struct SCPI));

  char *p = skip_space( (char*)req );
  if( !isscpi( *p )) {
    return snprintf( resp, respsz, "ERROR: not a SCPI!" );
  }
  p++;
  ret = parse_scpi( &S, p );

  if( ret != 0 ) {
    return snprintf( resp, respsz, "ERROR: Invalid SCPI (%d)!", ret );
  }

  HND hnd = vc_get_hnd( S.Scpi );
  if( !S.Ext[0] ) {
    ret = vars_as_string( hnd, S.Request, S.Value, S.Chan, REQ_CMD );
    if( ret != kErrNone ) {
      return snprintf( resp, respsz, "ERROR %04X", ret );
    }

    if( S.Request == VarWrite ) {
      vars_as_string( hnd, VarRead, S.Value, S.Chan, REQ_CMD );
    }

    if( S.ChanAvail ) {
      return sprintf(resp, ":%02d:%s %s", S.Chan, S.Scpi, S.Value );
    }
    
    return sprintf(resp, ":%s %s", S.Scpi, S.Value );
  }

  return sprintf( resp, "Not implemented.");
  
}

int parse_scpi( struct SCPI *scpi, char const *line ) {
  enum {
    stBegin,
    stScpi,
    stExt,
    stValue,

    stEnd,
    stError,
  };

  assert( line );

  int   result = 0;
  int   state = stBegin;
  char *src = (char*) line;
  char *dst = 0;

  scpi->Chan = 0;
  scpi->ChanAvail = 0;

  src = skip_space( src );

  while( *src && state < stEnd ) {
    char c = *src;
    char *endp;

    switch( state ) {

      case stBegin:
        if( isdigit( c )) {
          scpi->Chan = strtol( src, &endp, 0 );
          scpi->ChanAvail = 1;
          src = endp;

          if( ':' == *endp ) {
            dst = scpi->Scpi;
            ++src;
            state = stScpi;
          }
          else {
            state = stError;
            result = -1;
          }
        }
        else {
          dst = scpi->Scpi;
          state = stScpi;
        }
        break;

      case stScpi:
        if( '.' == c ) {
          state = stExt;
          dst = scpi->Ext;
          ++src;
        }
        else if( isspace( c )) {
          src = skip_space( src );
          c = *src;

          if ( '?' == c ) {
            state = stEnd;
            scpi->Request = VarRead;
          }
          else {
            state = stValue;
            dst = scpi->Value;
            src = skip_space( src );
            scpi->Request = VarWrite;
          }
        }
        else if( isalnum( c ) || ':' == c ) {
          *dst = *src;
          ++src;
          ++dst;
          *dst = 0;
        }
        else if ( '?' == c ) {
          state = stEnd;
          scpi->Request = VarRead;
        }
        else {
          state = stError;
          result = -2;
        }
        break;

      case stExt:
        if( isspace( c )) {
          src = skip_space( src );
          c = *src;

          if ( '?' == c ) {
            state = stEnd;
            scpi->Request = VarRead;
          }
          else {
            state = stValue;
            dst = scpi->Value;
            src = skip_space( src );

            scpi->Request = VarWrite;
          }
        }
        else if( isalnum( c )) {
          *dst = *src;
          src++;
          dst++;
          *dst = '\0';
        }
        else {
          state = stError;
          result = -3;
        }
        break;

      case stValue:
        *dst = *src;
        ++dst;
        ++src;
        break;

      case stEnd:
      case stError:
        break;
    }
  }
  *dst = '\0';
  return result;
}

int isscpi( char c ) {
  return c == '*';
}

char *skip_space( char *s ) {
  while( s && isspace(*s)) {
    s++;
  }
  return s;
}

void test_parse_scpi() {
  struct SCPI S;
  int n;

  memset( &S, 'S', sizeof(struct SCPI));
  n = parse_scpi( &S, "01:TMP?" );

  memset( &S, 'S', sizeof(struct SCPI));
  n = parse_scpi( &S, "TMP?" );

  memset( &S, 'S', sizeof(struct SCPI));
  n = parse_scpi( &S, "TMP    \t?" );

  memset( &S, 'S', sizeof(struct SCPI));
  n = parse_scpi( &S, "TMP:ACT ?" );

  memset( &S, 'S', sizeof(struct SCPI));
  n = parse_scpi( &S, "TMP 123" );

  memset( &S, 'S', sizeof(struct SCPI));
  n = parse_scpi( &S, "TMP123" );

  memset( &S, 'S', sizeof(struct SCPI));
  n = parse_scpi( &S, "TMP.MAX ?" );

  memset( &S, 'S', sizeof(struct SCPI));
  n = parse_scpi( &S, "TMP.MAX xyv" );
}
