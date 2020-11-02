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

#include "CUnit/CUnit.h"
#include "varcore_test.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


/* WARNING - MAINTENANCE NIGHTMARE AHEAD
 *
 * If you change any of the tests & suites below, you also need
 * to keep track of changes in the result statistics and reflect
 * any changes in the result report counts in print_example_results().
 *
 * Yes, this could have been designed better using a more
 * automated mechanism.  No, it was not done that way.
 */

#include <varcore.h>

#include "vardefs.h"
#include "vardef.inc"


#define xstr(s) str(s)
#define str(s) #s

#define CU_ASSERT_EQUAL16(actual, expected) \
  do { \
    char condition[256]; \
    sprintf( condition, "CU_ASSERT_EQUAL16( " xstr(actual) " = %d/%xh, %d )", actual, actual, expected ) ;\
    CU_assertImplementation(((actual) == (expected)), __LINE__, condition, __FILE__, "", CU_FALSE); \
  } while(0)

#define CU_ASSERT_EQUAL32(actual, expected) \
  do { \
    char condition[256]; \
    sprintf( condition, "CU_ASSERT_EQUAL32( " xstr(actual) " = %d/%x, %d )", actual, actual, expected ) ;\
    CU_assertImplementation(((actual) == (expected)), __LINE__, condition, __FILE__, "", CU_FALSE); \
  } while(0)

/* Suite initialization/cleanup functions */
static int suite_init(void) {
  vc_init(&g_var_data);
  return 0;
}

static int suite_clean(void) {
  return 0; 
}

#if 0
  CU_ASSERT_TRUE(CU_TRUE);
  CU_ASSERT_FALSE(CU_FALSE);
  CU_ASSERT_EQUAL(10, 10);
  CU_ASSERT_NOT_EQUAL(10, 11);


  CU_ASSERT_PTR_EQUAL((void*)0x100, (void*)0x100);
  CU_ASSERT_PTR_NOT_EQUAL((void*)0x100, (void*)0x101);

  CU_ASSERT_PTR_NULL((void*)(0x0));
  CU_ASSERT_PTR_NOT_NULL((void*)0x100);

  char str1[] = "test";
  char str2[] = "test";
  char str3[] = "suite";


  CU_ASSERT_STRING_EQUAL(str1, str2);
  CU_ASSERT_NSTRING_EQUAL(str1, str3, strlen(str1));
  CU_ASSERT_NSTRING_NOT_EQUAL(str2, str3, 2);

  CU_ASSERT_DOUBLE_EQUAL(10, 10.0001, 0.0001);
  CU_ASSERT_DOUBLE_NOT_EQUAL(-10, -10.001, -0.01);
#endif

/*** integer32 tests ********************************************************/
static void rd32(void)
{
  S32 ser = 0xaa55;
	ErrCode ret = vc_as_int32( VAR_SER, VarRead, &ser, 0, REQ_PRG );

  CU_ASSERT_EQUAL( ret, kErrNone );
  CU_ASSERT_EQUAL( ser, 10000 );
}

static void wr32(void)
{
  S32 ser = 0xaa55;
	ErrCode ret;
  
  ret = vc_as_int32( VAR_SER, VarWrite, &ser, 0, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrAccessDenied );

  ret = vc_as_int32( VAR_SER, VarWrite, &ser, 0, REQ_PRG | REQ_ADMIN );
  CU_ASSERT_EQUAL( ret, kErrNone );

  ser = 1000;
  ret = vc_as_int32( VAR_SER, VarRead, &ser, 0, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrNone );
  CU_ASSERT_EQUAL( ser, 0xaa55 );
}

static void rd32_chan(void) {
  S32 power[VEC_LEM] = { 0, 0, 0, 0 };
  ErrCode ret;

  for( int i = 0; i < VEC_LEM; i++ ) {
    power[i] = -99;
  }

  ret = vc_as_int32( VAR_POW, VarRead, &power[0], 0, REQ_PRG );
  CU_ASSERT_EQUAL32( ret, kErrNone );
  CU_ASSERT_EQUAL32( power[0], 0 );

  power[0] = -99;
  ret = vc_as_int32( VAR_POW, VarRead, &power[0], VEC_LEM, REQ_PRG );
  CU_ASSERT_EQUAL32( ret, kErrInvalidChan );
  CU_ASSERT_EQUAL32( power[0], -99 );

  power[1] = -99;
  ret = vc_as_int32( VAR_POW, VarRead, &power[1], 1, REQ_PRG );
  CU_ASSERT_EQUAL32( ret, kErrNone );
  CU_ASSERT_EQUAL32( power[1], 0 );
}

