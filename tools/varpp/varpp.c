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

/*
 * varpp.c
 *
 *  Created on: 25.12.2010
 *      Author: hae
 */

#include "defs.h"
#include "loc.h"
#include "log.h"
#include "strpool.h"
#include "utils.h"
#include "utlist.h"
#include "uthash.h"
#include "../../lib/varcore.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>
#include <time.h>


#ifndef UNUSED_PARAM
#  define UNUSED_PARAM(x) (void)(x)
#endif

#ifndef UNHANDLED_CASE
#  define UNHANDLED_CASE(x) log_printf( LogErr, 0, "%s:%d Unhandled case %d", __FUNCTION__, __LINE__, x );
#endif

#ifndef countof
# define countof(x) ( sizeof(x) / sizeof(x[0]) )
#endif

#define str( s ) #s
#define xstr( s ) str(x)

enum {BufSize = 256};

typedef enum {
  kSecVariables,
  kSecStrings
} Section;

typedef struct {
  Section section;
  char const *prefix;
  size_t prefix_len;
  int init_data;
} Config;

typedef struct _DATA_NUMBER {
  double def_value;
  double min;
  double max;
} PP_DATA_NUMBER;

typedef struct _ENUM_MBR_DESC {
  int    value;
  STRBUF hnd;
  STRBUF string;
  struct _ENUM_MBR_DESC *next;
} ENUM_MBR_DESC;

typedef struct _PP_DATA_ENUM {
  int def_mbr;
  int cnt;
  ENUM_MBR_DESC *items;
} PP_DATA_ENUM;

typedef struct _PP_DATA_STRING {
  STRBUF def_value;
  int flags;
} PP_DATA_STRING;

typedef struct _DataItem {
  STRBUF hnd;
  STRBUF scpi;
  
  int acc_rights;
  int vec_items;
  int storage;
  int format;
  int type;

  union _DATA {
    PP_DATA_NUMBER data_number;
    PP_DATA_ENUM data_enum;
    PP_DATA_STRING data_string;
  } data;

  struct _DataItem *next;
} DataItem;

typedef struct {
  char     szKey[BufSize];
  int      nValue;
} Map_t;

enum {
  ColHnd = 0,
  ColScpi = 1,
  ColCanopen = 2,
  ColAccess = 3,
  ColStorage = 4,
  ColVector = 5,
  ColFormat = 6,
  ColType = 7,

  ColCommonLast
};

/* internal functions
 ********************************************************************************/

static int  get_type( char *, int *);
static int  get_vector( char *, int *);
static int  get_access( char *, int *);
static int  get_storage( char *, int *);
static int  get_format( char *, int *);

enum {
  line_len  = 255,
  LineSize = line_len+1
};
enum {
  MaxCsvColumns = 16
};
typedef char CSV_BUF[MaxCsvColumns][LineSize];

static int parse_number( DataItem *, size_t, CSV_BUF* );
static int parse_string( DataItem *, size_t, CSV_BUF* );
static int parse_enum( DataItem *, size_t, CSV_BUF* );

int  read_csv_file( DataItem **, char * );
int  save_inc_file( DataItem *, char * );
int  save_var_file( DataItem *, char * );
int  save_data_int( FILE *fp, DataItem *head, char const *name, int type, int );
int  save_data_string( FILE *fp, DataItem *head, char const *name, int type );
int  save_data_const_string( FILE *fp, DataItem *head, char const *name, int type );
int  save_data_enum( FILE *fp, DataItem *head, char const *name, int type );
int  save_data_enum_mbr( FILE *fp, DataItem *head, char const *name, int type );
int  serialize_enum( char *, size_t, PP_DATA_ENUM * );
int  save_vc_def( FILE *fp, DataItem *head );

void handle_pragma( char const*, LOC const* );

char *get_path( char *path, char const *fname );
char *join_path( char *oname, char const *path, char const *fname );

typedef struct _Stats {
  size_t max_var_hnd_len;
} Stats;

enum {
  spScpi,
  spStrings,

  spMax
};

DataItem     *s_Data = NULL;
StringPool    s_StrPools[spMax];
/**
 * The s_EnumPool is a string pool that contains the
 * serialized enum member description and a pointer to the
 * PP_DATA_ENUM of the DataItem.
 * 
 * The enum pool ist the key to remove duplicate enuum descriptors.
 */
StringPool    s_EnumPool;
Stats         s_Stats;
Config        s_Cfg;


static int s_nVarCnt = 0;
static int s_nTypeCnt[TYPE_LAST] = { 0 };

typedef struct {
  S32 Major;
  S32 Minor;
  S32 Patch;
  S8  *Name;
  S8  *Shortname;
  S8  *Copyright;
  S8  *Date;
} VERSION;

static VERSION s_Version = {
  1, 2, 0,
  "Variable preprocessor",
  "varpp",
  "(C) R. Haertel",
  "2020"
};

static void puts_version() {
  fprintf( stdout, "%s %d.%d.%d, %s %s\n",
   s_Version.Name,
   s_Version.Major, s_Version.Minor, s_Version.Patch,
   s_Version.Copyright,
   s_Version.Date );
}

static void print_cfg( Config *C ) {
  log_printf(LogDebug, 0, "Pragmas:");
  log_printf(LogDebug, 0, "  prefix     : %s", C->prefix );
  log_printf(LogDebug, 0, "  init_data  : %s", C->init_data ? "on" : "off" );
}

/**
 *
 *
 *
 */
