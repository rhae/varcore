/*
 * varpp.c
 *
 *  Created on: 25.12.2010
 *      Author: hae
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include <limits.h>

#include "defs.h"
#include "loc.h"
#include "log.h"
#include "utils.h"
#include "utlist.h"
#include "uthash.h"
#include "../../lib/varcore.h"

#ifndef UNUSED_PARAM
#  define UNUSED_PARAM(x) (void)(x)
#endif

#ifndef UNHANDLED_CASE
#  define UNHANDLED_CASE(x) log_printf( LogErr, "%s:%d Unhandled case %d", __FUNCTION__, __LINE__, x );
#endif

#ifndef countof
# define countof(x) ( sizeof(x) / sizeof(x[0]) )
#endif

enum {BufSize = 256};

typedef struct _DATA_INT {
  int def_value;
  int min;
  int max;
} PP_DATA_INT;

typedef struct _DATA_DOUBLE {
  double def_value;
  double min;
  double max;
  double prec;
} PP_DATA_DOUBLE;

typedef struct _ENUM_MBR_DESC {
  int    value;
  STRBUF hnd;
  STRBUF string;
  struct _ENUM_MBR_DESC *next;
} ENUM_MBR_DESC;

typedef struct _DATA_ENUM {
  int def_mbr;
  int cnt;
  ENUM_MBR_DESC *items;
} PP_DATA_ENUM;

typedef struct _PP_DATA_STRING {
  STRBUF def_value;
  int flags;
} PP_DATA_STRING;

typedef struct _STRINGS {
  STRBUF str;                /* key for hash */
  int cnt;
  int constant;
  UT_hash_handle hh;         /* makes this structure hashable */
} STRINGS;

typedef struct _DATA_ITEM {
  STRBUF hnd;
  STRBUF scpi;
  int acc_rights;
  int fmt;
  int segment;
  int type;
  int vec_items;
  int storage;
  
  union _DATA {
    PP_DATA_INT data_int;
    PP_DATA_DOUBLE data_double;
    PP_DATA_ENUM data_enum;
    PP_DATA_STRING data_string;
  } data;

  struct _DATA_ITEM *next;
} DATA_ITEM;

typedef struct {
  char     szKey[BufSize];
  int32_t  nValue;
} Map_t;


static int  GetType( char *, int *);
static int  get_vector( char *, int *);
static int  GetAccess( char *, int *);
static int  GetStorage( char *, int *);

enum {
  line_len  = 255,
  LineSize = line_len+1
};
enum {
  MaxCsvColumns = 16
};
typedef char CSV_BUF[MaxCsvColumns][LineSize];

static int parse_int( DATA_ITEM *, size_t, CSV_BUF* );
static int parse_double( DATA_ITEM *, size_t, CSV_BUF* );
static int parse_string( DATA_ITEM *, size_t, CSV_BUF* );
static int parse_enum( DATA_ITEM *, size_t, CSV_BUF* );

int  read_csv_file( DATA_ITEM **, char * );
int  save_inc_file( DATA_ITEM *, char * );
int  save_var_file( DATA_ITEM *, char * );
int  save_data_int( FILE *fp, DATA_ITEM *head, char const *name, int type );
int  save_data_float( FILE *fp, DATA_ITEM *head, char const *name, int type );
int  save_data_string( FILE *fp, DATA_ITEM *head, char const *name, int type );
int  save_data_enum( FILE *fp, DATA_ITEM *head, char const *name, int type );
int  save_vc_def( FILE *fp, DATA_ITEM *head );

int  save_descr_int( FILE *fp, DATA_ITEM *head, char const *name, int type );

char *get_path( char *path, char const *fname );
char *join_path( char *oname, char const *path, char const *fname );

#if 0
static int AddDefaultValue( char *, int , int *);
static void PrintDefaultValues( FILE *, DefaultValue_t *, char *);
#endif

typedef struct _STATS {
  size_t max_var_hnd_len;
  size_t max_string_hnd_len;
} STATS;

DATA_ITEM *s_Data = NULL;
STRINGS   *s_Strings = NULL;
STATS      s_Stats;

#if 0
static DefaultValue_t *s_pInt16DefVal = NULL;
static DefaultValue_t *s_pInt32DefVal = NULL;
static DefaultValue_t *s_pFloatDefVal = NULL;
#endif

