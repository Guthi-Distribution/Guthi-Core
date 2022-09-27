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

#include <cstdint>

namespace Runtime
{
struct MemoryStatus
{
    uint64_t installed_physical_ram;
    uint64_t available_ram;
    uint64_t memory_load;
    // Information about virtual memory is not required here
};

struct ProcessorInfo
{
    uint32_t processor_number;
    uint32_t current_mhz;
    uint32_t total_mhz;
};

struct ProcessorStatus
{
    uint32_t      processor_count;
    ProcessorInfo processors[64] = {};
};

MemoryStatus    GetSysMemoryInfo();
ProcessorStatus GetSysProcessorInfo();

void            LogMemoryStatus(MemoryStatus &memory_status);
void            LogProcessorStatus(ProcessorStatus &processor_status);
double          GetCurrentAllCPUUsage();

} // namespace Runtime