int main( int argc, char **argv )
{
  UNUSED_PARAM( argc );
  char *fname = (char*) calloc( PATH_MAX, 1 );
  char *path = (char*) calloc( PATH_MAX, 1 );
  char *oname = (char*) calloc( PATH_MAX, 1 );
  char def[BufSize];
  int res;

  memset( &s_Stats, 0, sizeof(Stats));

  defs_init();
  loc_init();
  log_init( LogInfo );
  strpool_Init( &s_StrPools[spScpi], STRPOOL_DUP_FAIL );
  strpool_Init( &s_StrPools[spStrings], STRPOOL_DUP_ALLOW );
  strpool_Init( &s_EnumPool, 0 );

  /* initializes s_Cfg */
  handle_pragma( "#pragma section var", 0 );
  handle_pragma( "#pragma prefix VAR_", 0 );
  handle_pragma( "#pragma init_data off", 0 );

  puts_version();
  print_cfg( &s_Cfg );

  sprintf( def, "#define VARPP_MAJOR       %d", s_Version.Major );
  defs_add( def, 0 );
  sprintf( def, "#define VARPP_MINOR       %d", s_Version.Minor );
  defs_add( def, 0 );
  sprintf( def, "#define VARPP_PATCH       %d", s_Version.Patch );
  defs_add( def, 0 );
  sprintf( def, "#define VARPP_VERSION     0x%08X", (s_Version.Major << 24) | (s_Version.Minor << 16) | (s_Version.Patch << 8));
  defs_add( def, 0 );
  sprintf( def, "#define VARPP_VERSION_STR \"%d.%d.%d\"", s_Version.Major, s_Version.Minor, s_Version.Patch );
  defs_add( def, 0 );

  defs_add("#define VEC_DEFAULT 1", 0 );


  realpath( argv[1], fname );

  log_printf( LogInfo, 0, "Input File:  %s", fname );
  log_printf( LogInfo, 0, "Output path: %s", get_path( path, fname ));

  res = read_csv_file( &s_Data, fname );
  if( res == 0 ) {
    save_inc_file( s_Data, join_path( oname, path, "vardefs.h"));
    save_var_file( s_Data, join_path( oname, path, "vardef.inc"));
  }

  free( oname );
  free( path );
  free( fname );

  return res;
}

char *get_path( char* path, char const *fname ) {
  #define PATH_SEP '/'
  char *p;

  if( !path ) {
    return 0;
  }

  p = strrchr( fname, PATH_SEP );
  if( !p ) {
    *path = '\0';
  }
  else {
    int len;
    len = p - fname;
    memcpy( path, fname, len );
    *(path + len) = '\0';
  }

  return path;
}

char *join_path( char *oname, char const *path, char const *fname )
{
#ifdef _WIN32
  #define PATH_SEP '\\'
#else
  #define PATH_SEP '/'
#endif

  int have_path = 0;
  if( path && strlen(path) > 0) {
    have_path = 1;
  }

  if( have_path ) {
    sprintf( oname, "%s%c%s", path, PATH_SEP, fname );
  }
  else {
    strcpy( oname, fname );
  }

  return oname;
}

int is_hidden_scpi( char *s ) {
  return 0 == strcmp( s, "---" );
}

/**
 *
 *
 *
 */
int read_csv_file( DataItem **head, char * szFilename)
{
  int res;
  uint32_t uCols;
  FILE *fp;
  int line_nr = 0;

  char line[LineSize];
  CSV_BUF cols;

  res = 0;
  fp = fopen( szFilename, "r" );
  if( fp == NULL ) {
    log_printf( LogErr, 0, strerror( errno ));
    return -1;
  }

  loc_push( szFilename, line_nr );


  for(;;) {

    char *s = fgets( line, line_len, fp);
    if( !s ) {
      break;
    }

    strtrim( line, '\n' );
    strtrim( line, '\r' );
    line_nr ++;
    loc_set( line_nr );

    int n = sizeof(CSV_BUF);
    memset( cols, 0, n);

    uCols = MaxCsvColumns;
    split( line, ';', (char**) cols, LineSize, &uCols );

    if( cols[ColHnd][0] == '#' ) {
      char *p = skip_space( &cols[ColHnd][1] );
      if( 0 == strncmp("pragma", p, 6)) {
        handle_pragma( cols[ColHnd], loc_cur());
      }

      if( 0 == strncmp("define", p, 6)) {
        defs_add( cols[ColHnd], loc_cur());
      }

      continue;
    }

    size_t len = strlen(cols[ColHnd]);
    if((len == 0) || (strncmp( cols[ColHnd], s_Cfg.prefix, s_Cfg.prefix_len) != 0) ) {
      continue;
    }

    if( len > s_Stats.max_var_hnd_len ) {
      s_Stats.max_var_hnd_len = len;
    }

    DataItem *item = (DataItem *) calloc( sizeof(DataItem), 1 );
    if( !item ) {
      res = -1;
      break;
    }

    strcpy( item->hnd, cols[ColHnd] );
    strcpy( item->scpi, cols[ColScpi] );

    StringItem *si = strpool_Add( &s_StrPools[spScpi], cols[ColScpi], 0 );
    if( !si && !is_hidden_scpi(cols[ColScpi])) {
      log_printf( LogErr, 0, "SCPI %s already in use.", cols[ColScpi] );
      res = -2;
      break;
    }

    log_printf( LogDebug, 0, "Process %s with type %s", cols[ColHnd], cols[ColType] );
    int ret = get_type( cols[ColType], &item->type );
    if ( ret ) {
      log_printf( LogInfo, 0, "unknown datatype: %s", cols[ColType] );
      free( item );
      continue;
    }

    s_nVarCnt++;
    s_nTypeCnt[item->type & TYPE_MASK]++;

    ret = get_vector( cols[ColVector], &item->vec_items );
    if( ret < 0 ) {
        log_printf(LogErr, 0, "unknown vector: %s", cols[ColVector] );
        res = -3;
    }
    ret = get_storage( cols[ColStorage], &item->storage );
    if( ret < 0 ) {
        log_printf(LogErr, 0, "unknown storage: %s", cols[ColStorage] );
        res = -4;
    }
    ret = get_access( cols[ColAccess], &item->acc_rights );
    if( ret < 0 ) {
        log_printf(LogErr, 0, "unknown access: %s", cols[ColAccess] );
        res = -5;
    }
    ret = get_format( cols[ColFormat], &item->format );
    if( ret < 0 ) {
        log_printf(LogErr, 0, "unknown format: %s", cols[ColFormat] );
        res = -6;
    }

    item->type |= item->storage;
    if( item->vec_items > 1 ) {
      item->type |= TYPE_VECTOR;
    }

    int mask = (FLAG_LIMIT | FLAG_CLIP);
    if(( item->acc_rights & mask) == mask) {
      log_printf( LogWarn, loc_cur(), "%s: FLAG_LIMIT and FLAG_CLIP together do not make sense.", cols[ColScpi] );
    }

    switch( item->type & TYPE_MASK ) {
      case TYPE_ACTION:
        break;

      case TYPE_ENUM:
        parse_enum( item, uCols, &cols );
        break;

      case TYPE_FLOAT:
      case TYPE_DOUBLE:
      case TYPE_INT16:
      case TYPE_INT32:
        parse_number( item, uCols, &cols );
        break;

      case TYPE_STRING:
        parse_string( item, uCols, &cols );
        break;
    }

    LL_APPEND( *head, item );
  }

  fclose( fp );

  loc_pop();

  return res;
}