static int s_nVarCnt = 0;
static int s_nTypeCnt[16] = { 0 };

typedef struct {
  S32 Major;
  S32 Minor;
  S32 Patch;
  S8  *Name;
  S8  *Copyright;
  S8  *Date;
} VERSION;

static VERSION s_Version = {
  1, 1, 0,
  "Variable preprocessor",
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

  memset( &s_Stats, 0, sizeof(STATS));

  defs_init();
  loc_init();
  log_init( LogInfo );

  puts_version();

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

  log_printf( LogInfo, "Input File:  %s", fname );
  log_printf( LogInfo, "Output path: %s", get_path( path, fname ));

  read_csv_file( &s_Data, fname );

  
  save_inc_file( s_Data, join_path( oname, path, "vardefs.h"));

  save_var_file( s_Data, join_path( oname, path, "vardef.inc"));

  return 0;
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

/**
 *
 *
 *
 */
int read_csv_file( DATA_ITEM **head, char * szFilename)
{
  
  enum {
    ColHnd = 0,
    ColScpi = 1,
    ColCanopen = 2,
    ColAccess = 3,
    ColStorage = 4,
    ColVector = 5,
    ColType = 6,
  };

  int nRet;
  uint32_t uCols;
  FILE *fp;
  int line_nr = 0;

  char line[LineSize];
  CSV_BUF cols;
  // uint16_t uFlags;

  nRet = 1;
  fp = fopen( szFilename, "r" );
  if( fp == NULL ) {
    log_printf( LogErr, strerror( errno ));
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
    

    if( 0 == strncmp("#define", cols[ColHnd], 6)) {
      defs_add( cols[ColHnd], loc_cur());
    }

    size_t len = strlen(cols[ColHnd]);
    if((len == 0) || (strncmp( cols[ColHnd], "VAR_", 4) != 0) ) {
      continue;
    }

    if( len > s_Stats.max_var_hnd_len ) {
      s_Stats.max_var_hnd_len = len;
    }

    DATA_ITEM *item = (DATA_ITEM *) calloc( sizeof(DATA_ITEM), 1 );

    strcpy( item->hnd, cols[ColHnd] );
    strcpy( item->scpi, cols[ColScpi] );

    log_printf( LogDebug, "Process %s with type %s", cols[ColHnd], cols[ColType] );
    nRet = GetType( cols[ColType], &item->type );
    if ( nRet ) {
      log_printf( LogInfo, "unknown datatype: %s", cols[ColType] );
      continue;
    }

    s_nVarCnt++;
    s_nTypeCnt[(item->type >> 8) & 0xf]++;

    get_vector( cols[ColVector], &item->vec_items );
    GetStorage( cols[ColStorage], &item->storage );
    GetAccess( cols[ColAccess], &item->acc_rights );

    if( item->vec_items > 1 ) {
      item->type |= TYPE_VECTOR;
    }

    // uFlags = nType | nStorage | nAccess | nVector;

    switch( item->type & MSK_TYPE ) {
      case TYPE_ACTION:
        break;

      case TYPE_ENUM:
        parse_enum( item, uCols, &cols );
        break;

      case TYPE_FLOAT:
      case TYPE_DOUBLE:
        parse_double( item, uCols, &cols );
        break;

      case TYPE_INT16:
      case TYPE_INT32:
        parse_int( item, uCols, &cols );
        break;

      case TYPE_STRING:
        parse_string( item, uCols, &cols );
        break;
    }

    if( !head ) {
      *head = item;
    }
    else {
      LL_APPEND( *head, item );
    }
  }

  fclose( fp );

  loc_pop();

  return nRet;
}


/**
 *
 *
 *
 */
int save_inc_file( DATA_ITEM *head, char *szFilename )
{
  UNUSED_PARAM( head );
  int nRet;
  DATA_ITEM *item;
  char *spaces;
  DEF_ITER iter;

  nRet = 1;
  int i = 0;

  FILE *fp = fopen( szFilename, "w+");

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
    spaces = srepeat( ' ', 2 + s_Stats.max_var_hnd_len - len );
    fprintf(fp, "#define %s%s  0x%04x\n", item->hnd, spaces, i);
    i++;
  }

#if 0
  fputs( "\n/*** Strings ****/\n", fp );

  STRINGS *cur, *tmp;
  i = 0;
  HASH_ITER( hh, s_Strings, cur, tmp ) {
    int len = strlen( cur->str );
    spaces = srepeat( ' ', 2 + s_Stats.max_string_hnd_len - len );
    fprintf(fp, "#define %s%s  0x%04x\n", cur->str, spaces, i);
    i++;
  }
#endif

  fputs("/*** EOF *********/\n", fp );
  fclose( fp );

  return nRet;
}