static void wr32_chan(void)
{
  S32 power[VEC_LEM] = { 0, 0, 0, 0 };
  ErrCode ret;

  for( int i = 0; i < VEC_LEM; i++ ) {
    power[i] = -99;
  }

  ret = vc_as_int32( VAR_POW, VarWrite, &power[0], 0, REQ_PRG );
  CU_ASSERT_EQUAL32( ret, kErrNone );
  power[0] = 99;
  ret = vc_as_int32( VAR_POW, VarRead, &power[0], 0, REQ_PRG );
  CU_ASSERT_EQUAL32( ret, kErrNone );
  CU_ASSERT_EQUAL32( power[0], -99 );


  ret = vc_as_int32( VAR_POW, VarWrite, &power[VEC_LEM-1], VEC_LEM-1, REQ_PRG );
  CU_ASSERT_EQUAL32( ret, kErrNone );
  power[VEC_LEM-1] = 99;
  ret = vc_as_int32( VAR_POW, VarRead, &power[VEC_LEM-1], VEC_LEM-1, REQ_PRG );
  CU_ASSERT_EQUAL32( ret, kErrNone );
  CU_ASSERT_EQUAL32( power[VEC_LEM-1], -99 );

  ret = vc_as_int32( VAR_POW, VarWrite, &power[0], VEC_LEM, REQ_PRG );
  CU_ASSERT_EQUAL32( ret, kErrInvalidChan );
}


static CU_TestInfo tests_rdwr32[] = {
  { "RD", rd32 },
  { "WR", wr32 },
  { "RD chan x", rd32_chan },
  { "WR chan x", wr32_chan },
	CU_TEST_INFO_NULL,
};


/*** integer16 tests ********************************************************/

static void rd16(void)
{
  S16 node_id = 127;
	ErrCode ret;
  
  ret = vc_as_int16( VAR_CO_NODEID, VarRead, &node_id, 0, REQ_PRG );

  CU_ASSERT_EQUAL16( ret, kErrNone );
  CU_ASSERT_EQUAL16( node_id, 1 );
}

static void wr16(void)
{
  S16 node_id = 127;
	ErrCode ret;
  
  ret = vc_as_int16( VAR_CO_NODEID, VarWrite, &node_id, 0, REQ_PRG );
  CU_ASSERT_EQUAL16( ret, kErrNone );

  node_id = 25;
  ret = vc_as_int16( VAR_CO_NODEID, VarRead, &node_id, 0, REQ_PRG );
  CU_ASSERT_EQUAL16( ret, kErrNone );
  CU_ASSERT_EQUAL16( node_id, 127 );
}

static void rd16_chan(void)
{
  S16 temp[VEC_LEM] = { 0, 0, 0, 0 };
  ErrCode ret;

  for( int i = 0; i < VEC_LEM; i++ ) {
    temp[i] = -99;
  }

  ret = vc_as_int16( VAR_TP1, VarRead, &temp[0], 0, REQ_PRG );
  CU_ASSERT_EQUAL16( ret, kErrNone );
  CU_ASSERT_EQUAL16( temp[0], 0 );

  temp[0] = -99;
  ret = vc_as_int16( VAR_TP1, VarRead, &temp[0], VEC_LEM, REQ_PRG );
  CU_ASSERT_EQUAL16( ret, kErrInvalidChan );
  CU_ASSERT_EQUAL16( temp[0], -99 );

  temp[1] = -99;
  ret = vc_as_int16( VAR_TP1, VarRead, &temp[1], 1, REQ_PRG );
  CU_ASSERT_EQUAL16( ret, kErrNone );
  CU_ASSERT_EQUAL16( temp[1], 0 );
}

