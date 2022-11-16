/*
    This file is created so that structures can be exported easily to C api
*/
#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
typedef struct MemoryStatus {
    uint64_t installed_physical_ram;
    uint64_t available_ram;
    uint64_t memory_load;
    // Information about virtual memory is not required here
}MemoryStatus;

typedef struct ProcessorInfo {
    uint32_t processor_number;
    uint32_t current_mhz;
    uint32_t total_mhz;
}ProcessorInfo;

typedef struct ProcessorStatus {
    uint32_t      processor_count;
    ProcessorInfo processors[64];
} ProcessorStatus;


MemoryStatus    GetSysMemoryInfo();
ProcessorStatus GetSysProcessorInfo();

void            LogMemoryStatus(MemoryStatus memory_status);
void            LogProcessorStatus(ProcessorStatus processor_status);
double          GetCurrentAllCPUUsage();


#ifdef __cplusplus
}
#endif