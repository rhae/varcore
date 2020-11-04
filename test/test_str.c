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

static void set_min_max() {
  U16 T = 0;
  ErrCode ret;
  HND hnd = VAR_IDN;

  ret = vc_get_min( hnd, 0, 0 );
  CU_ASSERT_EQUAL16( ret, kErrInvalidArg );

  ret = vc_get_max( hnd, 0, 1 );
  CU_ASSERT_EQUAL16( ret, kErrInvalidArg );

  ret = vc_set_min( hnd, 0, 0 );
  CU_ASSERT_EQUAL16( ret, kErrInvalidArg );

  ret = vc_set_max( hnd, 0, 0 );
  CU_ASSERT_EQUAL16( ret, kErrInvalidArg );

  ret = vc_get_min( hnd, (U8*)&T, 0 );
  CU_ASSERT_EQUAL16( ret, kErrInvalidType );

  ret = vc_get_max( hnd, (U8*)&T, 0 );
  CU_ASSERT_EQUAL16( ret, kErrInvalidType );

  ret = vc_set_min( hnd, (U8*)&T, 0 );
  CU_ASSERT_EQUAL16( ret, kErrInvalidType );

  ret = vc_set_max( hnd, (U8*)&T, 0 );
  CU_ASSERT_EQUAL16( ret, kErrInvalidType );
}

static CU_TestInfo tests_rdwr_str[] = {
  { "STR, RD", rd_str },
  { "STR, WR", wr_str },
  #if 0
  { "RD chan x", rd16_chan },
  { "WR chan x", wr16_chan },
  { "as string", as_str16 },
  #endif
  { "STR, SET MIN/MAX", set_min_max },
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


/*** Suite definition  ******************************************************/

static CU_SuiteInfo suites[] = {
  { "variable str",  suite_init, suite_clean, NULL, NULL, tests_rdwr_str },
  { "variable const str",  suite_init, suite_clean, NULL, NULL, tests_rd_const_str },
	CU_SUITE_INFO_NULL,
};

void test_add_string(void)
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
