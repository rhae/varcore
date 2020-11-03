/*
 *  Copyright (c) 2020, Ruediger Haertel
 *  All rights reserved.
 *  
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *  
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *  
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  
 *  3. Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *  
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 *  TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file   varcore.c
 * \author rhae
 * 
 * The varcore has a set of variables that can be read or written.
 * As it is a global storage it makes it easy to access variables from 
 * all places of a programm. With its SCPI-Strings it is possible
 * to write a command line iinterface (CLI) to access the data from
 * outside.
 */

/* local header */
#include "varcore.h"

/* project headers */

/* header of common types */

/* shared common header */


/* header of standard C - libraries */
#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

/* constant definitions
----------------------------------------------------------------------------*/
#ifndef UNUSED_PARAM
# define UNUSED_PARAM(x) (void)(x)
#endif

#ifndef LOG_UNH_CASE
# define LOG_UNH_CASE(x)   printf( "[%s:%d] Unhandled case %d\n", __FILE__, __LINE__, x )
#endif

#ifndef LOG_NOT_IMPL
# define LOG_NOT_IMPL(x)   printf( "[%s:%d] %s not implemented\n", __FILE__, __LINE__, x )
#endif

#ifndef countof
# define countof(x) ( sizeof(x) / sizeof(x[0]) )
#endif

/* local defined data types
----------------------------------------------------------------------------*/

/* list of external used functions, if not in headers
----------------------------------------------------------------------------*/

/* list of global defined functions
----------------------------------------------------------------------------*/

/* list of local defined functions
----------------------------------------------------------------------------*/
static int  vc_chk_vector( VAR_DESC const *, int8_t );
static int  vc_init_s16( VAR_DESC const *);
static int  vc_init_s32( VAR_DESC const *);
static int  vc_init_f32( VAR_DESC const *);
static int  vc_init_f64( VAR_DESC const *);
static int  vc_init_enum( VAR_DESC const *);
static int  vc_init_string( VAR_DESC const *);

/* external variables
----------------------------------------------------------------------------*/

/* global variables
----------------------------------------------------------------------------*/

/* local defined variables
----------------------------------------------------------------------------*/
VC_DATA const *s_vc_data;

char const *s_type_str[] = {
	"TYPE_INT8",
	"TYPE_INT16",
	"TYPE_INT32",
	"TYPE_INT64",
	"TYPE_FLOAT",
	"TYPE_DOUBLE",
	"TYPE_ENUM",
	"TYPE_STRING",
	"TYPE_ACTION"
};


/*** is_vector **************************************************************/
/**
 *   Check if variable is a vector.
 */
static inline int is_vector( VAR_DESC const *var ) {
	int nRet;

	nRet = 0;
	if( var->type & TYPE_VECTOR ) {
		nRet = 1;
	}

	return nRet;
}

/*** acc_allowed *************************************************************/
/**
 *   Check if requested access is allowed.
 */
static inline ErrCode acc_allowed( VAR_DESC const *var, int rdwr, U16 req ) {

	int needs_admin = var->acc_rights & REQ_ADMIN;
	int has_admin = req & REQ_ADMIN;
	if( rdwr == VarWrite && needs_admin != has_admin) {
		return kErrAccessDenied;
	}

	U16 acc = var->acc_rights & MSK_ACC;
	U16 match = acc & req;
	if( match != (req & MSK_ACC)) {
		return kErrAccessDenied;
	}
	return kErrNone;
}

static inline char const* type2str( U16 n ) {
	if( n >= TYPE_LAST ) {
		return "UNKNOWN";
	}
	return s_type_str[n];
}

/*** vc_init ****************************************************************/
/**
 *
 *
 */
ErrCode vc_init( VC_DATA const *vc ) {

	s_vc_data = vc;
	vc_reset();
	
	return kErrNone;
}

