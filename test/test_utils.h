
#pragma once

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