/**
 *
 *
 *
 */
int save_var_file( DATA_ITEM *head, char *szFilename )
{
  UNUSED_PARAM( head );
  int nRet;
  DATA_ITEM *item;
  int data_cnt[MSK_TYPE+1] = {};
  int descr_cnt[MSK_TYPE+1] = {};

  memset(descr_cnt, 0, sizeof(descr_cnt));

  nRet = 1;
  int i = 1;

  FILE *fp = fopen( szFilename, "w+");

  fputs( "VAR_DESC g_vars[] = {\n", fp );

  LL_FOREACH( head, item ) {

    int len = strlen( item->hnd );
    char *spaces = srepeat( ' ', 2 + s_Stats.max_var_hnd_len - len );
    int type = item->type & MSK_TYPE;

    if( i != 1 ) {
      fputs( ",\n", fp );
    }

    fprintf(fp, "  { %s,%s 0x%04x, 0x%04x, 0x%04x, % 4d, % 4d }",
             item->hnd, spaces, item->type, item->vec_items, item->acc_rights, descr_cnt[type], data_cnt[type] );
    i++;
    switch( type ) {
      case TYPE_INT16:
      case TYPE_INT32:
      case TYPE_FLOAT:
      case TYPE_DOUBLE:
        data_cnt[type] += item->vec_items;
        break;

      case TYPE_STRING:
        data_cnt[type] += sizeof(STRBUF) * item->vec_items;
        break;

      case TYPE_ENUM:
        data_cnt[type] += 3 * item->data.data_enum.cnt;
        break;

      default:
        UNHANDLED_CASE( type );
    }
    descr_cnt[type]++;
  }

  fputs( "\n};\n\n", fp );

  save_descr_int( fp, head, "g_descr_int16", TYPE_INT16 );
  save_data_int( fp, head, "g_data_int16", TYPE_INT16 );
  
  save_descr_int( fp, head, "g_descr_int32", TYPE_INT32 );
  save_data_int( fp, head, "g_data_int32", TYPE_INT32 );

  save_data_float( fp, head, "g_data_float", TYPE_FLOAT );
  save_data_float( fp, head, "g_data_double", TYPE_DOUBLE );

  save_data_string( fp, head, "g_data_string", TYPE_STRING );

  save_data_enum( fp, head, "g_data_enum", TYPE_ENUM );

  save_vc_def( fp, head );
  

  fputs("/*** EOF *********/\n", fp );
  fclose( fp );

  return nRet;
}

int  save_descr_int( FILE *fp, DATA_ITEM *head, char const *name, int type ) {
  int i = 1;
  char const *ztype;
  char const *zfmt;
  int size;
  DATA_ITEM *item;

  switch( type ) {
    
    case TYPE_INT16:
      ztype = "DATA_S16";
      zfmt = "  { %d, %d, %d }";
      size = 3 * sizeof(short);
      break;

    case TYPE_INT32:
      ztype = "DATA_S32";
      zfmt = "  { %d, %d, %d }";
      size = 3 * sizeof(long);
      break;

    default:
      log_printf( LogErr, "INT: Type: %d not supported", type );
      return -1;
  }

  fprintf( fp, "%s const %s[] = {\n", ztype, name );
  LL_FOREACH( head, item ) {
    int len;
    char *spaces;
    PP_DATA_INT *data = &item->data.data_int;

    if((item->type & MSK_TYPE) != type ) {
      continue;
    }

    len = strlen( item->hnd );
    spaces = srepeat( ' ', 2 + s_Stats.max_var_hnd_len - len );

    if( i != 1 ) {
      fputs( ",\n", fp );
    }

    fprintf(fp, "  /* %s%s */", item->hnd, spaces );
    fprintf( fp, zfmt, data->def_value, data->min, data->max );
    
    i++;
  }
  fputs( "\n};\n\n", fp );

  return 0;
}