ErrCode vc_reset() {

	assert( s_vc_data );
	HND     var_cnt = s_vc_data->var_cnt;
	ErrCode E       = kErrNone;

	for( HND hVar = 0; E == kErrNone && hVar < var_cnt; hVar++ ) {
		VAR_DESC const *var  = &s_vc_data->vars[hVar];
		U16             type = var->type & TYPE_MASK;
		
		switch( type ) {
			case TYPE_INT16:
				E = vc_init_s16( var );
				break;

			case TYPE_INT32:
				E = vc_init_s32( var );
				break;

			case TYPE_ENUM:
				E = vc_init_enum( var );
				break;

			case TYPE_FLOAT:
				E = vc_init_f32( var );
				break;

			case TYPE_DOUBLE:
				E = vc_init_f64( var );
				break;

			case TYPE_STRING:
				vc_init_string( var );
				break;
		}
	}

	return E;
}

/*** vc_get_access ***************************************************/
/**
 *   Return access rights from handle.
 * 
 *   @note:
 *   At the moment chan is not used.
 * 
 *   @param hnd     Variable-handle
 *   @param chan    Channel
 */
int vc_get_access( HND hnd, int chan ) {
	VAR_DESC const *var;
	UNUSED_PARAM( chan );

	assert( s_vc_data );
	assert( hnd < s_vc_data->var_cnt );

	var = &s_vc_data->vars[hnd];
	return var->acc_rights;
}

/*** vc_get_datatype *************************************************/
/**
 *   Return datatype from handle.
 * 
 *   @param hnd     Variable-handle
 */
int vc_get_datatype( HND hnd ) {
	VAR_DESC const *var;

	assert( s_vc_data );
	assert( hnd < s_vc_data->var_cnt );

	var = &s_vc_data->vars[hnd];
	return var->type;
}

/*** vc_as_int16 *****************************************************/
/**
 *   Read or write a variable of TYPE_INT16 and TYPE_ENUM
 *
 *   @param hnd    Variable handle
 *   @param rdwr   Read/Write access
 *   @param val    Pointer to value
 *   @param chan   Channel
 *   @param req    Request source
 */
ErrCode vc_as_int16( HND hnd, int rdwr, S16 *val, U16 chan, U16 req ) {
	ErrCode ret = kErrNone;
	VAR_DESC const *var;
	DATA_S16 *data_s16 = NULL;
	DATA_ENUM *data_enum = NULL;

	assert( s_vc_data );
	assert( hnd < s_vc_data->var_cnt );

	var = &s_vc_data->vars[hnd];

	switch( var->type & TYPE_MASK ) {
		case TYPE_INT16:
			data_s16 = &s_vc_data->data_s16[var->data_idx + chan];
			break;

		case TYPE_ENUM:
			data_enum = &s_vc_data->data_enum[var->data_idx + chan];
			break;

		default:
			return kErrInvalidType;
	}

	ret = acc_allowed( var, rdwr, req );
	if( ret != kErrNone ) {
		return ret;
	}

  	if( chan > 0 ) {
		ret = vc_chk_vector( var, chan );
		if( ret != kErrNone ) {
			return ret;
		}
	}

	if( rdwr == VarRead ) {
		*val = data_s16 ? data_s16->def_value : *data_enum;
	}
	else {
		S16 *def_value = data_s16 ? &data_s16->def_value : data_enum;
		*def_value = *val;
	}
	
	return ret;
}

/*** vc_as_int32 ************************************************************/
/**
 *   Read or write a variable of TYPE_INT32
 *
 *   @param hnd    Variable handle
 *   @param rdwr   Read/Write access
 *   @param val    Pointer to value
 *   @param chan   Channel
 *   @param req    Request source
 */
ErrCode vc_as_int32( HND hnd, int rdwr, S32 *val, U16 chan, U16 req ) {
	ErrCode ret = kErrNone;
	VAR_DESC const *var;
	DATA_S32 *data;

	assert( s_vc_data );
	assert( hnd < s_vc_data->var_cnt );

	var = &s_vc_data->vars[hnd];
	data = &s_vc_data->data_s32[var->data_idx + chan];

	if((var->type & TYPE_MASK) != TYPE_INT32 ) {
		return kErrInvalidType;
	}

	ret = acc_allowed( var, rdwr, req );
	if( ret != kErrNone ) {
		return ret;
	}

    if( chan > 0) {
		ret = vc_chk_vector( var, chan );
		if( ret != kErrNone ) {
			return ret;
		}
	}

	if( rdwr == VarRead ) {
		*val = data->def_value;
	}
	else {
		data->def_value = *val;
	}
	
	return ret;
}


