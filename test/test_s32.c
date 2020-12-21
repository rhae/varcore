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
#include "test_utils.h"

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

extern VC_DATA g_var_data;


/* Suite initialization/cleanup functions */
static int suite_init(void) {
  vc_init(&g_var_data);
  return 0;
}

static int suite_clean(void) {
  return 0; 
}


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

static void wr32_min_max(void)
{
  S32 Min = 0;
  S32 Max = 0;
  S32 Mn;
  S32 Mx;
	ErrCode ret;

  /* Assumption: Min < 0, Max > 0 !! */
  
  ret = vc_get_min( VAR_POW, (U8*)&Min, 0 );
  CU_ASSERT_EQUAL( ret, kErrNone );

  ret = vc_get_max( VAR_POW, (U8*)&Max, 0 );
  CU_ASSERT_EQUAL( ret, kErrNone );
  
  Mn = Min;
  ret = vc_as_int32( VAR_POW, VarWrite, &Mn, 0, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrNone );

  Mn = Min-1;
  ret = vc_as_int32( VAR_POW, VarWrite, &Mn, 0, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrLowerLimit );

  Mn = Min+1;
  ret = vc_as_int32( VAR_POW, VarWrite, &Mn, 0, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrNone );

  Mx = Max;
  ret = vc_as_int32( VAR_POW, VarWrite, &Mx, 0, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrNone );

  Mx = Max-1;
  ret = vc_as_int32( VAR_POW, VarWrite, &Mx, 0, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrNone );

  Mx = Max+1;
  ret = vc_as_int32( VAR_POW, VarWrite, &Mx, 0, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrUpperLimit );
}

static void wr32_clip(void)
{
  S32 PAB = 0;
  S32 Min = 0;
  S32 Max = 0;
  S32 Mn;
  S32 Mx;
	ErrCode ret;


  /* Assumption: Min < 0, Max > 0 !! */
  
  ret = vc_get_min( VAR_PAB, (U8*)&Min, 0 );
  CU_ASSERT_EQUAL( ret, kErrNone );

  ret = vc_get_max( VAR_PAB, (U8*)&Max, 0 );
  CU_ASSERT_EQUAL( ret, kErrNone );
  
  Mn = Min;
  ret = vc_as_int32( VAR_PAB, VarWrite, &Mn, 0, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrNone );

  Mn = Min-1;
  ret = vc_as_int32( VAR_PAB, VarWrite, &Mn, 0, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrNone );
  ret = vc_as_int32( VAR_PAB, VarRead, &PAB, 0, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrNone );
  CU_ASSERT_EQUAL( PAB, Min );

  Mn = Min+1;
  ret = vc_as_int32( VAR_PAB, VarWrite, &Mn, 0, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrNone );
  ret = vc_as_int32( VAR_PAB, VarRead, &PAB, 0, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrNone );
  CU_ASSERT_EQUAL( PAB, Mn );

  Mx = Max;
  ret = vc_as_int32( VAR_PAB, VarWrite, &Mx, 0, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrNone );
  ret = vc_as_int32( VAR_PAB, VarRead, &PAB, 0, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrNone );
  CU_ASSERT_EQUAL( PAB, Mx );

  Mx = Max-1;
  ret = vc_as_int32( VAR_PAB, VarWrite, &Mx, 0, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrNone );
  ret = vc_as_int32( VAR_PAB, VarRead, &PAB, 0, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrNone );
  CU_ASSERT_EQUAL( PAB, Mx );

  Mx = Max+1;
  ret = vc_as_int32( VAR_PAB, VarWrite, &Mx, 0, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrNone );
  ret = vc_as_int32( VAR_PAB, VarRead, &PAB, 0, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrNone );
  CU_ASSERT_EQUAL( PAB, Max );
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

static void as_str32(void)
{
  ErrCode ret;
  STRBUF S;

  S32 ERR;

  memset( S, 'S', sizeof(S));
  ERR = 0xffff;
  ret = vc_as_int32( VAR_ERR, VarWrite, &ERR, 0, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrNone );

  ret = vc_as_string( VAR_ERR, VarRead, S, 0, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrNone );
  CU_ASSERT_STRING_EQUAL( S, "0xffff" );

  memset( S, 'S', sizeof(S));
  ERR = 0xffffffff;
  ret = vc_as_int32( VAR_ERR, VarWrite, &ERR, 0, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrNone );

  ret = vc_as_string( VAR_ERR, VarRead, S, 0, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrNone );
  CU_ASSERT_STRING_EQUAL( S, "0xffffffff" );
}

static void set_min_max( void ) {
  S16 Min;
  S16 Max;
  S16 Mn;
  S16 Mx;
  ErrCode ret;
  HND hnd = VAR_TP1;

  ret = vc_get_min( hnd, (U8*)&Min, 0 );
  CU_ASSERT_EQUAL16( ret, kErrNone );

  ret = vc_get_max( hnd, (U8*)&Max, 1 );
  CU_ASSERT_EQUAL16( ret, kErrNone );

  Mn = Min -10;
  ret = vc_set_min( hnd, (U8*)&Mn, 0 );
  CU_ASSERT_EQUAL16( ret, kErrNone );
  
  ret = vc_get_min( hnd, (U8*)&Min, 0 );
  CU_ASSERT_EQUAL16( ret, kErrNone );
  CU_ASSERT_EQUAL32( Min, Mn );

  Mx = Max +10;
  ret = vc_set_max( hnd, (U8*)&Mx, 1 );
  CU_ASSERT_EQUAL16( ret, kErrNone );
  
  ret = vc_get_max( hnd, (U8*)&Max, 1 );
  CU_ASSERT_EQUAL16( ret, kErrNone );
  CU_ASSERT_EQUAL32( Max, Mx );
}

static CU_TestInfo tests_rdwr32[] = {
  { "S32, RD",          rd32 },
  { "S32, WR",           wr32 },
  { "S32, RD chan x",    rd32_chan },
  { "S32, WR chan x",    wr32_chan },
  { "S32, AS string",    as_str32 },
  { "S32, WR min/max",   wr32_min_max },
  { "S32, WR clip",      wr32_clip },
  { "S32, SET MIN/MAX",  set_min_max },
	CU_TEST_INFO_NULL,
};



/*** Suite definition  ******************************************************/

static CU_SuiteInfo suites[] = {
  { "variable S32",  suite_init, suite_clean, NULL, NULL, tests_rdwr32 },
	CU_SUITE_INFO_NULL,
};

void test_add_s32(void)
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
