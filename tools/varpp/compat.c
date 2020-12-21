
#include <windows.h>
#include <stdlib.h>  // _fullpath
#include <time.h>    // _localtime_s

char *realpath(const char *path, char *resolved_path)
{
   return _fullpath( resolved_path, path, MAX_PATH );
}

struct tm *localtime_r( const time_t *timer, struct tm * result )
{
	localtime_s( result, timer );
	return result;
}