/*** vc_as_string ***********************************************************/
/**
 *   Read or write a variable of any type.
 * 
 *   VarWrite:
 *   - The data is converted to the underlying data type with
 *     one of the functions strtol, strtof, strtod
 * 
 *   VarRead:
 *   - The data is converted to a string.
 *
 *   @param hnd    Variable handle
 *   @param rdwr   Read/Write access
 *   @param val    Pointer to value
 *   @param chan   Channel
 *   @param req    Request source
 */
ErrCode vc_as_string( HND hnd, int rdwr, char *val, U16 chan, U16 req ) {
	ErrCode ret = kErrNone;
	VAR_DESC const *var;
	U16 type;
	U16 flags;
	
	assert( s_vc_data );
	assert( hnd < s_vc_data->var_cnt );

	if( NULL == val ) {
		return kErrInvalidArg;
	}

	var = &s_vc_data->vars[hnd];
	type = var->type & TYPE_MASK;
	flags = var->type & TYPE_FLAG;
	switch( type ) {

		case TYPE_INT16:
			if( rdwr == VarWrite ) {
				S32 n;
				S16 n16;
				char *endp;

				errno = 0;
				n = strtol( (char*)val, &endp, 0 );
				if( errno != 0 || (char*)val == endp || *endp != '\0' || n > 32767 || n < -32768) {
					return kErrInvalidValue;
				}
				n16 = (S16) n;
				ret = vc_as_int16( hnd, rdwr, &n16, chan, req );
			}
			else {
				S16 n16 = 0;
				char *p = val;
				ret = vc_as_int16( hnd, rdwr, &n16, chan, req );
				if( ret == kErrNone ) {
					sprintf( p, "%hd", n16 );
				}
			}
			break;

		case TYPE_INT32:
			if( rdwr == VarWrite ) {
				S32 n;
				char *endp;

				errno = 0;
				n = strtol( (char*)val, &endp, 0 );
				if( errno != 0 || (char*)val == endp || *endp != '\0' ) {
					return kErrInvalidValue;
				}
				ret = vc_as_int32( hnd, rdwr, &n, chan, req );
			}
			else {
				S32 n = 0;
				char *p = val;
				ret = vc_as_int32( hnd, rdwr, &n, chan, req );
				if( ret == kErrNone ) {
					sprintf( p, "%d", n );
				}
			}
			break;

		case TYPE_FLOAT:
			if( rdwr == VarWrite ) {
				F32 f;
				char *endp;

				errno = 0;
				f = strtof( (char*)val, &endp );
				if( errno != 0 || (char*)val == endp || *endp != '\0' ) {
					return kErrInvalidValue;
				}
				ret = vc_as_float( hnd, rdwr, &f, chan, req );
			}
			else {
				F32 f;
				char *p = val;
				ret = vc_as_float( hnd, rdwr, &f, chan, req );
				if( ret == kErrNone ) {
					sprintf( p, "%f", f );
				}
			}
			break;

		case TYPE_STRING:
		{
			if( chan > 0) {
				ret = vc_chk_vector( var, chan );
				if( ret != kErrNone ) {
					return ret;
				}
			}

			if( flags & TYPE_CONST ) {

				if( rdwr == VarWrite ) {
					return kErrAccessDenied;
				}

				S32 idx = var->descr_idx;
				DATA_STRING const *data = &s_vc_data->data_const_str[idx];
				size_t len = strlen(data);
				len = (len >= sizeof(STRBUF)) ? sizeof(STRBUF)-1 : len;
				memcpy( val, data, len );
				val[len] = '\0';
			}
			else {
				S32 idx = var->data_idx + chan * sizeof(STRBUF);
				S32 max_idx = sizeof(STRBUF) * s_vc_data->data_str_cnt;
				if( idx > max_idx ) {
					return kErrInvalidChan;
				}
				DATA_STRING *data = &s_vc_data->data_str[idx];

				if( rdwr == VarWrite ) {
					size_t len = strlen( val );
					if( len > sizeof(STRBUF)) {
						return kErrSizeTooBig;
					}
					memcpy( data, val, sizeof(STRBUF));
				}
				else {
					memcpy( val, data, sizeof(STRBUF));
					val[sizeof(STRBUF)-1] = '\0';
				}
			}
		}
		break;

		case TYPE_ENUM:
			LOG_NOT_IMPL( "TYPE_ENUM" );
			break;

		default:
			LOG_UNH_CASE( type );
			break;
	}
	
	return ret;
}


