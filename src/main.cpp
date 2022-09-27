// #include <format> --<> Wait till its support by gcc, clang has already

#include <cstdint>
#include <cstdio>
#include <iostream>
#include <thread>

#include "./event/event.h"
#include "./filesystem/fs.hpp"
#include "./runtime/sys_info.hpp"

int main(int argc, char *argv[])
{
    printf("Hello from Guthi : A framework for distributed application development\n");

    Runtime::MemoryStatus status = Runtime::GetSysMemoryInfo();
    Runtime::LogMemoryStatus(status);

    uint32_t i = 0;
    while (i++ < 100)
    {
        Runtime::ProcessorStatus processor = Runtime::GetSysProcessorInfo();
        Runtime::LogProcessorStatus(processor);
        std::cout << "Current CPU Usage : " << Runtime::GetCurrentAllCPUUsage() << '\%' << std::endl;
        std::this_thread::sleep_for(chrono::milliseconds(500));
    }

    return 0;
}
