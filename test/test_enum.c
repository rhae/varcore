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


#ifndef countof
# define countof(x) (sizeof(x)/sizeof(x[0]))
#endif

/* Suite initialization/cleanup functions */
static int suite_init(void) {
  vc_init(&g_var_data);
  return 0;
}

static int suite_clean(void) {
  return 0; 
}


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

static void wr_enum_invalid(void) {
  S16 YNU;
  ErrCode ret;

  YNU = -3;
  ret = vc_as_int16( VAR_YNU, VarWrite, &YNU, 5, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrInvalidEnum );
  YNU = 99;
  ret = vc_as_int16( VAR_YNU, VarRead, &YNU, 5, REQ_PRG );
  CU_ASSERT_EQUAL( ret, kErrNone );
  CU_ASSERT_EQUAL( YNU, -1 );
}

static CU_TestInfo tests_rdwr_enum[] = {
  { "RD ENUM", rd_enum },
  { "WR ENUM", wr_enum },
  { "WR ENUM (invalid value)", wr_enum_invalid },
	CU_TEST_INFO_NULL,
};

/*** Suite definition  ******************************************************/

static CU_SuiteInfo suites[] = {
  { "variable enum", suite_init, suite_clean, NULL, NULL, tests_rdwr_enum },
	CU_SUITE_INFO_NULL,
};

void test_add_enum(void)
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