/*** vc_as_float ************************************************************/
/**
 *   Read or write a variable of TYPE_FLOAT
 *
 *   @param hnd    Variable handle
 *   @param rdwr   Read/Write access
 *   @param val    Pointer to value
 *   @param chan   Channel
 *   @param req    Request source
 */
ErrCode vc_as_float( HND hnd, int rdwr, float *val, U16 chan, U16 req ) {
	ErrCode ret = kErrNone;
	VAR_DESC const *var;
	DATA_F32 *data;

	assert( s_vc_data );
	assert( hnd < s_vc_data->var_cnt );

	var = &s_vc_data->vars[hnd];
	data = &s_vc_data->data_f32[var->data_idx + chan];

	if((var->type & TYPE_MASK) != TYPE_FLOAT ) {
		return kErrInvalidType;
	}

	ret = acc_allowed( var, rdwr, req );
	if( ret != kErrNone ) {
		return ret;
	}

  	if( chan > 0 ) {
		ret = vc_chk_vector( var, chan );
		if( ret != kErrNone ) {
			return ret;
		}
	}

	if( rdwr == VarRead ) {
		*(F32 *)val = data->def_value;
	}
	else {
		data->def_value = *(F32*)val;
	}
	
	return ret;
}

/*** vc_dump_var *****************************************************/
/**
 *   Write the contents of the variable to a string buffer.
 * 
 *   This function is for introspection and debugging purposes.
 *
 *   @param buf    Pointer to atring buffer
 *   @param bufsz  Buffer size
 *   @param hnd    Variable handle
 *   @param chan   Channel
 */
