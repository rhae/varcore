# Note that headers are optional, and do not affect add_library, but they will not
# show up in IDEs unless they are listed in add_library.

include_directories(${varcore_SOURCE_DIR}/lib)

set(EXTERNAL_LIB_HEADERS ${PROJECT_SOURCE_DIR}/thirdparty/cunit/CUnit)

FILE(GLOB test_SOURCES *.c)
list(APPEND test_SOURCES vardefs.h)

# link_directories(${PROJECT_BINARY_DIR}/thirdparty/cunit/CUnit)

add_executable(test ${test_SOURCES} )
add_dependencies(test varpp)

target_link_libraries(test cunit varcore)

target_include_directories(
  test PRIVATE
  ${EXTERNAL_LIB_HEADERS}
)

add_custom_command(
  OUTPUT vardefs.h
  COMMAND varpp "${CMAKE_CURRENT_SOURCE_DIR}/res.csv"

  # If the file exists, then commands related to that file won't be executed
  # DONOT let other target depends on the same OUTPUT as current target,
  #   or it may be bad when doing parallel make
  DEPENDS varpp "${CMAKE_CURRENT_SOURCE_DIR}/res.csv"

  COMMENT "Generate variable definition"

  # to make quotes printable,for example
  VERBATIM
)

# IDEs should put the headers in a nice place
source_group(
  TREE "${CMAKE_CURRENT_SOURCE_DIR}"
  FILES ${test_SOURCES})