static void write_header( FILE *fp, char const *description ) {
  UNUSED_PARAM( description );

  char date[256];
  time_t t;
  struct tm *tmp;

  t = time(NULL);
  tmp = localtime(&t);
  
  strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", tmp);

  fprintf( fp, "/**\n"
           " * Generated from %s %d.%d.%d\n"
           " * \\date %s\n"
           " ******************************************************/\n\n",
           s_Version.Shortname,
           s_Version.Major, s_Version.Minor, s_Version.Patch,
           date );
}

/**
 *
 *
 *
 */
int save_inc_file( DataItem *head, char *szFilename )
{
  UNUSED_PARAM( head );
  int nRet;
  DataItem *item;
  DEF_ITER iter;

  nRet = 1;
  int i = 0;

  FILE *fp = fopen( szFilename, "w+");

  write_header( fp, 0 );

  defs_iter( &iter );
  for(;;) {
    DEF d;
    int ret = defs_next( &d, &iter );
    if( 0 == ret ) {
      break;
    }
    fprintf(fp, "#define %s %s\n", d.name, d.value );

  }

  fprintf(fp, "\n" );


  LL_FOREACH( head, item ) {
    int len = strlen( item->hnd );
    char *spaces = srepeat( ' ', 2 + s_Stats.max_var_hnd_len - len );
    fprintf(fp, "#define %s%s  0x%04x\n", item->hnd, spaces, i);
    i++;
  }

  fputs("/*** EOF *********/\n", fp );
  fclose( fp );

  return nRet;
}

/**
 *
 *
 *
 */
