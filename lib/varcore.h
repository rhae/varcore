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
 * \file varcore.h
 * \author: hae
 */

#pragma once

#include "errcode.h"

#include <stdint.h>

/* constant definitions
----------------------------------------------------------------------------*/
enum {
	VarWrite  = 0,
	VarRead   = 1,
};

enum {
	TYPE_INT8    =  0,
	TYPE_INT16   =  1,
	TYPE_INT32   =  2,
	TYPE_INT64   =  3,
	TYPE_FLOAT   =  4,
	TYPE_DOUBLE  =  5,
	TYPE_ENUM    =  6,
	TYPE_STRING  =  7,
	TYPE_ACTION  =  8,

	TYPE_LAST,  /* Use only for Array size */

	TYPE_VECTOR  = 0x1000,
	TYPE_CONST   = 0x2000,

	TYPE_FLAG    = 0xf000,
	TYPE_MASK    = 0x000f,
};

enum {
	RAM_VOLATILE   = 0x0000,
	EEPROM         = 0x2000,
	FLASH          = 0x4000,

	MSK_STORAGE    = 0x6000,
};

enum {
	REQ_PRG_R   = 0x0001,
	REQ_CMD_R   = 0x0002,
    REQ_EX1_R   = 0x0004,
	REQ_EX2_R   = 0x0008,

	REQ_PRG_W   = REQ_PRG_R << 4,
	REQ_CMD_W   = REQ_CMD_R << 4,
	REQ_EX1_W   = REQ_EX1_R << 4,
	REQ_EX2_W   = REQ_EX2_R << 4,

	REQ_PRG     = REQ_PRG_W | REQ_PRG_R,
	REQ_CMD     = REQ_CMD_W | REQ_CMD_R,
	REQ_EX1     = REQ_EX1_W | REQ_EX1_R,
	REQ_EX2     = REQ_EX2_W | REQ_EX2_R,
	
	REQ_ADMIN   = 0x8000,
	FLAG_LIMIT  = 0x4000,
	FLAG_CLIP   = 0x2000,

	MSK_ACC     = 0x00ff,
	REQ_FLAG    = 0xff00
};

enum {
	FMT_DEFAULT,
	FMT_HEX2,
	FMT_HEX4,
	FMT_HEX8,
	FMT_DEC,
};

/* global defined data types
----------------------------------------------------------------------------*/
typedef char            S8;
typedef unsigned char   U8;
typedef short           S16;
typedef unsigned short  U16;
typedef int             S32;
typedef float           F32;
typedef double          F64;

#ifndef STRBUF_SIZE
# define STRBUF_SIZE 16
#endif

#define HNON (U16)-1

typedef S16             HND;
typedef char            STRBUF[32];

typedef struct {
	uint8_t    val;       /* Wert des Enums */
	int        str_idx;   /* Index in string-Liste */
} ENUM_DESC;

typedef struct _DATA_S16 {
	S16 def_value;
	S16 min;
	S16 max;
} DATA_S16;

typedef struct _DATA_S32 {
	S32 def_value;
	S32 min;
	S32 max;
} DATA_S32;

typedef struct _DATA_F32 {
	F32 def_value;
	F32 min;
	F32 max;
} DATA_F32;

typedef struct _DATA_F64 {
	F64 def_value;
	F64 min;
	F64 max;
} DATA_F64;

typedef S8 DATA_STRING;

typedef struct _ENUM_MBR {
	S16 hnd;
	S16 value;
	S16 symbol;
} ENUM_MBR;

typedef struct _DESCR_ENUM {
	S16 def_value;
	U16 cnt;
	ENUM_MBR mbr[];
} DESCR_ENUM;

typedef S16 DATA_ENUM;
typedef S16 DATA_ENUM_MBR;



typedef struct _VAR_DESC {
	HND         hnd;          /* Variablen handle */
	U16         scpi_idx;
	U16         type;
	U16         vec_items;
	U16         acc_rights;
	U16         descr_idx;
	U16         data_idx;
} VAR_DESC;

typedef struct _VC_DATA {
	VAR_DESC const   *vars;
	HND const         var_cnt;

	DATA_S16 const  *descr_s16;
	HND              descr_s16_cnt;

	DATA_S16        *data_s16;
	HND              data_s16_cnt;

	DATA_S32 const  *descr_s32;
	HND              descr_s32_cnt;

	DATA_S32        *data_s32;
	HND              data_s32_cnt;

	DATA_STRING    *data_str;
	HND             data_str_cnt;

	DATA_STRING const *data_const_str;
	HND                data_const_str_cnt;

	S16             *data_enum;
	HND              data_enum_cnt;

	S16 const       *data_mbr;
	HND              mbr_cnt;

	DATA_F32 const  *descr_f32;
	HND              descr_f32_cnt;

	DATA_F32        *data_f32;
	HND              data_f32_cnt;

	DATA_F64 const  *descr_f64;
	HND              descr_f64_cnt;

	DATA_F64        *data_f64;
	HND              data_f64_cnt;
#if 0
	DATA_STRING *descr_str;
	HND          descr_str_cnt;

	DATA_STRING *data_str;
	HND          data_str_cnt;

#endif
} VC_DATA;

/* list of global defined functions
----------------------------------------------------------------------------*/
ErrCode vc_init( VC_DATA const* );
ErrCode vc_reset();
ErrCode vc_as_int16( HND hnd, int rdwr, S16 *val, U16 chan, U16 req );
ErrCode vc_as_int32( HND hnd, int rdwr, S32 *val, U16 chan, U16 req );
ErrCode vc_as_float( HND hnd, int rdwr, F32 *val, U16 chan, U16 req );
ErrCode vc_as_string( HND hnd, int rdwr, char *val, U16 chan, U16 req );

ErrCode vc_get_min( HND, U8*, U16 );
ErrCode vc_get_max( HND, U8*, U16 );

int vc_dump_var( char *, U16, HND, U16 );
