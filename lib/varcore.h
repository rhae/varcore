/**
 * \file varcore.h
 *
 *
 *      Author: hae
 */

#ifndef VARCORE_H_
#define VARCORE_H_

#include "errcode.h"

#include <stdint.h>

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

	MSK_TYPE     = 0x000f,
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

	kReqPrg_R   = 0x0001,
	kReqCmd_R   = 0x0002,
  kReqEx1_R   = 0x0004,
	kReqEx2_R   = 0x0008,

	REQ_PRG_W   = REQ_PRG_R << 4,
	REQ_CMD_W   = REQ_CMD_R << 4,
	REQ_EX1_W   = REQ_EX1_R << 4,
	REQ_EX2_W   = REQ_EX2_R << 4,

	kReqPrg_W   = 0x0001,
	kReqCmd_W   = 0x0002,
  kReqEx1_W   = 0x0004,
	kReqEx2_W   = 0x0008,

	REQ_PRG     = REQ_PRG_W | REQ_PRG_R,
	REQ_CMD     = REQ_CMD_W | REQ_CMD_R,
	REQ_EX1     = REQ_EX1_W | REQ_EX1_R,
	REQ_EX2     = REQ_EX2_W | REQ_EX2_R,

	kReq_PRG     = kReqPrg_W | kReqPrg_R,
	kReq_CMD     = kReqCmd_W | kReqCmd_R,
	kReq_EX1     = kReqEx1_W | kReqEx1_R,
	kReq_EX2     = kReqEx2_W | kReqEx2_R,
	
	kReqAdmin    = 0x8000,
	kReqMsk      = 0x00ff,
	
	REQ_ADMIN   = 0x8000,

	MSK_ACC     = 0x00ff,
};

enum {
	FMT_DEFAULT,
	FMT_HEX2,
	FMT_HEX4,
	FMT_HEX8,
	FMT_DEC,

	kFmtDefault,
	kFmtHex4,
	kFmtHex8
};

typedef char   S8;
typedef short           S16;
typedef unsigned short  U16;
typedef int             S32;
typedef float  F32;
typedef double F64;

#ifndef STRBUF_SIZE
# define STRBUF_SIZE 16
#endif

typedef S16 HND;
typedef char STRBUF[32];

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

#if 0
typedef struct _DATA_ENUM {
	S16 cnt;
	S16 def_value;
	S16 
} DATA_ENUM;
#endif

typedef struct _VAR_DESC {
	HND         hnd;          /* Variablen handle */
	U16         type;
	U16         vec_items;
	U16         acc_rights;
	U16         descr_idx;
	U16         data_idx;
} VAR_DESC;

typedef struct _VC_DATA {
	VAR_DESC    *vars;
	HND          var_cnt;

	DATA_S16 const  *descr_s16;
	HND              descr_s16_cnt;

	DATA_S16        *data_s16;
	HND              data_s16_cnt;

	DATA_S32 const  *descr_s32;
	HND              descr_s32_cnt;

	DATA_S32        *data_s32;
	HND              data_s32_cnt;
/*
	DATA_STRING    *descr_str;
	HND             descr_str_cnt;
*/
	DATA_STRING    *data_str;
	HND             data_str_cnt;

#if 0
	DATA_F32    *descr_f32;
	HND          descr_f32_cnt;

	DATA_F32    *data_f32;
	HND          data_f32_cnt;

	DATA_F64    *data_f64;
	HND          data_f64_cnt;

	DATA_STRING *descr_str;
	HND          descr_str_cnt;

	DATA_STRING *data_str;
	HND          data_str_cnt;

	DATA_ENUM   *data_enum;
	HND          data_enum_cnt;

	DATA_ENUM_MBR   *data_mbr;
	HND              mbr_cnt;
#endif
} VC_DATA;

int vc_init( VC_DATA const* );
ErrCode vc_as_int16( HND hnd, int rdwr, S16 *val, U16 chan, U16 req );
ErrCode vc_as_int32( HND hnd, int rdwr, S32 *val, U16 chan, U16 req );
ErrCode vc_as_float( HND hnd, int rdwr, float *val, U16 chan, U16 req );
ErrCode vc_as_string( HND hnd, int rdwr, char *val, U16 chan, U16 req );

#endif /* VARCORE_H_ */