int save_var_file( DataItem *head, char *szFilename )
{
  int nRet;
  DataItem *item;
  int data_cnt[TYPE_LAST];
  int descr_cnt[TYPE_LAST];
  spool_iter iter;
  S16 scpi_idx = 0;
  int scpi_ofs = 0;

  memset( descr_cnt, 0, sizeof(descr_cnt));
  memset( data_cnt, 0, sizeof(data_cnt));

  nRet = 1;
  int i = 1;

  FILE *fp = fopen( szFilename, "w+");
  write_header( fp, 0 );

  fputs( "VAR_DESC const g_vars[] = {\n", fp );

  strpool_iter( &iter, &s_StrPools[spScpi] );
  for(;;) {
    StringItem *si;
    int ret;

    ret = strpool_next2( &iter, &si );
    if( !ret ) {
      break;
    }

    if( is_hidden_scpi( si->buf )) {
      continue;
    }

    si->offset = scpi_idx;
    scpi_idx += strlen( si->buf ) +1;
  }

  scpi_ofs = scpi_idx;
  LL_FOREACH( head, item ) {

    int len = strlen( item->hnd );
    char *spaces = srepeat( ' ', 2 + s_Stats.max_var_hnd_len - len );
    int type = item->type & TYPE_MASK;
    int flags = item->type & TYPE_FLAG;
    int data_idx = data_cnt[type];
    int descr_idx = descr_cnt[type];
    int incr_descr = 1;

    if( 0 == strcmp(item->hnd, "VAR_MODE_AUTO")) {
      incr_descr = 1;
    }

    if( TYPE_STRING == type ) {
      PP_DATA_STRING *p = &item->data.data_string;
      StringItem *si = strpool_Get( &s_StrPools[spStrings], p->def_value );

      if( flags & TYPE_CONST ) {
        data_idx = 0;
      }

      if( si->offset == -1 ) {
        si->offset = descr_cnt[type] + scpi_ofs;
      }

      descr_idx = si->offset;
    }
    else if ( TYPE_ENUM == type ) {
      PP_DATA_ENUM *d = &item->data.data_enum;
      char *buf = calloc( 1024, 1 );
      StringItem *si;

      serialize_enum( buf, 1024, d );
      si = strpool_Get( &s_EnumPool, buf );
      if( !si ) {
        log_printf( LogErr, 0, "enum not found in pool.");
        continue;
      }

      incr_descr = 0;
      if( si->offset == -1 ) {
        si->offset = descr_cnt[type];
        incr_descr = 1;
      }

      descr_idx = si->offset;
      free(buf);
    }

    if( i != 1 ) {
      fputs( ",\n", fp );
    }

    StringItem *si = strpool_Get( &s_StrPools[spScpi], item->scpi );
    scpi_idx = si->offset;

    fprintf(fp, "  { %s,%s 0x%04hx,"
                " 0x%04x, 0x%04x, 0x%04x, %d,"
                " % 4d, % 4d }",
             item->hnd, spaces, (U16)scpi_idx,
             item->type, item->vec_items, item->acc_rights, item->format,
             descr_idx, data_idx );
    i++;
    switch( type ) {
      case TYPE_INT16:
      case TYPE_INT32:
      case TYPE_FLOAT:
      case TYPE_DOUBLE:
        data_cnt[type] += item->vec_items;
        descr_cnt[type] += item->vec_items;
        break;

      case TYPE_STRING:
        {
          PP_DATA_STRING *p = (PP_DATA_STRING*) &item->data.data_string;
          size_t slen = strlen( p->def_value ) +1;
          descr_cnt[type] += slen;

          if( TYPE_CONST != flags ) {
            data_cnt[type] += sizeof(STRBUF) * item->vec_items;
          }
        }
        break;

      case TYPE_ENUM:
        if( incr_descr ) {
          descr_cnt[type] += 3* item->data.data_enum.cnt +2;
        }
        data_cnt[type] += item->vec_items;
        break;

      default:
        UNHANDLED_CASE( type );
    }
  }

  fputs( "\n};\n\n", fp );

  save_data_int( fp, head, "g_descr_int16", TYPE_INT16, 1 );
  save_data_int( fp, head, "g_data_int16", TYPE_INT16, 0 );

  save_data_int( fp, head, "g_descr_int32", TYPE_INT32, 1 );
  save_data_int( fp, head, "g_data_int32", TYPE_INT32, 0 );

  save_data_int( fp, head, "g_descr_float", TYPE_FLOAT, 1 );
  save_data_int( fp, head, "g_data_float", TYPE_FLOAT, 0 );

  save_data_int( fp, head, "g_descr_double", TYPE_DOUBLE, 1 );
  save_data_int( fp, head, "g_data_double", TYPE_DOUBLE, 0 );

  save_data_string( fp, head, "g_data_string", TYPE_STRING );
  save_data_const_string( fp, head, "g_data_const_string", TYPE_STRING );

  save_data_enum( fp, head, "g_data_enum", TYPE_ENUM );
  save_data_enum_mbr( fp, head, "g_enum_mbr", TYPE_ENUM );

  save_vc_def( fp, head );


  fputs("/*** EOF *********/\n", fp );
  fclose( fp );

  return nRet;
}

/*** save_data_int ***************************************************/
/**
 *   Save data or descriptor of integeral (int16,int32,float/double) variabes.
 *
 *   Writes the data of the variables to the file pointer.
 *   If descr_flag is set, then the content of the data items
 *   is always written to the file. When descr_flag is unset (= 0)
 *   then the #pragma init_data specifies if the data content
 *   is written to the file. This is useful to reduce flash space
 *   in embedded devices. The #pragma init_data on is useful for debugging.
 *
 *   @param fp         file pointer
 *   @param head       poiter to head of data items (aka variables)
 *   @param name       variable name
 *   @param type       data type of the data items
 *   @param descr_flag this flag tells if a descriptor or the data
 *                       of the variables show be written to fp
 */

int  save_data_int( FILE *fp, DataItem *head, char const *name, int type, int descr_flag ) {
  int i = 1;
  int init_data;
  int data_cnt = 0;
  char const *ztype;
  char const *zfmt;
  char const *zmod = (descr_flag) ? "const" : "";
  DataItem *item;

  switch( type ) {
    case TYPE_INT16:
      ztype = "DATA_S16";
      zfmt = "  { %d, %d, %d }";
      break;

    case TYPE_INT32:
      ztype = "DATA_S32";
      zfmt = "  { %d, %d, %d }";
      break;

    case TYPE_FLOAT:
      ztype = "DATA_F32";
      zfmt = "  { %f, %f, %f }";
      break;

    case TYPE_DOUBLE:
      ztype = "DATA_F64";
      zfmt = "  { %g, %g, %g }";
      break;

    default:
      log_printf( LogErr, 0, "%s:%d Type: %d not supported", __FILE__, __LINE__, type );
      return -1;
  }

  init_data = descr_flag || s_Cfg.init_data;
  if( init_data ) {
    fprintf( fp, "%s %s %s[] = {\n", ztype, zmod, name );
  }
  else
  {
    fprintf( fp, "%s %s[", ztype, name );
  }

  LL_FOREACH( head, item ) {
    int len;
    char *spaces;
    PP_DATA_NUMBER    *data_number    = &item->data.data_number;

    if((item->type & TYPE_MASK) != type ) {
      continue;
    }

    if( !init_data ) {
      data_cnt += item->vec_items;
      continue;
    }

    len = strlen( item->hnd );
    spaces = srepeat( ' ', 2 + s_Stats.max_var_hnd_len - len );

    if( i != 1 ) {
      fputs( ",\n", fp );
    }

    fprintf(fp, "  /* %s%s */", item->hnd, spaces );
    for( int j = 0; j < item->vec_items; j++ ) {
      S32 min, max, def_value;
      if( j > 0 ) {
        fputs( ",\n", fp );
        spaces = srepeat( ' ', 10 + s_Stats.max_var_hnd_len );
        fputs( spaces, fp );
      }

      switch( type ) {
        case TYPE_INT16:
        case TYPE_INT32:
          min = (S32)data_number->min;
          max = (S32)data_number->max; 
          def_value = (S32)data_number->def_value;
          fprintf( fp, zfmt, def_value, min, max );
          break;

        case TYPE_FLOAT:
        case TYPE_DOUBLE:
          fprintf( fp, zfmt, data_number->def_value, data_number->min, data_number->max );
          break;
      }
    }

    i++;
  }

  if( init_data ) {
    if( 1 == i ) {
      // No item was declared.
      // Emit a 0 value. This prohibts a warning from the compiler.
      fputs( "  { 0, 0, 0 }", fp );
    }
    fputs( "\n};\n\n", fp );
  }
  else {
    // No item was declared
    // Emit an array of size 1. This prohibits a warning from the compiler.
    fprintf( fp, "%d];\n\n", (0 == data_cnt) ? 1 : data_cnt );
  }
  

  return 0;
}

