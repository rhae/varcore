

#pragma once

typedef struct _LOC {
  char file[512];
  int line_nr;

  struct _LOC *next;
} LOC;

void loc_init( void );

LOC* loc_cur();
void loc_push( const char*, int );
void loc_pop();
void loc_set( int );