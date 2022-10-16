// Implemented for win32 only for now

#include "./sys_info.hpp"
#include <iostream>
#include <memory>
#include "./parser_cpuinfo.hpp"

#define ByteToKB(byte_) ((byte_) / 1024)
#define ByteToMB(byte_) (ByteToKB(byte_) / 1024)

#define NT_PROCESSOR_INFO ProcessorInformation

// Bruh, windows

#ifdef _WIN32
#define STATUS_ACCESS_DENIED 0xC0000022
#define STATUS_BUFFER_TOO_SMALL 0xC0000023

typedef struct _PROCESSOR_POWER_INFORMATION
{
    ULONG Number;
    ULONG MaxMhz;
    ULONG CurrentMhz;
    ULONG MhzLimit;
    ULONG MaxIdleState;
    ULONG CurrentIdleState;
} PROCESSOR_POWER_INFORMATION, *PPROCESSOR_POWER_INFORMATION;

struct ProcessorMetadata
{
    HANDLE         handle;
    ULARGE_INTEGER last_cpu, last_sys_cpu, last_user_cpu;
};

struct PDHData
{
    PDH_HQUERY   cpu_query;
    PDH_HCOUNTER cpu_total;
};

#endif

namespace Runtime
{

void LogMemoryStatus(MemoryStatus &memory_status)
{
    std::cout << "Available ram : " << memory_status.available_ram
              << "\nTotal ram : " << memory_status.installed_physical_ram
              << "\nMemory load : " << memory_status.memory_load << std::endl;
}

void LogProcessorStatus(ProcessorStatus &processor_status)
{
    for (uint32_t pid = 0; pid < processor_status.processor_count; ++pid)
    {
        std::cout << "pid : " << processor_status.processors[pid].processor_number
                  << "\nCurrent MHz : " << processor_status.processors[pid].current_mhz
                  << "\nTotal MHz : " << processor_status.processors[pid].total_mhz << std::endl;
    }
}

MemoryStatus GetSysMemoryInfo()
{
    MemoryStatus status = {0};
#ifdef _WIN32
    MEMORYSTATUSEX mem_status = {.dwLength = sizeof(mem_status)};
    GlobalMemoryStatusEx(&mem_status);
    status.installed_physical_ram = ByteToMB(mem_status.ullTotalPhys);
    status.available_ram          = ByteToMB(mem_status.ullAvailPhys);
    status.memory_load            = mem_status.dwMemoryLoad;
#else
    struct sysinfo sys_info;
    sysinfo(&sys_info);
    status.installed_physical_ram = ByteToMB(sys_info.totalram * sys_info.mem_unit);
    status.available_ram          = ByteToMB(sys_info.freeram * sys_info.mem_unit);
    status.memory_load            = int(float(sys_info.freeram) / float(sys_info.totalram) * 100);
#endif
    return status;
}

ProcessorStatus GetSysProcessorInfo()
{
    ProcessorStatus processor_status = {0};

#ifdef _WIN32
    SYSTEM_INFO sys_info = {0};
    GetSystemInfo(&sys_info);
    processor_status.processor_count = sys_info.dwNumberOfProcessors;

    auto power_info                  = std::unique_ptr<PROCESSOR_POWER_INFORMATION[]>(
        new PROCESSOR_POWER_INFORMATION[processor_status.processor_count]);
    auto result = CallNtPowerInformation(NT_PROCESSOR_INFO, nullptr, 0, power_info.get(),
                                         sizeof(PROCESSOR_POWER_INFORMATION) * processor_status.processor_count);

    if (result)
    {
        fprintf(stderr, "PowerStatusInformation Hazard \nError : %x.", result);
        exit(0);
    }

    for (uint32_t pro = 0; pro < processor_status.processor_count; ++pro)
    {
        processor_status.processors[pro].processor_number = power_info.get()[pro].Number;
        processor_status.processors[pro].total_mhz        = power_info.get()[pro].MaxMhz;
        processor_status.processors[pro].current_mhz      = power_info.get()[pro].CurrentMhz;
    }
#else
// #error "Processor Status info not implemented for other OS except Win32"
    GetSystemInfo(&processor_status);
#endif
    return processor_status;
}

double GetCurrentAllCPUUsage()
{
#ifdef _WIN32
    static auto pdh_meta = []() {
        PDHData data = {};
        PdhOpenQuery(nullptr, NULL, &data.cpu_query);
        PdhAddEnglishCounter(data.cpu_query, "\\Processor(_Total)\\% Processor Time", NULL, &data.cpu_total);
        PdhCollectQueryData(data.cpu_query);
        return data;
    }();

    PDH_FMT_COUNTERVALUE counter_val;
    PdhCollectQueryData(pdh_meta.cpu_query);
    PdhGetFormattedCounterValue(pdh_meta.cpu_total, PDH_FMT_DOUBLE, NULL, &counter_val);
    return counter_val.doubleValue;
#else
// #error "Querying current CPU not supported on other platforms than win32 atm.";
return 0; 
#endif
}

ProcessorStatus GetSysProccessorInfoAlternate()
{
    ProcessorStatus processor_status = {0};

#ifdef _WIN32
    static ProcessorMetadata proc_meta = []() {
        FILETIME          ftime, fsys, fuser;
        ProcessorMetadata data;

        GetSystemTimeAsFileTime(&ftime);
        data.handle = GetCurrentProcess();
        memcpy((void *)&data.last_cpu, (void *)&ftime, sizeof(FILETIME));

        GetProcessTimes(data.handle, &ftime, &ftime, &fsys, &fuser);
        memcpy(&data.last_sys_cpu, &fsys, sizeof(FILETIME));
        memcpy(&data.last_user_cpu, &fuser, sizeof(FILETIME));

        return ProcessorMetadata();
    }();

    // TODO :: Complete it if you want
    SYSTEM_INFO sys_info = {0};
    GetSystemInfo(&sys_info);
    processor_status.processor_count = sys_info.dwNumberOfProcessors;

#endif

    return GetSysProcessorInfo();
}

} // namespace Runtime
