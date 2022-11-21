// #include <format> --<> Wait till its support by gcc, clang has already
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include <iostream>
#include <pthread.h>
#include <thread>
#include <vector>


#include "./event/event.h"
#include "./filesystem/fs.hpp"
#include "./filesystem/network_fs.hpp"
#include "./runtime/sys_info.hpp"
#include "runtime/sys_info.h"
#include "shared_memory/shm.h"
#include "shared_memory/semaphore.hpp"
#include "core/read_sharedmemory.hpp"

static FileSystem::NetworkFS GFS("./tmp");

int main(int argc, char *argv[])
{
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
    char* data = (char *)malloc(val.size()); // this data will be sent to shared memory
    safe_memcpy(data, val.size(), val.data(), val.size());
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
    SharedMemory shm;
    Semaphore sem;
    info.shm = shm;
    info.sem = sem;
    shm.write_data(data, val.size(), 0);
    pthread_t thread;
    pthread_create(&thread, NULL, read_shm, NULL);
    pthread_join(thread, NULL);
    // getchar();
    return 0;
}