int  save_data_string( FILE *fp, DataItem *head, char const *name, int type )
{
  int i = 1;
  char const *ztype;
  DataItem *item;
  int init_data;
  int data_cnt = 0;

  switch( type ) {
    case TYPE_STRING:
      ztype = "DATA_STRING";
      break;

    default:
      log_printf( LogErr, 0, "STRING: Type: %d not supported", type );
      return -1;
  }

  init_data = s_Cfg.init_data;
  if( init_data ) {
    fprintf( fp, "%s %s[] = {\n", ztype, name );
  }
  else
  {
    fprintf( fp, "%s %s[", ztype, name );
  }

  LL_FOREACH( head, item ) {
    PP_DATA_STRING *data = &item->data.data_string;

    U16 _type = item->type & TYPE_MASK;
    U16 isConst = (item->type & TYPE_FLAG) & TYPE_CONST;

    if( _type != TYPE_STRING || (_type == TYPE_STRING && isConst)) {
      continue;
    }

    data_cnt += sizeof(STRBUF) * item->vec_items;
    if( 0 == init_data ) {
      continue;
    }

    if( i != 1 ) {
      fputs( ",\n", fp );
    }

    fprintf(fp, "  /* %s */\n  ", item->hnd );
    for( int j = 0; j < item->vec_items; j++ ) {

      if( j > 0 ) {
        fputs( ",\n  ", fp );
      }

      for( size_t k = 0; k < sizeof(STRBUF); k++ ) {
        char c = data->def_value[k];
        if( k > 0 ) {
          fputs( ", ", fp );
        }

        if( c && isascii( c )) {
          fprintf( fp, "'%c'", c );
        }
        else{
          fprintf( fp, "%#x", c );
        }
      }
    }

    i++;
  }

  if( init_data ) {
    fputs( "\n};\n\n", fp );
  }
  else {
    // No item was declared
    // Emit an array of size 1. This prohibits a warning from the compiler.
    fprintf( fp, "%d];\n", (0 == data_cnt) ? 1 : data_cnt );
  }

  return 0;  
}

int  save_data_const_string( FILE *fp, DataItem *head, char const *name, int type ) {
  int i = 1;
  char const *ztype;
  DataItem *item;

  switch( type ) {
    case TYPE_STRING:
      ztype = "DATA_STRING const";
      break;

    default:
      log_printf( LogErr, 0, "STRING: Type: %d not supported", type );
      return -1;
  }

  fprintf( fp, "%s %s[] = {\n", ztype, name );

  /* Write SCPI strings first. */
  LL_FOREACH( head, item ) {
    char *s = item->scpi;

    if( is_hidden_scpi( s )) {
      continue;
    }

    fputs( "  ", fp );
    while( *s ) {
      fprintf( fp, "'%c', ", *s );
      s++;
    }
    fputs( "0,\n", fp );
  }

  /* Write initial values of strings and const strings  */
  LL_FOREACH( head, item ) {
    PP_DATA_STRING *data = &item->data.data_string;

    U16 _type = item->type & TYPE_MASK;
    if( _type != TYPE_STRING ) {
      continue;
    }

    if( i != 1 ) {
      fputs( ", 0,\n", fp );
    }

    fprintf(fp, "  /* %s */\n  ", item->hnd );
    /* We only need one value in the descriptor. */
    for( int j = 0; j < 1; j++ ) {
      int k;

      if( j > 0 ) {
        fputs( ", 0,\n  ", fp );
      }

      k = 0;
      for( char *p = data->def_value; *p != '\0'; p++, k++ ) {
        if( k > 0 ) {
          fputs( ", ", fp );
        }

        if( isascii( *p )) {
          fprintf( fp, "'%c'", *p );
        }
        else {
          fprintf( fp, "%#x", *p );
        }
      }
    }

    i++;
  }
  fputs( ", 0\n};\n\n", fp );

  return 0;  
}

int enum_get_def( PP_DATA_ENUM const *data ) {
  ENUM_MBR_DESC *mbr = data->items;

  for( int k = 0; k < data->def_mbr; k++ ) {
    mbr = mbr->next;
  }
  return mbr->value;
}

