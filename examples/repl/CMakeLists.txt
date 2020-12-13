# Note that headers are optional, and do not affect add_library, but they will not
# show up in IDEs unless they are listed in add_library.

include_directories(${varcore_SOURCE_DIR}/lib)

FILE(GLOB repl_SOURCES *.c)


add_executable(repl ${repl_SOURCES} )

target_link_libraries(repl varcore)

#target_include_directories(
#  test PRIVATE
#  ${EXTERNAL_LIB_HEADERS}
#)

# IDEs should put the headers in a nice place
source_group(
  TREE "${CMAKE_CURRENT_SOURCE_DIR}"
  FILES ${repl_SOURCES})