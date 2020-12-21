# Note that headers are optional, and do not affect add_library, but they will not
# show up in IDEs unless they are listed in add_library.

include_directories(${varcore_SOURCE_DIR}/lib)

FILE(GLOB repl_SOURCES *.c)
list(APPEND repl_SOURCES vardefs.h)

add_executable(repl ${repl_SOURCES} )
if(WIN32)
    add_definitions(-Drestrict)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_CRT_DEPRECATED_NO_WARNINGS)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
    add_definitions(-D_REENTRANT -D_POSIX_THREAD_SAFE_FUNCTIONS)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
endif()

add_dependencies(repl varpp)

target_link_libraries(repl varcore)


add_custom_command(
  OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/vardefs.h"
  COMMAND varpp "${CMAKE_CURRENT_SOURCE_DIR}/res.csv"

  # If the file exists, then commands related to that file won't be executed
  # DONOT let other target depends on the same OUTPUT as current target,
  #   or it may be bad when doing parallel make
  DEPENDS varpp "${CMAKE_CURRENT_SOURCE_DIR}/res.csv"

  COMMENT "Generate variable definition"

  # to make quotes printable,for example
  VERBATIM
)

target_link_libraries(repl varcore)

# IDEs should put the headers in a nice place
source_group(
  TREE "${CMAKE_CURRENT_SOURCE_DIR}"
  FILES ${repl_SOURCES})
