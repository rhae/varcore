# Note that headers are optional, and do not affect add_library, but they will not
# show up in IDEs unless they are listed in add_library.

include_directories(${varcore_SOURCE_DIR}/lib)

set(EXTERNAL_LIB_HEADERS ${PROJECT_SOURCE_DIR}/thirdparty/cunit/CUnit)

FILE(GLOB test_SOURCES *.c)


# link_directories(${PROJECT_BINARY_DIR}/thirdparty/cunit/CUnit)

add_executable(test ${test_SOURCES} )

target_link_libraries(test cunit varcore)

target_include_directories(
  test PRIVATE
  ${EXTERNAL_LIB_HEADERS}
)

# IDEs should put the headers in a nice place
source_group(
  TREE "${CMAKE_CURRENT_SOURCE_DIR}"
  FILES ${test_SOURCES})