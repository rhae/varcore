

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


  ret = vc_as_int16( VAR_TP1, VarWrite, &temp[VEC_LEM-1], VEC_LEM-1, REQ_PRG );
  CU_ASSERT_EQUAL16( ret, kErrNone );
  temp[VEC_LEM-1] = 99;
  ret = vc_as_int16( VAR_TP1, VarRead, &temp[VEC_LEM-1], VEC_LEM-1, REQ_PRG );
  CU_ASSERT_EQUAL16( ret, kErrNone );
  CU_ASSERT_EQUAL16( temp[VEC_LEM-1], -99 );

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
  { "RD CONST STRIGN", rd_const_str },
  { "WR CONST STRING", wr_const_str },
	CU_TEST_INFO_NULL,
};


static CU_SuiteInfo suites[] = {
  { "variable S32",  suite_init, suite_clean, NULL, NULL, tests_rdwr32 },
  { "variable S16",  suite_init, suite_clean, NULL, NULL, tests_rdwr16 },
  { "variable str",  suite_init, suite_clean, NULL, NULL, tests_rdwr_str },
  { "variable const str",  suite_init, suite_clean, NULL, NULL, tests_rd_const_str },
  #if 0
  { "suite_success_init",  suite_success_init, NULL,                NULL, NULL, tests_success},
  { "suite_success_clean", NULL,               suite_success_clean, NULL, NULL, tests_success},
  { "test_failure",        NULL,               NULL,                NULL, NULL, tests_failure},
  { "suite_failure_both",  suite_failure_init, suite_failure_clean, NULL, NULL, tests_suitefailure}, /* tests should not run */
  { "suite_failure_init",  suite_failure_init, NULL,                NULL, NULL, tests_suitefailure}, /* tests should not run */
  { "suite_success_but_failure_clean", NULL,   suite_failure_clean, NULL, NULL, tests_suitefailure}, /* tests will run, suite counted as running, but suite tagged as a failure */
  { "TestSimpleAssert",    NULL,               NULL,                NULL, NULL, tests_simple},
  { "TestBooleanAssert",   NULL,               NULL,                NULL, NULL, tests_bool},
  { "TestEqualityAssert",  NULL,               NULL,                NULL, NULL, tests_equal},
  { "TestPointerAssert",   NULL,               NULL,                NULL, NULL, tests_ptr},
  { "TestNullnessAssert",  NULL,               NULL,                NULL, NULL, tests_null},
  { "TestStringAssert",    NULL,               NULL,                NULL, NULL, tests_string},
  { "TestNStringAssert",   NULL,               NULL,                NULL, NULL, tests_nstring},
  { "TestDoubleAssert",    NULL,               NULL,                NULL, NULL, tests_double},
  { "TestFatal",           NULL,               NULL,                NULL, NULL, tests_fatal},
  #endif
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

/* implementation without shortcut registration
  CU_pSuite pSuite;

  pSuite = CU_add_suite("suite_success_both", suite_success_init, suite_success_clean);
  CU_add_test(pSuite, "testSuccess1", testSuccess1);
  CU_add_test(pSuite, "testSuccess2", testSuccess2);
  CU_add_test(pSuite, "testSuccess3", testSuccess3);

  pSuite = CU_add_suite("suite_success_init", suite_success_init, NULL);
  CU_add_test(pSuite, "testSuccess1", testSuccess1);
  CU_add_test(pSuite, "testSuccess2", testSuccess2);
  CU_add_test(pSuite, "testSuccess3", testSuccess3);

  pSuite = CU_add_suite("suite_success_clean", NULL, suite_success_clean);
  CU_add_test(pSuite, "testSuccess1", testSuccess1);
  CU_add_test(pSuite, "testSuccess2", testSuccess2);
  CU_add_test(pSuite, "testSuccess3", testSuccess3);

  pSuite = CU_add_suite("test_failure", NULL, NULL);
  CU_add_test(pSuite, "testFailure1", testFailure1);
  CU_add_test(pSuite, "testFailure2", testFailure2);
  CU_add_test(pSuite, "testFailure3", testFailure3);

  / * tests should not run * /
  pSuite = CU_add_suite("suite_failure_both", suite_failure_init, suite_failure_clean);
  CU_add_test(pSuite, "testSuiteFailure1", testSuiteFailure1);
  CU_add_test(pSuite, "testSuiteFailure2", testSuiteFailure2);

  / * tests should not run * /
  pSuite = CU_add_suite("suite_failure_init", suite_failure_init, NULL);
  CU_add_test(pSuite, "testSuiteFailure1", testSuiteFailure1);
  CU_add_test(pSuite, "testSuiteFailure2", testSuiteFailure2);

  / * tests will run, suite counted as running, but suite tagged as a failure * /
  pSuite = CU_add_suite("suite_success_but_failure_clean", NULL, suite_failure_clean);
  CU_add_test(pSuite, "testSuiteFailure1", testSuiteFailure1);
  CU_add_test(pSuite, "testSuiteFailure2", testSuiteFailure2);

  pSuite = CU_add_suite("TestSimpleAssert", NULL, NULL);
  CU_add_test(pSuite, "testSimpleAssert", testSimpleAssert);
  CU_add_test(pSuite, "testFail", testFail);

  pSuite = CU_add_suite("TestBooleanAssert", NULL, NULL);
  CU_add_test(pSuite, "testAssertTrue", testAssertTrue);
  CU_add_test(pSuite, "testAssertFalse", testAssertFalse);

  pSuite = CU_add_suite("TestEqualityAssert", NULL, NULL);
  CU_add_test(pSuite, "testAssertEqual", testAssertEqual);
  CU_add_test(pSuite, "testAssertNotEqual", testAssertNotEqual);

  pSuite = CU_add_suite("TestPointerAssert", NULL, NULL);
  CU_add_test(pSuite, "testAssertPtrEqual", testAssertPtrEqual);
  CU_add_test(pSuite, "testAssertPtrNotEqual", testAssertPtrNotEqual);

  pSuite = CU_add_suite("TestNullnessAssert", NULL, NULL);
  CU_add_test(pSuite, "testAssertPtrNull", testAssertPtrNull);
  CU_add_test(pSuite, "testAssertPtrNotNull", testAssertPtrNotNull);

  pSuite = CU_add_suite("TestStringAssert", NULL, NULL);
  CU_add_test(pSuite, "testAssertStringEqual", testAssertStringEqual);
  CU_add_test(pSuite, "testAssertStringNotEqual", testAssertStringNotEqual);

  pSuite = CU_add_suite("TestNStringAssert", NULL, NULL);
  CU_add_test(pSuite, "testAssertNStringEqual", testAssertNStringEqual);
  CU_add_test(pSuite, "testAssertNStringNotEqual", testAssertNStringNotEqual);

  pSuite = CU_add_suite("TestDoubleAssert", NULL, NULL);
  CU_add_test(pSuite, "testAssertDoubleEqual", testAssertDoubleEqual);
  CU_add_test(pSuite, "testAssertDoubleNotEqual", testAssertDoubleNotEqual);

  pSuite = CU_add_suite("TestFatal", NULL, NULL);
  CU_add_test(pSuite, "testFatal", testFatal);
*/
}
