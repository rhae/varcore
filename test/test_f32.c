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

static void wr_f32_min_max(void)
{
  F32 Min;
  F32 Max;
  F32 Mn;
  F32 Mx;
  ErrCode ret;
  HND hnd = VAR_CUR;

  typedef struct {
    F32      offset;
    ErrCode  err;
  } test_spec_t;

  test_spec_t test_min[] = {
    {  0, kErrNone },
    { +1, kErrNone },
    { -1, kErrLowerLimit }
  };

  test_spec_t test_max[] = {
    {  0, kErrNone },
    { +1, kErrUpperLimit },
    { -1, kErrNone }
  };

  ret = vc_get_min( hnd, (U8*)&Min, 0 );
  CU_ASSERT_EQUAL16( ret, kErrNone );

  ret = vc_get_max( hnd, (U8*)&Max, VEC_LEM-1 );
  CU_ASSERT_EQUAL16( ret, kErrNone );

  CU_ASSERT( Min < 0 );
  CU_ASSERT( Max > 0 );

  for( size_t i = 0; i < countof(test_min); i++) {
    test_spec_t *t = &test_min[i];
    Mn = Min + t->offset;
    ret = vc_as_float( hnd, VarWrite, &Mn, 0, REQ_PRG );
    CU_ASSERT_EQUAL( ret, t->err );
  }
  
  for( size_t i = 0; i < countof(test_max); i++) {
    test_spec_t *t = &test_max[i];
    Mx = Max + t->offset;
    ret = vc_as_float( hnd, VarWrite, &Mx, VEC_LEM-1, REQ_PRG );
    CU_ASSERT_EQUAL( ret, t->err );
  }
}

static void wr_f32_clip(void)
{
  F32 NMAX;
  F32 Min;
  F32 Max;
  F32 Mn;
  F32 Mx;
  ErrCode ret;
  HND hnd = VAR_CUR_NMAX;

  typedef struct {
    F32      offset;
    F32      exp_value;
    ErrCode  err;
  } test_spec_t;

  test_spec_t test_min[] = {
    {  0, 0, kErrNone },
    { +1, 0, kErrNone },
    { -1, 0, kErrNone }
  };

  test_spec_t test_max[] = {
    {  0, 0, kErrNone },
    { +1, 0, kErrNone },
    { -1, 0, kErrNone }
  };

  ret = vc_get_min( hnd, (U8*)&Min, 0 );
  CU_ASSERT_EQUAL16( ret, kErrNone );

  ret = vc_get_max( hnd, (U8*)&Max, 1 );
  CU_ASSERT_EQUAL16( ret, kErrNone );

  CU_ASSERT( Min < 0 );
  CU_ASSERT( Max > 0 );

  test_min[0].exp_value = Min;
  test_min[1].exp_value = Min+1;
  test_min[2].exp_value = Min;

  test_max[0].exp_value = Max;
  test_max[1].exp_value = Max;
  test_max[2].exp_value = Max-1;

  for( size_t i = 0; i < countof(test_min); i++) {
    test_spec_t *t = &test_min[i];
    Mn = Min + t->offset;
    ret = vc_as_float( hnd, VarWrite, &Mn, 0, REQ_PRG );
    CU_ASSERT_EQUAL16( ret, t->err );

    ret = vc_as_float( hnd, VarRead, &NMAX, 0, REQ_PRG );
    CU_ASSERT_EQUAL16( ret, kErrNone );
    CU_ASSERT_DOUBLE_EQUAL( NMAX, t->exp_value, 0.1 );
  }
  
  for( size_t i = 0; i < countof(test_max); i++) {
    test_spec_t *t = &test_max[i];
    Mx = Max + t->offset;
    // printf("%d: %2d   %d    %d/%x\n", i, t->offset, t->exp_value, t->err, t->err );
    ret = vc_as_float( hnd, VarWrite, &Mx, 1, REQ_PRG );
    CU_ASSERT_EQUAL16( ret, t->err );

    ret = vc_as_float( hnd, VarRead, &NMAX, 1, REQ_PRG );
    CU_ASSERT_EQUAL16( ret, kErrNone );
    CU_ASSERT_DOUBLE_EQUAL( NMAX, t->exp_value, 0.1 );
  }
}

static void set_min_max() {
  F32 Min;
  F32 Max;
  F32 Mn;
  F32 Mx;
  ErrCode ret;
  HND hnd = VAR_CUR_NMAX;

  ret = vc_get_min( hnd, 0, 0 );
  CU_ASSERT_EQUAL16( ret, kErrInvalidArg );

  ret = vc_get_max( hnd, 0, 1 );
  CU_ASSERT_EQUAL16( ret, kErrInvalidArg );

  ret = vc_set_min( hnd, 0, 0 );
  CU_ASSERT_EQUAL16( ret, kErrInvalidArg );

  ret = vc_set_max( hnd, 0, 0 );
  CU_ASSERT_EQUAL16( ret, kErrInvalidArg );

  ret = vc_get_min( hnd, (U8*)&Min, 0 );
  CU_ASSERT_EQUAL16( ret, kErrNone );

  ret = vc_get_max( hnd, (U8*)&Max, 1 );
  CU_ASSERT_EQUAL16( ret, kErrNone );

  Mn = Min -10;
  ret = vc_set_min( hnd, (U8*)&Mn, 0 );
  CU_ASSERT_EQUAL16( ret, kErrNone );
  
  ret = vc_get_min( hnd, (U8*)&Min, 0 );
  CU_ASSERT_EQUAL16( ret, kErrNone );
  CU_ASSERT_DOUBLE_EQUAL( Min, Mn, 0.1 );

  Mx = Max +10;
  ret = vc_set_max( hnd, (U8*)&Mx, 1 );
  CU_ASSERT_EQUAL16( ret, kErrNone );
  
  ret = vc_get_max( hnd, (U8*)&Max, 1 );
  CU_ASSERT_EQUAL16( ret, kErrNone );
  CU_ASSERT_DOUBLE_EQUAL( Max, Mx, 0.1 );
}

void get_storage_f32() {
  ErrCode ret;
  U16 storage;

  storage = -1;
  ret = vc_get_storage( VAR_CUR, &storage );
  CU_ASSERT_EQUAL16( ret, kErrNone );
  CU_ASSERT_EQUAL16( storage, RAM_VOLATILE );

  storage = -1;
  ret = vc_get_storage( VAR_CUR_NMAX, &storage );
  CU_ASSERT_EQUAL16( ret, kErrNone );
  CU_ASSERT_EQUAL16( storage, EEPROM );

  storage = -1;
  ret = vc_get_storage( VAR_CUR_PMAX, &storage );
  CU_ASSERT_EQUAL16( ret, kErrNone );
  CU_ASSERT_EQUAL16( storage, FLASH );
}

static CU_TestInfo tests_rdwr_f32[] = {
  { "F32, RD",           rd_f32 },
  { "F32, WR",           wr_f32 },
  { "F32, As string",    as_str_f32 },
  { "F32, WR min/max",   wr_f32_min_max },
  { "F32, WR clip",      wr_f32_clip },
  { "F32, SET MIN/MAX",  set_min_max },
  { "F32, GET STORAGE",  get_storage_f32 },
	CU_TEST_INFO_NULL,
};


static CU_SuiteInfo suites[] = {
  { "variable F32",  suite_init, suite_clean, NULL, NULL, tests_rdwr_f32 },
	CU_SUITE_INFO_NULL,
};

void test_add_f32(void)
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
