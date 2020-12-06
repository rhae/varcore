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
static int     vc_chk_vector( VAR_DESC const *, int8_t );
static int     init_s16( VAR_DESC const *);
static int     init_s32( VAR_DESC const *);
static int     init_f32( VAR_DESC const *);
static int     init_f64( VAR_DESC const *);
static int     init_enum( VAR_DESC const *);
static int     init_string( VAR_DESC const *);

static ErrCode valid_enum( DESCR_ENUM const *, S16 );
static ErrCode rw_min_max( HND hnd, U8* val, U16 chan, U16 flag );

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

char const *s_format_str[] = {
	"FMT_DEFAULT",
	"FMT_PREC_1",
	"FMT_PREC_2",
	"FMT_PREC_3",
	"FMT_PREC_4",
	"",
	"",
	"",
	"",
	"",
	"FMT_SCI",
  "FMT_DATE",
	"FMT_HEX2",
	"FMT_HEX4",
	"FMT_HEX8"
};

char const *s_storage_str[] = {
	"RAM_VLOATILE",
	"EEPROM",
	"FLASH"
};

static VAR_DESC const* get_var( HND hnd ) {
	return &s_vc_data->vars[hnd];
}


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

static inline char const* format2str( U16 n ) {
	if( n >= FMT_LAST ) {
		return "UNKNOWN";
	}
	
	return s_format_str[n];
}

static inline char const* storage2str( U16 n ) {
	n &= MSK_STORAGE;
	if( n >= FLASH ) {
		return "UNKNOWN";
	}
	n >>= 8;
	return s_storage_str[n];
}

