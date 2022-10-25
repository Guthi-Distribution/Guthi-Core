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

extern "C" void             *GetLocalFileMetadata(uint32_t *size)
{
    auto ser = GFS.SerializeLocalFS();
    auto ptr = malloc(sizeof(uint8_t) * ser.size());
    *size    = (uint32_t)ser.size();
    safe_memcpy(ptr, *size, ser.data(), ser.size());
    return ptr;
}

int main() {
    Semaphore s;
    s.lock();
    getchar();
    s.unlock();
}


#if 0
int main(int argc, char *argv[])
{   
    Runtime::GetSysProcessorInfo();
    printf("Hello from Guthi : A framework for distributed application development\n");

    Runtime::MemoryStatus status = Runtime::GetSysMemoryInfo();
    Runtime::LogMemoryStatus(status);

    uint32_t i = 0;

    while (i++ < 10)
    {
        Runtime::ProcessorStatus processor = Runtime::GetSysProcessorInfo();
        Runtime::LogProcessorStatus(processor);
        std::cout << "Current CPU Usage : " << Runtime::GetCurrentAllCPUUsage() << "%" << std::endl;
        std::this_thread::sleep_for(chrono::milliseconds(500));
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
    return 0;
}
#endif