int  save_data_enum( FILE *fp, DataItem *head, char const *name, int type )
{
  int i = 1;
  char const *ztype;
  DataItem *item;
  int init_data = s_Cfg.init_data;
  int data_cnt = 0;

  switch( type ) {
    case TYPE_ENUM:
      ztype = "S16";
      break;

    default:
      log_printf( LogErr, 0, "ENUM: Type: %d not supported", type );
      return -1;
  }

  if( init_data ) {
    fprintf( fp, "%s %s[] = {\n", ztype, name );
  }
  else {
    fprintf( fp, "%s %s[", ztype, name );
  }

  LL_FOREACH( head, item ) {
    PP_DATA_ENUM *data = &item->data.data_enum;

    if((item->type & TYPE_MASK) != type ) {
      continue;
    }

    data_cnt += item->vec_items;
    if( !init_data ) {
      continue;
    }

    if( i > 1 ) {
        fprintf( fp, ",\n" );
    }

    fprintf(fp, "  /* %s */\n  ", item->hnd );
    for( int j = 0; j < item->vec_items; j++ ) {
      int enm_def = enum_get_def( data );

      if( j > 0 ) {
        fprintf( fp, ", " );
      }

      fprintf(fp, "%d", enm_def );
    }

    i++;
  }
  
  if( init_data ) {
    fputs( "\n};\n\n", fp );
  }
  else {
    fprintf( fp, "%d];\n", data_cnt );
  }

  return 0;
}

int  save_data_enum_mbr( FILE *fp, DataItem *head, char const *name, int type )
{
  enum { kBufSize = 1024 };
  int i = 1;
  char const *ztype;
  DataItem *item;
  char *buf;

  switch( type ) {
    case TYPE_ENUM:
      ztype = "S16";
      break;

    default:
      log_printf( LogErr, 0, "ENUM: Type: %d not supported", type );
      return -1;
  }

  buf = (char*)calloc( BufSize, 1 );
  fprintf( fp, "%s const %s[] = {\n", ztype, name );
  LL_FOREACH( head, item ) {
    StringItem *si;
    PP_DATA_ENUM *data = &item->data.data_enum;
    ENUM_MBR_DESC *mbr = data->items;

    if((item->type & TYPE_MASK) != type ) {
      continue;
    }

    serialize_enum( buf, kBufSize, data );
    si = strpool_Get( &s_EnumPool, buf );
    if( !si ) {
      log_printf( LogErr, 0, "[%s:%d] enum for handle %s not found in enum pool.",
        __FUNCTION__, __LINE__, item->hnd );
      return -1;
    }

    if( si->offset == -2 )
    {
      continue;
    }

    if( i > 1 ) {
        fprintf( fp, ",\n" );
    }

    fprintf(fp, "  /* %s */\n  ", item->hnd );
    for( int j = 0; mbr; j++ ) {
      size_t len = strlen( mbr->hnd );
      char *spaces = srepeat( ' ', 2 + s_Stats.max_var_hnd_len - len );
      if( j == 0 ) {
        int def_val = enum_get_def( data );
        fprintf(fp, "%d, %d,\n    ", def_val, data->cnt );
      }
      else {
        fprintf(fp, ",\n    " );  
      }
      fprintf(fp, "%s,%s % 4d, % 4d", mbr->hnd, spaces, mbr->value, -1 );
      mbr = mbr->next;
    }
    si->offset = -2;

    i++;
  }

  fputs( "\n};\n\n", fp );
  free(buf);

  return 0;
}

int save_vc_def( FILE *fp, DataItem *head )
{
enum {
    kTypeConstString = TYPE_LAST,
    kTypeEnumMbr,

    kTypeLastPP
  };

  size_t cnt_total = 0;
  size_t cnt_data[kTypeLastPP+1] = {};
  size_t cnt_descr[kTypeLastPP+1] = {};
  DataItem *item;

  LL_FOREACH( head, item ) {
    int type = item->type & TYPE_MASK;
    int flags = item->type & TYPE_FLAG;


    if( type == TYPE_STRING && flags & TYPE_CONST ) {
      type = kTypeConstString;
    }

    cnt_total++;
    cnt_descr[type]++;

    switch( type ) {
      case kTypeConstString:
        {
          PP_DATA_STRING *p = (PP_DATA_STRING*) &item->data.data_string;
          cnt_data[type] +=  strlen( p->def_value ) * item->vec_items +1;
          cnt_descr[type]++;
        }
        break;

      case TYPE_ENUM:
        {
          cnt_data[type] += item->vec_items;
          cnt_descr[type] += 1 + item->data.data_enum.cnt * 3;
        }
        break;

      default:
        cnt_data[type] += item->vec_items;
        cnt_descr[type]++;
    }

  }


  fputs( "VC_DATA g_var_data = {\n", fp );
  fprintf( fp, "  g_vars,\n"
               "  %zu,\n"
               ""
               "  g_descr_int16,\n"
               "  %zu,\n"
               "  g_data_int16,\n"
               "  %zu,\n"
               "  g_descr_int32,\n"
               "  %zu,\n"
               "  g_data_int32,\n"
               "  %zu,\n"
               "  g_data_string,\n"
               "  %zu,\n"
               "  g_data_const_string,\n"
               "  %zu,\n"
               "  g_data_enum,\n"
               "  %zu,\n"
               "  g_enum_mbr,\n"
               "  %zu,\n"
               "  g_descr_float,\n"
               "  %zu,\n"
               "  g_data_float,\n"
               "  %zu,\n"
               "  g_descr_double,\n"
               "  %zu,\n"
               "  g_data_double,\n"
               "  %zu,\n"
               "};\n",
               cnt_total,
               cnt_descr[TYPE_INT16],
               cnt_data[TYPE_INT16],
               cnt_descr[TYPE_INT32],
               cnt_data[TYPE_INT32],

               cnt_data[TYPE_STRING],
               cnt_data[kTypeConstString],

               cnt_data[TYPE_ENUM],
               cnt_descr[kTypeEnumMbr],

               cnt_descr[TYPE_FLOAT],
               cnt_data[TYPE_FLOAT],

               cnt_descr[TYPE_DOUBLE],
               cnt_data[TYPE_DOUBLE]
         );
    return 0;
}