static inline char const *get_scpi( HND hnd ) {
	VAR_DESC const *var;

	var = get_var( hnd );
	if( !var ) {
		return 0;
	}
	return (var->scpi_idx == HNON) ? "---" : &s_vc_data->data_const_str[var->scpi_idx];
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
		VAR_DESC const *var  = get_var(hVar);
		U16             type = var->type & TYPE_MASK;
		
		switch( type ) {
			case TYPE_INT16:
				E = init_s16( var );
				break;

			case TYPE_INT32:
				E = init_s32( var );
				break;

			case TYPE_ENUM:
				E = init_enum( var );
				break;

			case TYPE_FLOAT:
				E = init_f32( var );
				break;

			case TYPE_DOUBLE:
				E = init_f64( var );
				break;

			case TYPE_STRING:
				init_string( var );
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

	var = get_var( hnd );
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

	var = get_var( hnd );
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
	DATA_S16 *data;
	DATA_ENUM *data_enum;
	U16 type;

	assert( s_vc_data );
	
	if( hnd >= s_vc_data->var_cnt ) {
		return kErrUnknownCmd;
	}

	if( NULL == val ) {
		return kErrInvalidArg;
	}

	var = get_var( hnd );
	type = var->type & TYPE_MASK;

	switch( type ) {
		case TYPE_INT16:
			data = &s_vc_data->data_s16[var->data_idx + chan];
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
		*val = (TYPE_INT16 == type) ? data->def_value : *data_enum;
	}
	else {
		if( TYPE_INT16 == type ) {
			U16 flags = var->acc_rights & REQ_FLAG;
			if( flags ) {
				if( flags & FLAG_LIMIT ) {
					if( *val > data->max ) {
						return kErrUpperLimit;
					}
					else if ( *val < data->min ) {
						return kErrLowerLimit;
					}
				}
				else if ( flags & FLAG_CLIP ) {
					if( *val > data->max ) {
						*val = data->max;
					}
					else if ( *val < data->min ) {
						*val = data->min;
					}
				}
			}

			data->def_value = *val;
		}
		else {
			DESCR_ENUM const *dscr = (DESCR_ENUM const *)&s_vc_data->data_mbr[var->descr_idx];
			ret = valid_enum( dscr, *val );
			if( ret == kErrNone ) {
				*data_enum = *val;
			}
		}
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

	if( hnd >= s_vc_data->var_cnt ) {
		return kErrUnknownCmd;
	}

	if( NULL == val ) {
		return kErrInvalidArg;
	}

	var = get_var( hnd );
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
		U16 flags = var->acc_rights & REQ_FLAG;
		if( flags ) {
			if( flags & FLAG_LIMIT ) {
				if( *val > data->max ) {
					return kErrUpperLimit;
				}
				else if ( *val < data->min ) {
					return kErrLowerLimit;
				}
			}
			else if ( flags & FLAG_CLIP ) {
				if( *val > data->max ) {
					*val = data->max;
				}
				else if ( *val < data->min ) {
					*val = data->min;
				}
			}
		}
		data->def_value = *val;
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
ErrCode vc_as_float( HND hnd, int rdwr, F32 *val, U16 chan, U16 req ) {
	ErrCode ret = kErrNone;
	VAR_DESC const *var;
	DATA_F32 *data;

	assert( s_vc_data );

	if( hnd >= s_vc_data->var_cnt ) {
		return kErrUnknownCmd;
	}

	if( NULL == val ) {
		return kErrInvalidArg;
	}

	var = get_var( hnd );
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
		U16 flags = var->acc_rights & REQ_FLAG;
		if( flags ) {
			if( flags & FLAG_LIMIT ) {
				if( *val > data->max ) {
					return kErrUpperLimit;
				}
				else if ( *val < data->min ) {
					return kErrLowerLimit;
				}
			}
			else if ( flags & FLAG_CLIP ) {
				if( *val > data->max ) {
					*val = data->max;
				}
				else if ( *val < data->min ) {
					*val = data->min;
				}
			}
		}
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

	if( hnd >= s_vc_data->var_cnt ) {
		return kErrUnknownCmd;
	}
	
	if( NULL == val ) {
		return kErrInvalidArg;
	}

	var = get_var( hnd );
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
				U16 n16 = 0;
				char *p = val;
				ret = vc_as_int16( hnd, rdwr, (S16*)&n16, chan, req );
				if( ret == kErrNone ) {
					switch( var->fmt ) {

						case FMT_HEX2:
							sprintf( p, "%#hx", n16 );
							break;

						case FMT_HEX4:
						  sprintf( p, "%#hx", n16 );
							break;

						default:
							sprintf( p, "%d", n16 );
					}
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
					U16 n16 = 0;
					U16 fmt = var->fmt;
					if( n > 0xffff ) {
						fmt = FMT_HEX8;
					}
					else {
						n16 = (U16) n;
					}

					switch( fmt ) {

						case FMT_HEX2:
							sprintf( p, "%#hx", n16 );
							break;

						case FMT_HEX4:
							sprintf( p, "%#hx", n16 );
							break;

						case FMT_HEX8:
						  sprintf( p, "%#x", n );
							break;

						default:
							sprintf( p, "%d", n );
					}
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

					switch( var->fmt ) {

						case FMT_PREC_1:
						case FMT_PREC_2:
						case FMT_PREC_3:
						case FMT_PREC_4:
						  sprintf( p, "%.*f", var->fmt, f );
							break;

						default:
							sprintf( p, "%f", f );
					}
					
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

/*** vc_get_min ***********************************************************/
/**
 *   Read minimum value of a variable of types:
 *      TYPE_INT16, TYPE_INT32, TYPE_F32.
 *
 *   @param hnd    Variable handle
 *   @param val    Pointer to value
 *   @param chan   Channel
 */
ErrCode vc_get_min( HND hnd, U8* val, U16 chan ) {
	return rw_min_max( hnd, val, chan, 0 );
}

/*** vc_get_max ***********************************************************/
/**
 *   Read maximum value of a variable of types:
 *      TYPE_INT16, TYPE_INT32, TYPE_F32.
 *
 *   @param hnd    Variable handle
 *   @param val    Pointer to value
 *   @param chan   Channel
 */
ErrCode vc_get_max( HND hnd, U8* val, U16 chan ) {
	return rw_min_max( hnd, val, chan, 1 );
}

/*** vc_get_min ***********************************************************/
/**
 *   Read minimum value of a variable of types:
 *      TYPE_INT16, TYPE_INT32, TYPE_F32.
 *
 *   @param hnd    Variable handle
 *   @param val    Pointer to value
 *   @param chan   Channel
 */
ErrCode vc_set_min( HND hnd, U8* val, U16 chan ) {
	return rw_min_max( hnd, val, chan, 2 );
}

/*** vc_set_max ***********************************************************/
/**
 *   Write maximum value of a variable of types:
 *      TYPE_INT16, TYPE_INT32, TYPE_F32.
 *
 *   @param hnd    Variable handle
 *   @param val    Pointer to value
 *   @param chan   Channel
 */
ErrCode vc_set_max( HND hnd, U8* val, U16 chan ) {
	return rw_min_max( hnd, val, chan, 3 );
}

/*** vc_get_format ***************************************************/
/**
 *   Get format of the variable
 *
 *   @param hnd    Variable handle
 *   @param fmt    Pointer to format
 */
ErrCode vc_get_format( HND hnd, U8 *fmt ) {
	VAR_DESC const *var;

	assert( hnd < s_vc_data->var_cnt );

	var = get_var( hnd );
	*(U16*)fmt = var->fmt;

	return kErrNone;
}

/*** vc_get_storage *********************************************************/
/**
 *   Get the storage modifier of the variable.
 *
 *   @param hnd    Variable handle
 *   @param store  Storage
 */
ErrCode vc_get_storage( HND hnd, U16 *store ) {
	VAR_DESC const *var;

	assert( s_vc_data );
	
	if( hnd >= s_vc_data->var_cnt ) {
		return kErrUnknownCmd;
	}

	if( NULL == store ) {
		return kErrInvalidArg;
	}

	var = get_var( hnd );
	*store = var->type & MSK_STORAGE;

	return kErrNone;
}

/*** vc_get_hnd *************************************************************/
/**
 *   Get the storage modifier of the variable.
 *
 *   @param scpi   SCPI string
 *
 *   @return HNON, when scpi was not found.
 */
HND vc_get_hnd( char const *scpi ) {

	for( HND i = 0; i < s_vc_data->var_cnt; i++ ) {

		char const *s = get_scpi( i );

		if( s ) {
			int res = strcmp( scpi, s );
			if( 0 == res ) {
				return i;
			}
		}
	}

	return HNON;
}

static int add_sep( char *buf, int bufsz, char c, int len ) {
	int n;

	assert( buf );
	assert( len > 0 );

	n = len > bufsz ? bufsz : len;
	memset( buf, c, n );

	if(( n+2 ) < bufsz ) {
		buf[n] = '\n'; n++;
		buf[n] = '\0';
	}
	return n;

}


/*** vc_dump_var ************************************************************/
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
     if( n < 0 || (l+n) >= s ) {  \
			 b[l] = '\0'; \
			 return l; \
		 } \
	} while(0)


	VAR_DESC const *var;
	STRBUF spaces;
	U16 type;
	U16 storage;
	int n;
	int i;
	U16 len = 0;
	char const *scpi;

	if( hnd >= s_vc_data->var_cnt ) {
		return kErrUnknownCmd;
	}

	memset( spaces, ' ', sizeof(STRBUF));

	var     = get_var( hnd );
	type    = var->type & TYPE_MASK;
	storage = (var->type & MSK_STORAGE) >> 8;
	scpi    = get_scpi( hnd );

	n = add_sep( &buf[len], bufsz - len, '=', 50 );
	CHECK_LEN( buf, n, len, bufsz );
	len += n;

	n = snprintf( &buf[len], bufsz - len,
		"SCPI:               %s\n"
		"Hnd:                %#04X\n"
		"Data type:          %s (%#04hX)\n"
		"Items:              %d\n"
		"Access:             %#04hX\n"
		"Storage:            %s (%#04hX)\n"
		"Format:             %s (%#04hX)\n"
		"Descriptor Idx:     %d\n"
		"Data Idx:           %d\n"
		"Data\n"
		,
		scpi,
		hnd,
		type2str(type), var->type,
		var->vec_items,
		var->acc_rights,
		storage2str(storage), storage,
		format2str(var->fmt), var->fmt,
		var->descr_idx,
		var->data_idx
	);

	CHECK_LEN( buf, n, len, bufsz );
	len += n;

	n = add_sep( &buf[len], bufsz - len, '-', 50 );
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
			n = snprintf( &buf[len], bufsz - len, "                val           min           max\n");
			CHECK_LEN( buf, n, len, bufsz );
			len += n;

			if( var->vec_items == 1 ) {
				n = snprintf( &buf[len], bufsz - len, "     ");
				CHECK_LEN( buf, n, len, bufsz );
				len += n;
			}

			for( i = 0; i < var->vec_items; i++ ) {
				DATA_F32 *d = &s_vc_data->data_f32[var->data_idx + i];

				if( var->vec_items > 1 ) {
					n = snprintf( &buf[len], bufsz - len, " %3d:", i );
					CHECK_LEN( buf, n, len, bufsz );
					len += n;	
				}

				n = snprintf( &buf[len], bufsz - len, " %13.*f %13.*f %13.*f\n", 
											var->fmt, d->def_value, var->fmt, d->min, var->fmt, d->max );
				CHECK_LEN( buf, n, len, bufsz );
				len += n;
			}
			break;

		case TYPE_ENUM:
			{
				DESCR_ENUM const *dscr = (DESCR_ENUM const *)&s_vc_data->data_mbr[var->descr_idx];
				for( i = 0; i < dscr->cnt; i++ ) {
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

	n = add_sep( &buf[len], bufsz - len, '=', 50 );
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

/*** vc_init_s16 ************************************************************/
/**
 *	 Copy data from the descriptor into the data location of \b var
 *
 *   @param var   Variable handle
 *
 *   @return kErrNone, when done.
 */
static ErrCode init_s16( VAR_DESC const *var ) {
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
static ErrCode init_s32( VAR_DESC const *var ) {
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
static ErrCode init_f32( VAR_DESC const *var ) {
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
static ErrCode init_f64( VAR_DESC const *var ) {
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
static ErrCode init_enum( VAR_DESC const *var ) {
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
static int  init_string( VAR_DESC const *var ) {
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

/*** vc_valid_enum ***************************************************/
/**
 *	 Check if \b val is a valid enum member.
 *
 *   @param dscr   pointer to ENUM descriptor
 *   @param val    value
 *
 *   @return kErrNone when val is a member of the enum.
 */
static ErrCode valid_enum( DESCR_ENUM const *dscr, S16 val ) {
	ErrCode E = kErrInvalidEnum;

	for( U16 i = 0; i < dscr->cnt; i++ ) {
		ENUM_MBR const *mbr = (ENUM_MBR const *)&dscr->mbr[i];
		if( val == mbr->value ) {
			return kErrNone;
		}
	}

	return E;
}

/*** vc_get_min_max **************************************************/
/**
 *   Read minimum or maximum value of a variable of types:
 *      TYPE_INT16, TYPE_INT32, TYPE_F32.
 *
 *   @param hnd    Variable handle
 *   @param val    Pointer to value
 *   @param chan   Channel
 *   @param flag   Flag, {Bit 0: 0 -> minimum, 1 -> maximum,
 *                        Bit 1: 0 -> read, 1 -> write }
 */
static ErrCode rw_min_max( HND hnd, U8* val, U16 chan, U16 flag ) {
	U16 type;

	union {
		/*U8   bytes[4];*/
		S32  val_s32;
    F32  val_f32;
	} conv;

	DATA_S16 *data_s16;
	DATA_S32 *data_s32;
	DATA_F32 *data_f32;

	VAR_DESC const *var;

	int minmax = flag & 1;
	int wr     = (flag & 2) >> 1;
	
	assert( s_vc_data );
	
	if( hnd >= s_vc_data->var_cnt ) {
		return kErrUnknownCmd;
	}

	if( NULL == val ) {
		return kErrInvalidArg;
	}

	var = get_var( hnd );
	type = var->type & TYPE_MASK;

	if( chan > 0 ) {
		ErrCode ret = vc_chk_vector( var, chan );
		if( ret != kErrNone ) {
			return ret;
		}
	}
	
	switch( type ) {
		case TYPE_INT16:
			data_s16 = &s_vc_data->data_s16[ var->data_idx + chan ];
			if( wr ) {
				S16 *p = (0 == minmax) ? &data_s16->min : &data_s16->max;
				*p = *(S16*)val;
			}
			else {
				*(S16*) val = (0 == minmax) ? data_s16->min : data_s16->max;
			}
			break;

		case TYPE_INT32:
			data_s32 = &s_vc_data->data_s32[ var->data_idx + chan ];
			if( wr ) {
				S32 *p = (0 == minmax) ? &data_s32->min : &data_s32->max;
				*p = *(S32*)val;
			}
			else {
				*(S32*) val = (0 == minmax) ? data_s32->min : data_s32->max;
			}
			break;

		case TYPE_FLOAT:
			data_f32 = &s_vc_data->data_f32[ var->data_idx + chan ];
			if( wr ) {
				F32 *p = (0 == minmax) ? &data_f32->min : &data_f32->max;
				conv.val_s32 = *(S32 *)val;
				*p = conv.val_f32;
			}
			else {
				conv.val_f32 = (0 == minmax) ? data_f32->min : data_f32->max;
				*(S32*) val = conv.val_s32;
			}
			break;

		default:
			return kErrInvalidType;
	}

	return kErrNone;
}

/*______________________________________________________________________EOF_*/
