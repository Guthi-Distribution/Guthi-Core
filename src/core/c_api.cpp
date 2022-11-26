#include "../filesystem/fs.hpp"
#include "../filesystem/network_fs.hpp"
#include "c_api.h"
#include "shared_memory/semaphore.hpp"
#include <iostream>


#ifdef __cplusplus
extern "C" {
#endif
static FileSystem::NetworkFS GFS("./tmp");
void *GetLocalFileMetadata(void *size) {
    auto ser = GFS.SerializeLocalFS();
    auto ptr = malloc(sizeof(uint8_t) * ser.size());
    uint32_t* size_ptr = (uint32_t *)size;
    *size_ptr    = (uint32_t)ser.size();
    safe_memcpy(ptr, *size, ser.data(), ser.size());
    return ptr;
}

void ReleaseLocalFileMetadata(void *meta_data)
{
    // No safety checks on this side
    free(meta_data);
}

// Queries if the file from this ip_addr is cached currently
bool IsFileCached(const char *str, uint32_t length,
                  uint8_t ip_addr[4] /* uint32_t ip_addr*/) // guess packed  in little endian format
{
    std::string filename = std::string(str, length);
    return GFS.local_cache.IsFileCached(filename.c_str(), ip_addr);
}

unsigned char *GetCachedFile(const char *file_name, uint32_t name_length, uint8_t ip_addr[4],
                             uint32_t *file_size_in_bytes, uint64_t /*preferably NTP timestamp*/ timestamp)
{
    // TODO :: Include timestamp comparisons
    std::string filename = std::string(file_name, name_length);
    auto        file     = GFS.local_cache.GetCachedFile(filename.c_str(), ip_addr); // timestamp excluded for now
    *file_size_in_bytes  = file.file_size;

    return file.data;
}

// Just this metadata is not enough
void AddToFileCache(const char *file_name, uint32_t name_length, uint8_t ip_addr[4], const char *data,
                    uint32_t data_length)
{
    FileSystem::RawFile rawfile;
    rawfile.data = new uint8_t[data_length];
    safe_memcpy(rawfile.data, data_length, data, data_length);
    GFS.local_cache.AddFileToCache(rawfile, ip_addr);
}

void PrettyPrintFileSystem()
{
    std::cout << "Local FS : \n" << GFS.local_fs << std::endl;
    std::cout << "Root FS : \n" << GFS.GFS_root << std::endl;
}

FileSystemRef InitFileSystem() {
    return (FileSystemRef)&GFS;
}

MemoryStatus    _GetSysMemoryInfo() {
    return GetSysMemoryInfo();
}
ProcessorStatus _GetSysProcessorInfo() {
    return GetSysProcessorInfo();
}

void            _LogMemoryStatus(MemoryStatus memory_status) {
    LogMemoryStatus(memory_status);
}

void            _LogProcessorStatus(ProcessorStatus processor_status) {
    LogProcessorStatus(processor_status);

}

double          _GetCurrentAllCPUUsage() {
    return GetCurrentAllCPUUsage();
}

#ifdef __cplusplus
}
#endif