static void wr16_chan(void)
{
  S16 temp[VEC_LEM] = { 0, 0, 0, 0 };
  ErrCode ret;

  for( int i = 0; i < VEC_LEM; i++ ) {
    temp[i] = -99;
  }

  ret = vc_as_int16( VAR_TP1, VarWrite, &temp[0], 0, REQ_PRG );
  CU_ASSERT_EQUAL16( ret, kErrNone );
  temp[0] = 99;
  ret = vc_as_int16( VAR_TP1, VarRead, &temp[0], 0, REQ_PRG );
  CU_ASSERT_EQUAL16( ret, kErrNone );
  CU_ASSERT_EQUAL16( temp[0], -99 );


  temp[VEC_LEM-1] = 98;
  ret = vc_as_int16( VAR_TP1, VarWrite, &temp[VEC_LEM-1], VEC_LEM-1, REQ_PRG );
  CU_ASSERT_EQUAL16( ret, kErrNone );
  temp[VEC_LEM-1] = 99;
  ret = vc_as_int16( VAR_TP1, VarRead, &temp[VEC_LEM-1], VEC_LEM-1, REQ_PRG );
  CU_ASSERT_EQUAL16( ret, kErrNone );
  CU_ASSERT_EQUAL16( temp[VEC_LEM-1], 98 );

  ret = vc_as_int16( VAR_TP1, VarWrite, &temp[0], VEC_LEM, REQ_PRG );
  CU_ASSERT_EQUAL16( ret, kErrInvalidChan );
}

static void as_str16(void)
{
  ErrCode ret;
  STRBUF S;

  S16 node_id = 16;
  S16 tmp = 36;

  // read S16, read str
  ret = vc_as_int16( VAR_CO_NODEID, VarWrite, &node_id, 0, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrNone );

  ret = vc_as_string( VAR_CO_NODEID, VarRead, S, 0, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrNone );
  CU_ASSERT_STRING_EQUAL( S, "16");

  // write str, read S16
  sprintf( S, "18");
  ret = vc_as_string( VAR_CO_NODEID, VarWrite, S, 0, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrNone );

  ret = vc_as_int16( VAR_CO_NODEID, VarRead, &node_id, 0, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrNone );
  CU_ASSERT_EQUAL( node_id, 18 );

  // write S16 vector, read str
  tmp = 36;
  ret = vc_as_int16( VAR_TP1, VarWrite, &tmp, 3, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrNone );

  ret = vc_as_string( VAR_TP1, VarRead, S, 3, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrNone );
  CU_ASSERT_STRING_EQUAL( S, "36");

  // write str vector, read S16
  sprintf( S, "18");
  ret = vc_as_string( VAR_TP1, VarWrite, S, 4, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrNone );
  
  ret = vc_as_int16( VAR_TP1, VarRead, &tmp, 4, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrNone );
  CU_ASSERT_STRING_EQUAL( S, "18");

  // write str vector (float to int16)
  sprintf( S, "18.23");
  ret = vc_as_string( VAR_TP1, VarWrite, S, 4, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrInvalidValue );
}

static CU_TestInfo tests_rdwr16[] = {
  { "RD", rd16 },
  { "WR", wr16 },
  { "RD chan x", rd16_chan },
  { "WR chan x", wr16_chan },
  { "as string", as_str16 },
	CU_TEST_INFO_NULL,
};

/*** float (f32) tests ********************************************************/

static void rd_f32(void)
{
  F32 NCUR = -1;
  ErrCode E = kErrGeneric;

  E = vc_as_float( VAR_CUR_NMAX, VarRead, &NCUR, 0, REQ_PRG );
  CU_ASSERT_EQUAL( E, kErrNone );
  CU_ASSERT_DOUBLE_EQUAL( NCUR, -500, .001 );

  NCUR = -1;
  E = vc_as_float( VAR_CUR_NMAX, VarRead, &NCUR, VEC_LEM, REQ_PRG );
  CU_ASSERT_EQUAL( E, kErrInvalidChan );
  CU_ASSERT_DOUBLE_EQUAL( NCUR, -1, .001 );
}

static void wr_f32(void)
{
  F32 NCUR;
  ErrCode E = kErrGeneric;

  NCUR = -10;
  E = vc_as_float( VAR_CUR_NMAX, VarWrite, &NCUR, 0, REQ_PRG );
  CU_ASSERT_EQUAL( E, kErrNone );

  NCUR = -1;
  E = vc_as_float( VAR_CUR_NMAX, VarRead, &NCUR, 0, REQ_PRG );
  CU_ASSERT_EQUAL( E, kErrNone );
  CU_ASSERT_DOUBLE_EQUAL( NCUR, -10, .001 );
  
  NCUR = -1;
  E = vc_as_float( VAR_CUR_NMAX, VarWrite, &NCUR, VEC_LEM, REQ_PRG );
  CU_ASSERT_EQUAL( E, kErrInvalidChan );
  CU_ASSERT_DOUBLE_EQUAL( NCUR, -1, .001 );
}

