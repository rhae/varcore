# Note that headers are optional, and do not affect add_library, but they will not
# show up in IDEs unless they are listed in add_library.

include_directories(${varcore_SOURCE_DIR}/lib)

FILE(GLOB varpp_SOURCES *.c)

# Make an automatic library - will be static or dynamic based on user setting
add_executable(varpp ${varpp_SOURCES} )

# IDEs should put the headers in a nice place
source_group(
  TREE "${CMAKE_CURRENT_SOURCE_DIR}"
  FILES ${varpp_SOURCES})