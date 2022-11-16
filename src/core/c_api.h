#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "../runtime/sys_info.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef void* FileSystemRef;

void          *GetLocalFileMetadata(uint32_t *size);
bool           IsFileCached(const char *str, uint32_t length, uint8_t ip_addr[4]);
unsigned char *GetCachedFile(const char *file_name, uint32_t name_length, uint8_t ip_addr[4],
                                uint32_t *file_size_in_bytes, uint64_t /*preferably NTP timestamp*/ timestamp);

void           PrettyPrintFileSystem();
void           ReleaseLocalFileMetadata(void *meta_data);

FileSystemRef InitFileSystem();
MemoryStatus    GetSysMemoryInfo();
ProcessorStatus GetSysProcessorInfo();

void            _LogMemoryStatus(MemoryStatus memory_status);
void            _LogProcessorStatus(ProcessorStatus processor_status);
double          GetCurrentAllCPUUsage();

#ifdef __cplusplus
}
#endif