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

static inline ERR_CODE acc_allowed( VAR_DESC const *var, int rdwr, U16 req ) {

	int needs_admin = var->acc_rights & REQ_ADMIN;
	int has_admin = req & REQ_ADMIN;
	if( rdwr == VarWrite && needs_admin != has_admin) {
		return ERR_ACCESS_DENIED;
	}

	U16 acc = var->acc_rights & MSK_ACC;
	U16 match = acc & req;
	if( match != (req & MSK_ACC)) {
		return ERR_ACCESS_DENIED;
	}
	return ERR_NONE;
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
ERR_CODE vc_as_int16( HND hnd, int rdwr, S16 *val, U16 chan, U16 req ) {
	ERR_CODE ret = ERR_NONE;
	VAR_DESC const *var;
	DATA_S16 *data;

	assert( hnd < s_vc_data->var_cnt );

	var = &s_vc_data->vars[hnd];
	data = &s_vc_data->data_s16[var->data_idx + chan];

	if((var->type & MSK_TYPE) != TYPE_INT16 ) {
		return ERR_INVALID_TYPE;
	}

	ret = acc_allowed( var, rdwr, req );
	if( ret != ERR_NONE ) {
		return ret;
	}

  if( chan > 0) {
		ret = vc_chk_vector( var, chan );
		if( ret != ERR_NONE ) {
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
ERR_CODE vc_as_int32( HND hnd, int rdwr, S32 *val, U16 chan, U16 req ) {
	ERR_CODE ret = ERR_NONE;
	VAR_DESC const *var;
	DATA_S32 *data;

	assert( hnd < s_vc_data->var_cnt );

	var = &s_vc_data->vars[hnd];
	data = &s_vc_data->data_s32[var->data_idx + chan];

	if((var->type & MSK_TYPE) != TYPE_INT32 ) {
		return ERR_INVALID_TYPE;
	}

	ret = acc_allowed( var, rdwr, req );
	if( ret != ERR_NONE ) {
		return ret;
	}

  if( chan > 0) {
		ret = vc_chk_vector( var, chan );
		if( ret != ERR_NONE ) {
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
ERR_CODE vc_as_string( HND hnd, int rdwr, char *val, U16 chan, U16 req ) {
	ERR_CODE ret = ERR_NONE;
	VAR_DESC const *var;
	U16 type;
	
	assert( hnd < s_vc_data->var_cnt );

	var = &s_vc_data->vars[hnd];
	type = var->type & MSK_TYPE;
	switch( type ) {

		case TYPE_INT16:
			if( rdwr == VarWrite ) {
				S32 n;
				S16 n16;
				char *endp;

				errno = 0;
				n = strtol( (char*)val, &endp, 0 );
				if( errno != 0 || (char*)val == endp || *endp != '\0' || n > 32767 || n < -32768) {
					return ERR_INVALID_VALUE;
				}
				n16 = (S16) n;
				ret = vc_as_int16( hnd, rdwr, &n16, chan, req );
			}
			else {
				S16 n16;
				char *p = val;
				ret = vc_as_int16( hnd, rdwr, &n16, chan, req );
				if( ret == ERR_NONE ) {
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
					return ERR_INVALID_VALUE;
				}
				ret = vc_as_int32( hnd, rdwr, &n, chan, req );
			}
			else {
				S32 n;
				char *p = val;
				ret = vc_as_int32( hnd, rdwr, &n, chan, req );
				if( ret == ERR_NONE ) {
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
					return ERR_INVALID_VALUE;
				}
				ret = vc_as_float( hnd, rdwr, &f, chan, req );
			}
			else {
				F32 f;
				char *p = val;
				ret = vc_as_float( hnd, rdwr, &f, chan, req );
				if( ret == ERR_NONE ) {
					sprintf( p, "%f", f );
				}
			}
			break;

		case TYPE_STRING:
		{
			S32 idx = var->data_idx + chan * sizeof(STRBUF);
			S32 max_idx = sizeof(STRBUF) * s_vc_data->data_str_cnt;
			if( idx > max_idx ) {
				return ERR_INVALID_CHAN;
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
ERR_CODE vc_as_float( HND hnd, int rdwr, float *val, U16 chan, U16 req ) {
	ERR_CODE ret = ERR_NONE;
	VAR_DESC const *var;
	DATA_S32 *data;

	assert( hnd < s_vc_data->var_cnt );

	var = &s_vc_data->vars[hnd];
	data = &s_vc_data->data_s32[var->data_idx + chan];

	if((var->type & MSK_TYPE) != TYPE_INT32 ) {
		return ERR_INVALID_TYPE;
	}

	ret = acc_allowed( var, rdwr, req );
	if( ret != ERR_NONE ) {
		return ret;
	}

  if( chan > 0) {
		ret = vc_chk_vector( var, chan );
		if( ret != ERR_NONE ) {
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
static ERR_CODE vc_chk_vector( VAR_DESC const *var, int8_t chan )
{
	int ret;

	ret = ERR_NOT_VECTOR;
	if ( is_vector( var )) {
		ret = ERR_NONE;
		if(chan >= var->vec_items) {
			ret = ERR_INVALID_CHAN;
		}
	}
	return ret;
}

#ifdef TEST

#include <stdio.h>

#include "../vardefs.h"
#include "../vardef.inc"

int main( int argc, char **argv )
{
	int16_t ser = 0;
	int ret;
	UNUSED_PARAM( argc );
	UNUSED_PARAM( argv );

	vc_init( &g_var_data );

	printf( "sizeof(VAR_DESC) = %lu\n", sizeof(VAR_DESC));

	ser = 0xaa55;
	ret = vc_as_int16( VAR_SER, VarRead, &ser, 0, REQ_PRG );
	printf("rd SER 0x%hx, ret = %x\n", ser, ret );

	ser = 0x55aa;
	ret = vc_as_int16( VAR_SER, VarWrite, &ser, 0, REQ_PRG | REQ_ADMIN );
	printf("wr SER 0x%hx, ret = %x\n", ser, ret );
	ret = vc_as_int16( VAR_SER, VarRead,  &ser, 0, REQ_PRG );
	printf("rd SER 0x%hx, ret = %x\n", ser, ret );

#if 0
	for ( i = 0; i < VEC_LEM; i++ )
	{
		ffCur = -1.2 * (i + 1);
		vc_as_float( VAR_CUR, VarRead, &ffCur, i );
		printf("%d:CUR %f\n", i, ffCur);

		ffCur = 3.12f * (i + 1);
		vc_as_float( VAR_CUR, VarWrite, &ffCur, i );
		vc_as_float( VAR_CUR, VarRead,  &ffCur, i );
		printf("%d:CUR %f\n", i, ffCur);
	}
#endif

	return 0;
}

#endif
