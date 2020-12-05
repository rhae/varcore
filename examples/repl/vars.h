
#pragma once

#include <varcore.h>

void vars_init();

#define vars_as_string( hnd, rdwr, val, chan, req ) vc_as_string( hnd, rdwr, val, chan, req )
