#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "../runtime/sys_info.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef void* FileSystemRef;

void          *GetLocalFileMetadata(void *size);
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

#ifdef _MSC_VER
#include <Windows.h>

char* get_error_message(const char* message) {
    char* error_message_buff;
    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        message,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&error_message_buff,
        0, NULL);
    char* error = (char*)malloc(1024);
    sprintf_s(error, 1024, "%s:%s", message, error_message_buff);
    LocalFree(error_message_buff);
    return error;
}

struct ShmInfo {
    HANDLE hnd;
    char* err; // to make this stuff "goish"
};

ShmInfo OpenSharedMemory(char *name) {
    ShmInfo info;
    info.hnd = OpenFileMapping (
        FILE_MAP_ALL_ACCESS,   // read/write access
        FALSE,                 // do not inherit the name
        name
    );
    info.err = NULL;
    if (info.hnd == NULL) {
        info.err = get_error_message("Opening of shared memory failed");
        return info;
    }

    return info;
}
#endif

#ifdef __cplusplus
}
#endif