static void as_str_f32(void)
{
  STRBUF S;
  F32 NCUR;
  ErrCode E = kErrGeneric;

  strcpy( S, "10.0" );
  E = vc_as_string( VAR_CUR_NMAX, VarWrite, S, 0, REQ_PRG );
  CU_ASSERT_EQUAL( E, kErrNone );

  NCUR = -1;
  E = vc_as_float( VAR_CUR_NMAX, VarRead, &NCUR, 0, REQ_PRG );
  CU_ASSERT_EQUAL( E, kErrNone );
  CU_ASSERT_DOUBLE_EQUAL( NCUR, 10, .001 );
}

static CU_TestInfo tests_rdwr_f32[] = {
  { "RD", rd_f32 },
  { "WR", wr_f32 },
  { "as string", as_str_f32 },
	CU_TEST_INFO_NULL,
};

/*** volatile string tests **************************************************/

static void rd_str(void) {
  STRBUF S;
  ErrCode ret;
  
  ret = vc_as_string( VAR_NAS, VarRead, S, 0, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrNone );
  CU_ASSERT_STRING_EQUAL( S, "192.168.2.10" );


  ret = vc_as_string( VAR_NAS, VarRead, S, 1, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrNone );
  CU_ASSERT_STRING_EQUAL( S, "192.168.2.10" );

  ret = vc_as_string( VAR_NAS, VarRead, S, 2, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrInvalidChan );
}

static void wr_str(void) {
  enum {
    BigBuf = sizeof(STRBUF)*2
  };
  ErrCode ret;
  STRBUF S;
  char T[BigBuf];
  memset( T, 'S', BigBuf );

  /* write string */
  ret = vc_as_string( VAR_NAS, VarWrite, 0, 1, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrInvalidArg );

  /* write string */
  strcpy( S, "192.168.178.22" );
  ret = vc_as_string( VAR_NAS, VarWrite, S, 1, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrNone );

  /* read back */
  memset( S, 'F', sizeof(STRBUF));
  ret = vc_as_string( VAR_NAS, VarRead, S, 1, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrNone );
  CU_ASSERT_STRING_EQUAL( S, "192.168.178.22" );

  /* write string that is too big */
  ret = vc_as_string( VAR_NAS, VarWrite, T, 1, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrSizeTooBig );

  /* check that old value still exists */
  ret = vc_as_string( VAR_NAS, VarRead, S, 1, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrNone );
  CU_ASSERT_STRING_EQUAL( S, "192.168.178.22" );
}

static CU_TestInfo tests_rdwr_str[] = {
  { "RD", rd_str },
  { "WR", wr_str },
  #if 0
  { "RD chan x", rd16_chan },
  { "WR chan x", wr16_chan },
  { "as string", as_str16 },
  #endif
	CU_TEST_INFO_NULL,
};

/*** constant string tests **************************************************/

static void rd_const_str(void) {
  STRBUF S;
  ErrCode ret;
  
  ret = vc_as_string( VAR_IDN, VarRead, S, 0, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrNone );
  CU_ASSERT_STRING_EQUAL( S, "Test application V1.01 (R) foo" );

  ret = vc_as_string( VAR_OFF, VarRead, S, 0, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrNone );
  CU_ASSERT_STRING_EQUAL( S, "OFF" );
}

static void wr_const_str(void) {
  STRBUF S;
  ErrCode ret;

  sprintf( S, "Hello World!");  
  ret = vc_as_string( VAR_IDN, VarWrite, S, 0, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrAccessDenied );
}

static CU_TestInfo tests_rd_const_str[] = {
  { "RD CONST STRING", rd_const_str },
  { "WR CONST STRING", wr_const_str },
	CU_TEST_INFO_NULL,
};

/*** enum tests *************************************************************/

