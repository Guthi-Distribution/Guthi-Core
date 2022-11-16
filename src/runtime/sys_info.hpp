#pragma once

#ifdef _WIN32
#include <windows.h>
#include <powerbase.h>
#include <powrprof.h>
#include <Pdh.h>
#pragma comment(lib, "Powrprof.lib")
#pragma comment(lib, "Pdh.lib")

#elif defined(__linux__)
#include <sys/stat.h>
#include <sys/types.h> 
#include <sys/sysinfo.h>
#endif

#include <stdint.h>

namespace Runtime
{

#include "sys_info.h"

} // namespace Runtime
