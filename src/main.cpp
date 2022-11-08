// #include <format> --<> Wait till its support by gcc, clang has already

#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include <iostream>
#include <thread>
#include <vector>

#include <shared_memory/shm.h>

#if 0
int main() {
    SharedMemory mem;
    mem.read_data();
    getchar();
}
#endif

#include "./event/event.h"
#include "./filesystem/fs.hpp"
#include "./filesystem/network_fs.hpp"
#include "./runtime/sys_info.hpp"
#include "shared_memory/semaphore.hpp"

static FileSystem::NetworkFS GFS("./tmp");

// C API Implementation

void *GetLocalFileMetadata(uint32_t *size)
{
    auto ser = GFS.SerializeLocalFS();
    auto ptr = malloc(sizeof(uint8_t) * ser.size());
    *size    = (uint32_t)ser.size();
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

//  C API End

int main(int argc, char *argv[])
{
    Runtime::GetSysProcessorInfo();
    printf("Hello from Guthi : A framework for distributed application development\n");

    Runtime::MemoryStatus status = Runtime::GetSysMemoryInfo();
    Runtime::LogMemoryStatus(status);

    uint32_t i              = 0;

    float    moving_average = 0.0f;

    while (i++ < 50)
    {
        if ((i + 1) % 10 == 0)
        {
            std::cout << "Current CPU Usage : " << moving_average << "%" << std::endl;
            moving_average = 0.0f;
        }
        // Runtime::ProcessorStatus processor = Runtime::GetSysProcessorInfo();
        // Runtime::LogProcessorStatus(processor);
        else
        {
            auto n         = (i + 1) % 10;
            moving_average = (n-1) * moving_average / n + (Runtime::GetCurrentAllCPUUsage() / n);
        }

        std::this_thread::sleep_for(chrono::milliseconds(100));
    }

    using namespace FileSystem;

    auto content  = std::shared_ptr<FileContent>(new FileContent);
    content->name = "CGraphics";
    content->type = FileType::Directory;

    auto file     = std::shared_ptr<FileContent>(new FileContent);
    file->name    = "raytraced.png";
    file->type    = FileType::File;

    auto file2    = std::shared_ptr<FileContent>(new FileContent);
    file2->name   = "rasterised.png";
    file2->type   = FileType::File;

    content->contents.push_back(file);
    content->contents.push_back(file2);

    std::cout << '\n' << *content.get() << std::endl;

    GFS.local_fs    = *content.get(); // [unsafe], just for test

    auto        val = GFS.SerializeLocalFS();
    FileContent deserialized;
    assert(decltype(GFS)::DeserializeToFileContent(val, deserialized));
    std::cout << "Content after deserialization : \n\n" << deserialized << std::endl;
    std::cout << deserialized.name << std::endl;

    // FS Cache Test
    uint8_t myfile[] = "Hello this is my file.";
    RawFile rfile;
    rfile.file_name = "CacheTest.exe";
    rfile.file_size = sizeof(myfile);
    rfile.data      = new uint8_t[sizeof(myfile)];
    safe_memcpy(rfile.data, sizeof(myfile), myfile, sizeof(myfile));

    uint8_t ip[] = {127, 0, 0, 1};

    GFS.local_cache.AddFileToCache(rfile, ip);
    assert(GFS.local_cache.RemoveFromCache("CacheTest.exe", ip));
    assert(!GFS.local_cache.RemoveFromCache("CacheTest.exe", ip));
    return 0;
}
