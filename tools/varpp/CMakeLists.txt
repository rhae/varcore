# Note that headers are optional, and do not affect add_library, but they will not
# show up in IDEs unless they are listed in add_library.

include_directories(${varcore_SOURCE_DIR}/lib)

SET(varpp_SOURCES 
		defs.c
		loc.c
		log.c
		strpool.c
		utils.c
		varpp.c
	)
if(WIN32)
    LIST( APPEND varpp_SOURCES compat.c )
endif()

# Make an automatic library - will be static or dynamic based on user setting
add_executable(varpp ${varpp_SOURCES} )

if(WIN32)
    add_definitions(-Drestrict)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_CRT_DEPRECATED_NO_WARNINGS)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
    add_definitions(-D_REENTRANT -D_POSIX_THREAD_SAFE_FUNCTIONS)
    add_definitions(-DWIN32_LEAN_AND_MEAN -D_WIN32_WINNT=0x0501)
endif()

# IDEs should put the headers in a nice place
source_group(
  TREE "${CMAKE_CURRENT_SOURCE_DIR}"
  FILES ${varpp_SOURCES})