static void rd_enum(void) {
  S16 LOD;
  S16 XON = -1;
  ErrCode ret;
  
  /* Important: Use a enum that has a initial value != 0 */
  ret = vc_as_int16( VAR_XON, VarRead, &XON, 0, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrNone );
  CU_ASSERT_EQUAL( XON, 1 );

  LOD = -1;
  ret = vc_as_int16( VAR_LOD, VarRead, &LOD, 10, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrNoVector );
  CU_ASSERT_EQUAL( LOD, -1 );

}

static void wr_enum(void) {
  S16 LOD;
  S16 YNU;
  ErrCode ret;
  
  LOD = 1;
  ret = vc_as_int16( VAR_LOD, VarWrite, &LOD, 0, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrNone );
  
  LOD = 99;
  ret = vc_as_int16( VAR_LOD, VarRead, &LOD, 0, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrNone );
  CU_ASSERT_EQUAL( LOD, 1 );

  YNU = 99;
  ret = vc_as_int16( VAR_YNU, VarRead, &YNU, 0, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrNone );
  CU_ASSERT_EQUAL( YNU, -2 );

  YNU = -1;
  ret = vc_as_int16( VAR_YNU, VarWrite, &YNU, 5, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrNone );
  YNU = 99;
  ret = vc_as_int16( VAR_YNU, VarRead, &YNU, 5, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrNone );
  CU_ASSERT_EQUAL( YNU, -1 );
}

static CU_TestInfo tests_rdwr_enum[] = {
  { "RD ENUM", rd_enum },
  { "WR ENUM", wr_enum },
	CU_TEST_INFO_NULL,
};

/*** dump tests **********************************************************/

static void dump(void) {

  char *buf = calloc( 1024, 1 );
  int n;

  vc_dump_var( buf, 1024, VAR_LOD, 0 );
  puts( buf );

  vc_dump_var( buf, 1024, VAR_YNU, 0 );
  puts( buf );

  vc_dump_var( buf, 1024, VAR_TP1, 0 );
  puts( buf );

  vc_dump_var( buf, 1024, VAR_CO_NODEID, 0 );
  puts( buf );

  vc_dump_var( buf, 1024, VAR_CUR, 0 );
  puts( buf );

  // buffer too small. Not filled but closed with '\0'.
  n = vc_dump_var( buf, 10, VAR_CUR, 0 );
  CU_ASSERT_EQUAL( n, 0 );
  CU_ASSERT_EQUAL( strlen(buf), 0 );

  // buffer is partly filled.
  memset( buf, 'F', 1024 );
  n = vc_dump_var( buf, 282, VAR_CUR, 0 );
  CU_ASSERT_EQUAL( n, 270 );
  CU_ASSERT_EQUAL( strlen(buf), 270 );

  S16 YNU = -1;
  vc_as_int16( VAR_YNU, VarWrite, &YNU, 0, REQ_PRG );
  vc_as_int16( VAR_YNU, VarWrite, &YNU, 5, REQ_PRG );
  vc_dump_var( buf, 1024, VAR_YNU, 0 );
  puts( buf );

  free(buf);
}

static CU_TestInfo tests_dump[] = {
  { "Dump variables", dump },
	CU_TEST_INFO_NULL,
};

/*** Suite definition  ******************************************************/

static CU_SuiteInfo suites[] = {
  { "variable S32",  suite_init, suite_clean, NULL, NULL, tests_rdwr32 },
  { "variable S16",  suite_init, suite_clean, NULL, NULL, tests_rdwr16 },
  { "variable F32",  suite_init, suite_clean, NULL, NULL, tests_rdwr_f32 },
  { "variable str",  suite_init, suite_clean, NULL, NULL, tests_rdwr_str },
  { "variable const str",  suite_init, suite_clean, NULL, NULL, tests_rd_const_str },
  { "variable enum", suite_init, suite_clean, NULL, NULL, tests_rdwr_enum },
  { "variable dump", suite_init, suite_clean, NULL, NULL, tests_dump },
	CU_SUITE_INFO_NULL,
};

void S16_AddTests(void)
{
  assert(NULL != CU_get_registry());
  assert(!CU_is_test_running());

	/* Register suites. */
	if (CU_register_suites(suites) != CUE_SUCCESS) {
		fprintf(stderr, "suite registration failed - %s\n",
			CU_get_error_msg());
		exit(EXIT_FAILURE);
	}
}