int  save_data_int( FILE *fp, DATA_ITEM *head, char const *name, int type ) {
  int i = 1;
  char const *ztype;
  char const *zfmt;
  int size;
  DATA_ITEM *item;

  switch( type ) {
    case TYPE_INT16:
      ztype = "DATA_S16";
      zfmt = "  { %d, %d, %d }";
      size = 3 * sizeof(short);
      break;

    case TYPE_INT32:
      ztype = "DATA_S32";
      zfmt = "  { %d, %d, %d }";
      size = 3 * sizeof(long);
      break;

    default:
      log_printf( LogErr, "Type: %d not supported", type );
      return -1;
  }

  fprintf( fp, "%s %s[] = {\n", ztype, name );
  LL_FOREACH( head, item ) {
    int len;
    char *spaces;
    PP_DATA_INT *data = &item->data.data_int;

    if((item->type & MSK_TYPE) != type ) {
      continue;
    }

    len = strlen( item->hnd );
    spaces = srepeat( ' ', 2 + s_Stats.max_var_hnd_len - len );

    if( i != 1 ) {
      fputs( ",\n", fp );
    }

    fprintf(fp, "  /* %s%s */", item->hnd, spaces );
    for( int j = 0; j < item->vec_items; j++ ) {
      if( j > 0 ) {
        fputs( ",\n", fp );
        spaces = srepeat( ' ', 10 + s_Stats.max_var_hnd_len );
        fputs( spaces, fp );
      }

      fprintf( fp, zfmt, data->def_value, data->min, data->max );
    }
    
    i++;
  }
  fputs( "\n};\n\n", fp );

  return 0;
}

int  save_data_float( FILE *fp, DATA_ITEM *head, char const *name, int type ) {
  int i = 1;
  char const *ztype;
  char const *zfmt;
  int size;
  DATA_ITEM *item;

  switch( type ) {
    case TYPE_FLOAT:
      ztype = "DATA_F32";
      zfmt = "  { %f, %f, %f }";
      size = 3 * sizeof(float);
      break;

    case TYPE_DOUBLE:
      ztype = "DATA_F64";
      zfmt = "  { %g, %g, %g }";
      size = 3 * sizeof(double);
      break;

    default:
      log_printf( LogErr, "FLOAT: Type: %d not supported", type );
      return -1;
  }

  fprintf( fp, "%s %s[] = {\n", ztype, name );
  LL_FOREACH( head, item ) {
    int len;
    char *spaces;
    PP_DATA_DOUBLE *data = &item->data.data_double;

    if((item->type & MSK_TYPE) != type ) {
      continue;
    }

    len = strlen( item->hnd );
    spaces = srepeat( ' ', 2 + s_Stats.max_var_hnd_len - len );

    if( i != 1 ) {
      fputs( ",\n", fp );
    }

    fprintf(fp, "  /* %s%s */", item->hnd, spaces );
    for( int j = 0; j < item->vec_items; j++ ) {
      if( j > 0 ) {
        fputs( ",\n", fp );
        spaces = srepeat( ' ', 10 + s_Stats.max_var_hnd_len );
        fputs( spaces, fp );
      }

      fprintf( fp, zfmt, data->def_value, data->min, data->max );
    }
    
    i++;
  }
  fputs( "\n};\n\n", fp );

  return 0;
}

int  save_data_string( FILE *fp, DATA_ITEM *head, char const *name, int type )
{
  int i = 1;
  char const *ztype;
  DATA_ITEM *item;

  switch( type ) {
    case TYPE_STRING:
      ztype = "DATA_STRING";
      break;

    default:
      log_printf( LogErr, "STRING: Type: %d not supported", type );
      return -1;
  }

  fprintf( fp, "%s %s[] = {\n", ztype, name );
  LL_FOREACH( head, item ) {
    PP_DATA_STRING *data = &item->data.data_string;

    if((item->type & MSK_TYPE) != type ) {
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
        if( k > 0 ) {
          fputs( ", ", fp );
        }
        fprintf( fp, "0x%02x", data->def_value[k] );
      }
    }
    
    i++;
  }
  fputs( "\n};\n\n", fp );

  return 0;  
}