#define _MAP( x ) { str(x), x }
static int map_search( Map_t const *map, size_t n, char const *needle )
{
  unsigned int  i;

  for (i=0; i<n; i++) {
    if ( strcmp(needle, map->szKey) == 0 ) {
      return (int)i;
    }
    map++;
  }
  return -1;
}

/**
 *
 *
 *
 */
int  get_type( char *pType , int *pValue )
{
  static Map_t Types[] = {
    _MAP( TYPE_INT16 ),
    _MAP( TYPE_INT32 ),

    _MAP( TYPE_FLOAT ),
    _MAP( TYPE_DOUBLE ),
    _MAP( TYPE_ENUM ),
    _MAP( TYPE_STRING ),
    _MAP( TYPE_ACTION ),
  };

  int i = map_search( Types, countof(Types), pType );

  if( i > -1 ) {
    *pValue = Types[i].nValue;
    return 0;
  }
  return -1;
}

/*** get_vector *************************************************************/
/**
 *
 *
 *
 */
int  get_vector( char *name, int *value )
{
  DEF* def = defs_get( name );
  int n;
  char *endp;

  if( !def ) {
    return -1;
  }

  errno = 0;

  n = strtol( def->value, &endp, 0 );
  if( errno != 0 || *endp != '\0' ) {
    return -2;
  }

  *value = n;
  return 0;
}

/**
 *
 *
 *
 */
int  get_access( char *pAccess, int *pValue)
{
  static Map_t Access[] = {
    _MAP( REQ_ADMIN ),
    _MAP( FLAG_LIMIT),
    _MAP( FLAG_CLIP ),
  };

  int result = -1;
  CSV_BUF items = {};
  uint32_t col_cnt = MaxCsvColumns;
  split( pAccess, ',', (char**)items, LineSize, &col_cnt );


  for( uint32_t j = 0; j < col_cnt; j++ ) {
    int i = map_search( Access, countof(Access), skip_space(items[j]));

    if( i > -1 ) {
      *pValue |= Access[i].nValue;
      result = 0;
    }
    else {
      errno = 0;
      i = strtoul( pAccess, 0, 0 );
      if( 0 == errno ) {
        *pValue = i;
        result = 0;
      }
      else {
        return -1;
      }
    }
  }

  return result;
}

/**
 *
 *
 *
 */
int  get_storage( char * pStorage, int *pValue)
{
  static Map_t Storage[] = {
    _MAP( RAM_VOLATILE ),
    _MAP( FLASH ),
    _MAP( EEPROM ),
  };

  int i = map_search( Storage, countof(Storage), pStorage );

  if( i > -1 ) {
    *pValue = Storage[i].nValue;
    return 0;
  }

  return -1;
}

static int  get_format( char *fmt, int *value) {

  static Map_t format[] = {
    _MAP( FMT_DEFAULT ),
    _MAP( FMT_PREC_1 ),
    _MAP( FMT_PREC_2 ),
    _MAP( FMT_PREC_3 ),
    _MAP( FMT_PREC_4 ),
    _MAP( FMT_SCI ),
    _MAP( FMT_HEX2 ),
    _MAP( FMT_HEX4 ),
    _MAP( FMT_HEX8 ),
    _MAP( FMT_DATE ),
  };

  int i = map_search( format, countof(format), fmt );

  if( i > -1 ) {
    *value = format[i].nValue;
    return 0;
  }

  return -1;
}

#define CSV_COL( _buf, _col ) ((char*)(*_buf)[_col])

static int parse_string( DataItem *item, size_t col_cnt, CSV_BUF *cols )
{
  enum {
    colModifier = ColCommonLast,
    colValue
  };
  char *s;
  StringItem *si;

  if( col_cnt < colModifier ) {
    log_printf( LogErr, 0, "Not enough columns for variable %s.", CSV_COL(cols, 0 ));
    return -1;
  }

  PP_DATA_STRING *ds = &item->data.data_string;
  if( 0 == strcmp("CONST", CSV_COL(cols, colModifier ))) {
    item->type |= TYPE_CONST;
  }

  s = CSV_COL(cols, colValue);
  strcpy( ds->def_value, s );

  si = strpool_Add( &s_StrPools[spStrings], s, 0 );
  if( si ) {
    si->constant = (item->type & TYPE_CONST) ? 1 : 0;
  }

  return 0;
}

/*** strton *****************************************************************/
/**
 *   Read string and format as number, ie. double.
 * 
 *   Accepts hex, decimal an floating point strings.
 * 
 *   The syntax is adopted from the strtol and strtod functions.
 * 
 *   @param p      string
 *   @param endp   see strtod and strtol
 */
static F64 strton( char const *p, char **endp ) {
  F64 value;

  if( 0 == strncmp( p, "0x", 2 ) || 0 == strncmp( p, "0X", 2 )) {
    value = strtol( p, endp, 0 );
  }
  else {
    value = strtod( p, endp );
  }
  
  return value;
}

static int parse_number( DataItem *item, size_t col_cnt, CSV_BUF *cols )
{
  enum {
    colDefault = ColCommonLast,
    colMin,
    colMax
  };

  if( col_cnt < colDefault ) {
    log_printf( LogErr, 0, "Not enough columns for variable %s.", CSV_COL(cols, 0 ));
    return -1;
  }

  PP_DATA_NUMBER *d = &item->data.data_number;
  d->def_value = strton( CSV_COL(cols, colDefault), 0 );
  d->min = strton( CSV_COL(cols, colMin), 0 );
  d->max = strton( CSV_COL(cols, colMax), 0 );

  return 0;
}


/** parse_enum **************************************************************/
/**
 *  @param item
 *  @param col_cnt
 *  @param cols 
 */
