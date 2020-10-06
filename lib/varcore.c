/**
 * \file varcore.c
 * \author: hae
 */

#include "varcore.h"

#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#ifndef UNUSED_PARAM
#  define UNUSED_PARAM(x) (void)(x)
#endif

#ifndef LOG_UNH_CASE
#  define LOG_UNH_CASE(x)   printf( "[%s:%d] Unhandled case %d", __FILE__, __LINE__, x )
#endif

#ifndef countof
# define countof(x) ( sizeof(x) / sizeof(x[0]) )
#endif

VC_DATA const *s_vc_data;

static int  vc_chk_vector( VAR_DESC const *, int8_t );

static inline int is_vector( VAR_DESC const *var )
{
	int nRet;

	nRet = 0;
	if( var->type & TYPE_VECTOR ) {
		nRet = 1;
	}

	return nRet;
}

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

/************************************************************************************/
/**
 *
 *
 */
int vc_init( VC_DATA const *vc )
{
	s_vc_data = vc;
	
	return 0;
}

int vc_get_access( HND hnd, int chan ) {
	VAR_DESC const *var;
	UNUSED_PARAM( chan );

	assert( hnd < s_vc_data->var_cnt );

	var = &s_vc_data->vars[hnd];
	return var->acc_rights;
}

/************************************************************************************/
/**
 *
 *
 */
ErrCode vc_as_int16( HND hnd, int rdwr, S16 *val, U16 chan, U16 req ) {
	ErrCode ret = kErrNone;
	VAR_DESC const *var;
	DATA_S16 *data;

	assert( hnd < s_vc_data->var_cnt );

	var = &s_vc_data->vars[hnd];
	data = &s_vc_data->data_s16[var->data_idx + chan];

	if((var->type & MSK_TYPE) != TYPE_INT16 ) {
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

/************************************************************************************/
/**
 *
 *
 */
ErrCode vc_as_int32( HND hnd, int rdwr, S32 *val, U16 chan, U16 req ) {
	ErrCode ret = kErrNone;
	VAR_DESC const *var;
	DATA_S32 *data;

	assert( hnd < s_vc_data->var_cnt );

	var = &s_vc_data->vars[hnd];
	data = &s_vc_data->data_s32[var->data_idx + chan];

	if((var->type & MSK_TYPE) != TYPE_INT32 ) {
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


/************************************************************************************/
/**
 *
 *
 */
ErrCode vc_as_string( HND hnd, int rdwr, char *val, U16 chan, U16 req ) {
	ErrCode ret = kErrNone;
	VAR_DESC const *var;
	U16 type;
	U16 flags;
	
	assert( hnd < s_vc_data->var_cnt );

	var = &s_vc_data->vars[hnd];
	type = var->type & MSK_TYPE;
	flags = var->type & kTypeFlag;
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

			if( flags & kTypeConst ) {

				if( rdwr == VarWrite ) {
					return kErrAccessDenied;
				}

				S32 idx = var->data_idx;
				DATA_STRING const *data = &s_vc_data->data_const_str[idx];
				size_t len = strlen(data);
				len = len >= sizeof(STRBUF) ? sizeof(STRBUF)-1 : len;
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
					memcpy( data, val, sizeof(STRBUF));
				}
				else {
					memcpy( val, data, sizeof(STRBUF));
					val[sizeof(STRBUF)-1] = '\0';
				}
			}
		}
		break;

		default:
			LOG_UNH_CASE( type );
			break;
	}
	
	return ret;
}


/************************************************************************************/
/**
 *
 *
 */
ErrCode vc_as_float( HND hnd, int rdwr, float *val, U16 chan, U16 req ) {
	ErrCode ret = kErrNone;
	VAR_DESC const *var;
	DATA_S32 *data;

	assert( hnd < s_vc_data->var_cnt );

	var = &s_vc_data->vars[hnd];
	data = &s_vc_data->data_s32[var->data_idx + chan];

	if((var->type & MSK_TYPE) != TYPE_INT32 ) {
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
		*(S32 *)val = data->def_value;
	}
	else {
		data->def_value = *(S32*)val;
	}
	
	return ret;
}

/************************************************************************************/
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


