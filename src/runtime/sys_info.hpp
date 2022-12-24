#pragma once

#ifdef _MSC_VER
#include <windows.h>
#include <powerbase.h>
#include <powrprof.h>
#include <Pdh.h>
#pragma comment(lib, "Powrprof.lib")
#pragma comment(lib, "Pdh.lib")

#elif _WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x601
#endif
#include <Windows.h>
#include "./Pdh.h"

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