static int parse_enum( DataItem *item, size_t col_cnt, CSV_BUF *cols )
{
  enum { kBufSize = 512 };
  UNUSED_PARAM( col_cnt );

  CSV_BUF Buf;
  PP_DATA_ENUM *d;
  
  char *es = calloc( BufSize, 1 );

  enum {
    colFirstMbr = ColCommonLast
  };

  if( col_cnt < colFirstMbr ) {
    log_printf( LogErr, 0, "Not enough columns for variable %s.", CSV_COL(cols, 0 ));
    free( es );
    return -1;
  }

  d = &item->data.data_enum;
  d->def_mbr = 0;
  d->cnt = 0;
  d->items = NULL;

  int i = colFirstMbr;
  for( ; ; i++) {
    ENUM_MBR_DESC *mbr;
    uint32_t cnt;

    char *s = strtrim( CSV_COL( cols, i ), ' ');
    if( 0 == strlen( s )) {
      break;
    }

    mbr = (ENUM_MBR_DESC *)calloc( sizeof(ENUM_MBR_DESC), 1 );
    LL_APPEND( d->items, mbr );

    memset( Buf, 0, sizeof(Buf));
    cnt = MaxCsvColumns;
    split( s, '=', (char**)Buf, LineSize, &cnt );

    s = CSV_COL( &Buf, 0 );
    if( ':' == *s ) {
      d->def_mbr = i - colFirstMbr;
      s++;
    }
    strcpy( mbr->hnd, s );

    mbr->value = i - colFirstMbr;
    if( cnt > 1 ) {
      s = CSV_COL( &Buf, 1 );
      errno = 0;
      mbr->value = strtol( s, 0, 0 );
      if( errno != 0 ) {
        log_printf( LogErr, 0, "Invalid number for enum member %s in variable %s", mbr->hnd, CSV_COL(cols, 0 ));
      }
    }

    if( cnt > 2 ) {
      StringItem *si;
      s = CSV_COL( &Buf, 2 );
      s = skip_space( s );
      strcpy( mbr->string, s );

      si = strpool_Add( &s_StrPools[spStrings], s, 0 );
      if( si ) {
        si->constant = 1;
      }
    }

    d->cnt++;
  }

  serialize_enum( es, (size_t)kBufSize, d );
  strpool_Add( &s_EnumPool, es, d );

  free(es);

  return 0;
}

int serialize_enum( char *Buf, size_t BufSize, PP_DATA_ENUM *enm ) {
  char *s = Buf;
  size_t rest = BufSize;
  ENUM_MBR_DESC *mbr = enm->items;

  for( int i = 0; i < enm->cnt; i++ ) {
    int n;
    
    // Include the default value into the descriptor.
    // This leads to different descriptors for
    // enums that are mostly identical and where some
    // space could be saved.
    // But excluding the default value would require
    // to store the default values somewhere else which
    // I don't want to. The overhead is - I hope - neglegible.
    if( mbr->value == enm->def_mbr ) {
      *s = ':';
      s++;
    }

    n = snprintf( s, rest, "%s=%d", mbr->hnd, mbr->value );
    if( n <= 0 ) {
      log_printf( LogErr, 0, "%s: invalid length", __FUNCTION__ );
      return 0;
    }

    s += n;
    rest -= n;

#if 0
    // Don't append the symbol.
    // This could even further reduce the number
    // of enum mbr descriptors.
    if( strlen(mbr->string) > 0 ) {
      n = snprintf( s, rest, "=%s", mbr->string );
      if( n <= 0 ) {
        log_printf( LogErr, 0, "%s: invalid length", __FUNCTION__ );
        return 0;
      }
      s += n;
      rest -= n;
    }
#endif
    n = snprintf( s, rest, ";" );
    s += n;
    rest -= n;

    mbr = mbr->next;
  }

  return (int)(s - BufSize);
}

static int find_opt( char const *p, char const **opt_list, char **endp, int *idx ) {

  int found = 0;
  int i = 0;

  char *needle;
  char *sep;
  size_t len;

  needle = strdup( p );
  if( !needle ) {
    return 0;
  }

  sep = strchr( needle, ' ');
  len = sep ? (size_t) (sep - needle) : strlen(needle);
  needle[len] = '\0';

  for( char const **opt = opt_list; *opt != NULL; opt++, i++ ) {

    if( 0 == strcmp( *opt, needle )) {
      *idx = i;
      *endp = (char*) (p+len+1);
      found = 1;
      break;
    }
  }

  free( needle );

  if( !found ) {
    *endp = (char*) p;
  }

  return found;
}

void handle_pragma( char const *line, LOC const *loc ) {
  enum { kSection, kPrefix, kInitData };
  static char const* Pragmas[] = { "section", "prefix", "init_data", NULL };

  char *endp = NULL;
  int idx = -1;

  int n = find_opt( line + 8, (char const **)&Pragmas, &endp, &idx );
  if( !n ) {
    const int buf_size = PATH_MAX * 1.5;
    char *buf = (char*)calloc( buf_size, sizeof(char));
    loc_fmt( buf, buf_size, loc );
    log_printf( LogErr, 0, "%s Unknown pragma: %s.", buf, endp );
    free( buf );
    return;
  }

  switch( idx ) {

    case kSection:
      if( 0 == strcmp( endp, "var")) {
        s_Cfg.section = kSecVariables;
      }
      break;

    case kPrefix:
      s_Cfg.prefix = strdup( endp );
      s_Cfg.prefix_len = strlen( s_Cfg.prefix );
      break;

    case kInitData:
      s_Cfg.init_data = (0 == strcmp( endp, "on")) ? 1 : 0;
      break;

    default:
      UNHANDLED_CASE( idx );
  }
}
