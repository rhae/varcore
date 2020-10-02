
#include <stdint.h>

#ifndef __UTILS_H_
#define __UTILS_H_

void   split( char* s, char sep, char** svec, uint32_t len, uint32_t* n );
char*  srepeat( char c, uint16_t len );
char*  strtrim( char* s, char c );
char*  skip_space( char * );

#endif