int vc_dump_var( char *buf, U16 bufsz, HND hnd, U16 chan ) {
	#define CHECK_LEN( b, n, l, s ) do { \
     if( n < 0 || (l+n) > s ) {  \
			 b[l] = '\0'; \
			 return l; \
		 } \
	} while(0)


	VAR_DESC const *var;
	STRBUF spaces;
	U16 type;
	int n;
	int i;
	U16 len = 0;
	char const *scpi;

	assert( hnd < s_vc_data->var_cnt );

	memset( spaces, ' ', sizeof(STRBUF));

	var = &s_vc_data->vars[hnd];
	type = var->type & TYPE_MASK;
	scpi = (var->scpi_idx == HNON) ? "---" : &s_vc_data->data_const_str[var->scpi_idx];

	n = snprintf( &buf[len], bufsz - len,
	  "===========================================\n"
		"SCPI:               %s\n"
		"Hnd:                %04X\n"
		"Data type:          %s (%04X)\n"
		"Items:              %d\n"
		"Access:             %04X\n"
		"Descriptor Idx:     %d\n"
		"Data Idx:           %d\n"
		"Data\n"
		"------------------------------------------\n"
		,
		scpi,
		hnd,
		type2str(type), var->type,
		var->vec_items,
		var->acc_rights,
		var->descr_idx,
		var->data_idx
	);

	CHECK_LEN( buf, n, len, bufsz );
	len += n;

	switch( type ) {
		case TYPE_INT16:

		  n = snprintf( &buf[len], bufsz - len, "          val    min    max\n");
			CHECK_LEN( buf, n, len, bufsz );
			len += n;

			if( var->vec_items == 1 ) {
				n = snprintf( &buf[len], bufsz - len, "     ");
				CHECK_LEN( buf, n, len, bufsz );
				len += n;
			}

			for( i = 0; i < var->vec_items; i++ ) {
				DATA_S16 *d = &s_vc_data->data_s16[var->data_idx + i];

				if( var->vec_items > 1 ) {
					n = snprintf( &buf[len], bufsz - len, " %3d:", i );
					CHECK_LEN( buf, n, len, bufsz );
					len += n;	
				}
				n = snprintf( &buf[len], bufsz - len, "  %6hd %6hd %6hd\n", d->def_value, d->min, d->max );
				CHECK_LEN( buf, n, len, bufsz );
				len += n;
			}
			break;

		case TYPE_INT32:

		  n = snprintf( &buf[len], bufsz - len, "          val    min    max\n");
			CHECK_LEN( buf, n, len, bufsz );
			len += n;

			if( var->vec_items == 1 ) {
				n = snprintf( &buf[len], bufsz - len, "     ");
				CHECK_LEN( buf, n, len, bufsz );
				len += n;
			}

			for( i = 0; i < var->vec_items; i++ ) {
				DATA_S32 *d = &s_vc_data->data_s32[var->data_idx + i];

				if( var->vec_items > 1 ) {
					n = snprintf( &buf[len], bufsz - len, " %3d:", i );
					CHECK_LEN( buf, n, len, bufsz );
					len += n;	
				}
				n = snprintf( &buf[len], bufsz - len, " %6d %6d %6d\n", d->def_value, d->min, d->max );
				CHECK_LEN( buf, n, len, bufsz );
				len += n;
			}
			break;

		case TYPE_FLOAT:
#if 0
			for( i = 0; i < var->vec_items; i++ ) {
				DATA_S32 *d = &s_vc_data->data_s32[var->data_idx];
				n = snprintf( &buf[len], bufsz - len, "  %5d %5d %5d", d->def_value, d->min, d->max );
				if( n < 0 ) {
					return len;
				}
				len += n;
			}
#else
			n = snprintf( &buf[len], bufsz - len, "NOT IMPLEMENTED.\n" );
			CHECK_LEN( buf, n, len, bufsz );
			len += n;
#endif
			break;

		case TYPE_ENUM:
			{
				DESCR_ENUM const *dscr = (DESCR_ENUM const *)&s_vc_data->data_mbr[var->descr_idx];
				for( U16 i = 0; i < dscr->cnt; i++ ) {
					ENUM_MBR const *mbr = (ENUM_MBR const *)&dscr->mbr[i];
					STRBUF S;
					int flag = 0;
					int x;

					vc_as_string( mbr->hnd, VarRead, S, chan, REQ_PRG );
					x = strlen( S );
					spaces[11-x] = '\0';
					n = snprintf( &buf[len], bufsz - len, "%2d:  %s%s %4d", i, S, spaces, mbr->value );
					spaces[11-x] = ' ';
					CHECK_LEN( buf, n, len, bufsz );
					len += n;
						
					for( U16 c = 0; c < var->vec_items; c++ ) {
						U16 idx = var->data_idx + c;
						DATA_ENUM *d = (DATA_ENUM *)&s_vc_data->data_enum[idx];

						if( *d == mbr->value ) {
							if( !flag ) {
								n = snprintf( &buf[len], bufsz - len, " <= [" );
								flag = 1;
							}
							else {
								n = snprintf( &buf[len], bufsz - len, ", " );
							}
							CHECK_LEN( buf, n, len, bufsz );
							len += n;

							n = snprintf( &buf[len], bufsz - len, "%d", c+1 );
							CHECK_LEN( buf, n, len, bufsz );
							len += n;
						}
					}

					if( flag ) {
						n = snprintf( &buf[len], bufsz - len, "]\n" );
					}
					else {
						n = snprintf( &buf[len], bufsz - len, "\n" );
					}
					CHECK_LEN( buf, n, len, bufsz );
					len += n;
				}
			}
			break;
	}

	n = snprintf( &buf[len], bufsz - len,"===========================================\n" );
	CHECK_LEN( buf, n, len, bufsz );
	len += n;

	return len;
}

