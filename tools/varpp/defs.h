
#pragma once

#include "loc.h"
#include "uthash.h"

#include <stdbool.h>

typedef struct _DEF {
  char name[64];
  char value[64];

  LOC loc;
  UT_hash_handle hh;         /* makes this structure hashable */
} DEF;

typedef struct _DEF_ITER {
  DEF *cur;
} DEF_ITER;

void defs_init();

void defs_add( char const*, LOC const* );
DEF* defs_get( char const* );

void defs_iter( DEF_ITER * );
int  defs_next( DEF *, DEF_ITER * );