int  save_data_enum( FILE *fp, DATA_ITEM *head, char const *name, int type )
{
  int i = 1;
  char const *ztype;
  DATA_ITEM *item;

  switch( type ) {
    case TYPE_ENUM:
      ztype = "uint16_t";
      break;

    default:
      log_printf( LogErr, "ENUM: Type: %d not supported", type );
      return -1;
  }

  fprintf( fp, "%s %s[] = {\n", ztype, name );
  LL_FOREACH( head, item ) {
    PP_DATA_ENUM *data = &item->data.data_enum;

    if((item->type & MSK_TYPE) != type ) {
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

      fprintf(fp, "%d, %d, ", data->cnt, data->def_mbr );

      ENUM_MBR_DESC *mbr = data->items;
      size_t k = 0;
      for(; mbr != NULL; mbr = mbr->next, k++ ) {
        if( k > 0 ) {
          fputs( ", ", fp );
        }
        
        fprintf( fp, "%s, %d", mbr->hnd, mbr->value );
      }
    }
    
    i++;
  }
  fputs( "\n};\n\n", fp );

  return 0;
}

int save_vc_def( FILE *fp, DATA_ITEM *head )
{
  size_t cnt_total = 0;
  size_t cnt_descr[TYPE_LAST] = {};
  size_t cnt_data[TYPE_LAST] = {};
  DATA_ITEM *item;

  LL_FOREACH( head, item ) {
    cnt_total++;
    cnt_descr[item->type & MSK_TYPE]++;
    cnt_data[item->type & MSK_TYPE] += item->vec_items;
  }
  

  fputs( "VC_DATA g_var_data = {\n", fp );
  fprintf( fp, "  g_vars,\n"
               "  %ld,\n"
               ""
               "  g_descr_int16,\n"
               "  %ld,\n"
                "  g_data_int16,\n"
               "  %ld,\n"
               "  g_descr_int32,\n"
               "  %ld,\n"
               "  g_data_int32,\n"
               "  %ld,\n"
               "  g_data_string,\n"
               "  %ld\n"
               "};\n",
               cnt_total,
               cnt_descr[TYPE_INT16],
               cnt_data[TYPE_INT16],
               cnt_descr[TYPE_INT32],
               cnt_data[TYPE_INT32],

               cnt_data[TYPE_STRING]
         );
    return 0;
}

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
int  GetType( char *pType , int *pValue )
{
  static Map_t Types[] = {

      {"TYPE_INT16", TYPE_INT16},
      {"TYPE_INT32", TYPE_INT32},

      {"TYPE_FLOAT", TYPE_FLOAT},
      {"TYPE_DOUBLE", TYPE_DOUBLE},
      {"TYPE_ENUM", TYPE_ENUM},
      {"TYPE_STRING", TYPE_STRING},
      {"TYPE_ACTION", TYPE_ACTION},
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
int  GetAccess( char *pAccess, int *pValue)
{
  static Map_t Access[] = {
      {"REQ_ADMIN", REQ_ADMIN}
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
int  GetStorage( char * pStorage, int *pValue)
{
  static Map_t Storage[] = {
      {"RAM_VOLATILE", RAM_VOLATILE},
      {"FLASH", FLASH},
      {"EEPROM", EEPROM},

  };

  int i = map_search( Storage, countof(Storage), pStorage );

  if( i > -1 ) {
    *pValue = Storage[i].nValue;
    return 0;
  }

  return -1;
}



static int string_add( char const *s, int n ) {
  STRINGS *str = (STRINGS*)calloc( sizeof(STRINGS), 1 );

  if( !str ) {
    return 0;
  }

  size_t len = strlen( s );
  if( len > s_Stats.max_string_hnd_len ) {
    s_Stats.max_string_hnd_len = len;
  }

  strcpy( str->str, s );
  str->cnt = n;
  str->constant = 0;
  HASH_ADD_STR( s_Strings, str, str );

  return 1;
}

#define CSV_COL( _buf, _col ) ((char*)(*_buf)[_col])

static int parse_string( DATA_ITEM *item, size_t col_cnt, CSV_BUF *cols )
{
  enum {
    colModifier = 7,
    colValue = 8
  };

  if( col_cnt < colModifier ) {
    log_printf( LogErr, "Not enough columns for variable %s.", CSV_COL(cols, 0 ));
    return -1;
  }

  PP_DATA_STRING *s = &item->data.data_string;
  if( 0 == strcmp("CONST", CSV_COL(cols, colModifier ))) {
    item->type |= TYPE_CONST;
  }

  char *str = CSV_COL(cols, colValue);
  strcpy( s->def_value, str );

#if 0
  if( (s->flags & TYPE_CONST) != 0) {
    string_add( str, 1 );
  }
#endif
  return 0;
}

static int parse_int( DATA_ITEM *item, size_t col_cnt, CSV_BUF *cols )
{
  enum {
    colDefault = 7,
    colMin = 8,
    colMax = 9
  };

  if( col_cnt < colDefault ) {
    log_printf( LogErr, "Not enough columns for variable %s.", CSV_COL(cols, 0 ));
    return -1;
  }

  PP_DATA_INT *d = &item->data.data_int;
  d->def_value = strtol( CSV_COL(cols, colDefault), 0, 0 );
  d->min = strtol( CSV_COL(cols, colMin), 0, 0 );
  d->max = strtol( CSV_COL(cols, colMax), 0, 0 );

  return 0;
}

static int parse_double( DATA_ITEM *item, size_t col_cnt, CSV_BUF *cols )
{
  enum {
    colDefault = 7,
    colMin,
    colMax,
    colPrec
  };

  if( col_cnt < colDefault ) {
    log_printf( LogErr, "Not enough columns for variable %s.", CSV_COL(cols, 0 ));
    return -1;
  }

  PP_DATA_DOUBLE *d = &item->data.data_double;
  d->def_value = strtod( CSV_COL(cols, colDefault), 0 );
  d->min = strtod( CSV_COL(cols, colMin), 0 );
  d->max = strtod( CSV_COL(cols, colMax), 0 );
  d->prec = strtod( CSV_COL(cols, colPrec), 0 );

  return 0;
}

/** parse_enumn *************************************************************/
/**
 *  @param item
 *  @param col_cnt
 *  @param cols 
 */
static int parse_enum( DATA_ITEM *item, size_t col_cnt, CSV_BUF *cols )
{
  UNUSED_PARAM( col_cnt );

  CSV_BUF Buf;

  enum {
    colFirstMbr = 7
  };

  if( col_cnt < colFirstMbr ) {
    log_printf( LogErr, "Not enough columns for variable %s.", CSV_COL(cols, 0 ));
    return -1;
  }

  PP_DATA_ENUM *d = &item->data.data_enum;
  d->def_mbr = 0;
  d->cnt = 0;
  d->items = NULL;

  int i = colFirstMbr;
  for( ; ; i++) {

    char *s = strtrim( CSV_COL( cols, i ), ' ');
    if( 0 == strlen( s )) {
      break;
    }

    ENUM_MBR_DESC *mbr = (ENUM_MBR_DESC *)calloc( sizeof(ENUM_MBR_DESC), 1 );
    LL_APPEND( d->items, mbr );

    memset( Buf, 0, sizeof(Buf));
    uint32_t col_cnt = MaxCsvColumns;
    split( s, '=', (char**)Buf, LineSize, &col_cnt );

    s = CSV_COL( &Buf, 0 );
    if( ':' == *s ) {
      d->def_mbr = i - colFirstMbr;
      s++;
    }
    strcpy( mbr->hnd, s );
    
    mbr->value = i - colFirstMbr;
    if( col_cnt > 1 ) {
      s = CSV_COL( &Buf, 1 );
      errno = 0;
      mbr->value = strtol( s, 0, 0 );
      if( errno != 0 ) {
        log_printf( LogErr, "Invalid number for enum member %s in variable %s", mbr->hnd, CSV_COL(cols, 0 ));
      }
    }

    if( col_cnt > 2 ) {
      s = CSV_COL( &Buf, 2 );
      s = skip_space( s );
      strcpy( mbr->string, s );

      string_add( s, 0 );
    }

    d->cnt++;
  }

  return 0;
}