/*** vc_chk_vector **********************************************************/
/**
 *
 *
 */
static ErrCode vc_chk_vector( VAR_DESC const *var, int8_t chan )
{
	int ret;

	ret = kErrNoVector;
	if ( is_vector( var )) {
		ret = kErrNone;
		if(chan >= var->vec_items) {
			ret = kErrInvalidChan;
		}
	}
	return ret;
}

/*** vc_init_s16 **********************************************************/
/**
 *	 Copy data from the descriptor into the data location of \b var
 *
 *   @param var   Variable handle
 *
 *   @return kErrNone, when done.
 */
static ErrCode vc_init_s16( VAR_DESC const *var ) {
	DATA_S16 const *descr = &s_vc_data->descr_s16[var->descr_idx];
	DATA_S16       *data  = &s_vc_data->data_s16[var->data_idx];

	memcpy( data, descr, sizeof(DATA_S16) * var->vec_items );
	return kErrNone;
}

/*** vc_init_s32 **********************************************************/
/**
 *	 Copy data from the descriptor into the data location of \b var
 *
 *   @param var   Variable handle
 *
 *   @return kErrNone, when done.
 */
static ErrCode vc_init_s32( VAR_DESC const *var ) {
	DATA_S32 const *descr = &s_vc_data->descr_s32[var->descr_idx];
	DATA_S32       *data  = &s_vc_data->data_s32[var->data_idx];

	memcpy( data, descr, sizeof(DATA_S32) * var->vec_items );
	return kErrNone;
}

/*** vc_init_f32 **********************************************************/
/**
 *	 Copy data from the descriptor into the data location of \b var
 *
 *   @param var   Variable handle
 *
 *   @return kErrNone, when done.
 */
static ErrCode vc_init_f32( VAR_DESC const *var ) {
	DATA_F32 const *descr = &s_vc_data->descr_f32[var->descr_idx];
	DATA_F32       *data  = &s_vc_data->data_f32[var->data_idx];

	memcpy( data, descr, sizeof(DATA_F32) * var->vec_items );
	return kErrNone;
}

/*** vc_init_f64 **********************************************************/
/**
 *	 Copy data from the descriptor into the data location of \b var
 *
 *   @param var   Variable handle
 *
 *   @return kErrNone, when done.
 */
static ErrCode vc_init_f64( VAR_DESC const *var ) {
	DATA_F64 const *descr = &s_vc_data->descr_f64[var->descr_idx];
	DATA_F64       *data  = &s_vc_data->data_f64[var->data_idx];

	memcpy( data, descr, sizeof(DATA_F64) * var->vec_items );
	return kErrNone;
}

/*** vc_init_enum **********************************************************/
/**
 *	 Copy data from the descriptor into the data location of \b var
 *
 *   @param var   Variable handle
 *
 *   @return kErrNone, when done.
 */
static ErrCode vc_init_enum( VAR_DESC const *var ) {
	DESCR_ENUM const *descr = (DESCR_ENUM*)&s_vc_data->data_mbr[var->descr_idx];
	S16              *data  = &s_vc_data->data_enum[var->data_idx];
    
	for( int i = 0; i < var->vec_items; i++ ) {
		*data = descr->def_value;
		data++;
	}
	return kErrNone;
}

/*** vc_init_string **********************************************************/
/**
 *	 Copy data from the descriptor into the data location of \b var
 *
 *   @param var   Variable handle
 *
 *   @return kErrNone, when done.
 */
static int  vc_init_string( VAR_DESC const *var ) {
	DATA_STRING const *descr = &s_vc_data->data_const_str[var->descr_idx];
	DATA_STRING       *data  = &s_vc_data->data_str[var->data_idx];
	U16 flags = var->type & TYPE_FLAG;

	if( flags & TYPE_CONST ) {
		return kErrNone;
	}
    
	size_t len = strlen( descr );
	for( U16 i = 0; i < var->vec_items; i++ ) {
		memcpy( data, descr, len );
		data += sizeof(STRBUF);
	}
	return kErrNone;
}
/*______________________________________________________________________EOF_*/
