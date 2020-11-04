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
    temp[i] = -71;
  }

  ret = vc_as_int16( VAR_TP1, VarWrite, &temp[0], 0, REQ_PRG );
  CU_ASSERT_EQUAL16( ret, kErrNone );
  temp[0] = 99;
  ret = vc_as_int16( VAR_TP1, VarRead, &temp[0], 0, REQ_PRG );
  CU_ASSERT_EQUAL16( ret, kErrNone );
  CU_ASSERT_EQUAL16( temp[0], -71 );


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

static void wr16_min_max(void)
{
  S16 Min;
  S16 Max;
  S16 Mn;
  S16 Mx;
  ErrCode ret;

  typedef struct {
    S16      offset;
    ErrCode  err;
  } test_spec_t;

  test_spec_t test_min[] = {
    {  0, kErrNone },
    { +1, kErrNone },
    { -1, kErrLowerLimit },
    {  0, kErrLowerLimit },
  };

  test_spec_t test_max[] = {
    {  0, kErrNone },
    { +1, kErrUpperLimit },
    { -1, kErrNone },
    {  0, kErrUpperLimit },
  };

  ret = vc_get_min( VAR_IAB, (U8*)&Min, 0 );
  CU_ASSERT_EQUAL16( ret, kErrNone );

  ret = vc_get_max( VAR_IAB, (U8*)&Max, VEC_LEM-1 );
  CU_ASSERT_EQUAL16( ret, kErrNone );

  CU_ASSERT( Min < 0 );
  CU_ASSERT( Max > 0 );

  test_min[3].offset = 0x8000 - Min;
  test_max[3].offset = 0x7fff - Max;

  for( size_t i = 0; i < countof(test_min); i++) {
    test_spec_t *t = &test_min[i];
    Mn = Min + t->offset;
    ret = vc_as_int16( VAR_IAB, VarWrite, &Mn, 0, REQ_PRG );
    CU_ASSERT_EQUAL16( ret, t->err );
  }
  
  for( size_t i = 0; i < countof(test_max); i++) {
    test_spec_t *t = &test_max[i];
    Mx = Max + t->offset;
    ret = vc_as_int16( VAR_IAB, VarWrite, &Mx, VEC_LEM-1, REQ_PRG );
    CU_ASSERT_EQUAL16( ret, t->err );
  }
}

static void wr16_clip(void)
{
  S16 TP1;
  S16 Min;
  S16 Max;
  S16 Mn;
  S16 Mx;
  ErrCode ret;

  typedef struct {
    S16      offset;
    S16      exp_value;
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

  ret = vc_get_min( VAR_TP1, (U8*)&Min, 0 );
  CU_ASSERT_EQUAL16( ret, kErrNone );

  ret = vc_get_max( VAR_TP1, (U8*)&Max, 1 );
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
    ret = vc_as_int16( VAR_TP1, VarWrite, &Mn, 0, REQ_PRG );
    CU_ASSERT_EQUAL16( ret, t->err );

    ret = vc_as_int16( VAR_TP1, VarRead, &TP1, 0, REQ_PRG );
    CU_ASSERT_EQUAL16( ret, kErrNone );
    CU_ASSERT_EQUAL16( TP1, t->exp_value );
  }
  
  for( size_t i = 0; i < countof(test_max); i++) {
    test_spec_t *t = &test_max[i];
    Mx = Max + t->offset;
    // printf("%d: %2d   %d    %d/%x\n", i, t->offset, t->exp_value, t->err, t->err );
    ret = vc_as_int16( VAR_TP1, VarWrite, &Mx, 1, REQ_PRG );
    CU_ASSERT_EQUAL16( ret, t->err );

    ret = vc_as_int16( VAR_TP1, VarRead, &TP1, 1, REQ_PRG );
    CU_ASSERT_EQUAL16( ret, kErrNone );
    CU_ASSERT_EQUAL16( TP1, t->exp_value );
  }
}

static CU_TestInfo tests_rdwr16[] = {
  { "RD INT16", rd16 },
  { "WR INT16", wr16 },
  { "RD INT16 chan x", rd16_chan },
  { "WR INT16 chan x", wr16_chan },
  { "as string INT16", as_str16 },
  { "WR INT16 min/max", wr16_min_max },
  { "WR INT16 clip", wr16_clip },
	CU_TEST_INFO_NULL,
};

/*** Suite definition  ******************************************************/

static CU_SuiteInfo suites[] = {
  { "variable S16",  suite_init, suite_clean, NULL, NULL, tests_rdwr16 },
	CU_SUITE_INFO_NULL,
};

void test_add_s16(void)
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
