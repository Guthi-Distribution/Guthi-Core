#pragma once
#include <cstdint>
#include <iostream>
#include <shared_memory/shm.h>
#include <shared_memory/semaphore.hpp>
#include <vector>
#include "filesystem/fs.hpp"
#include "filesystem/network_fs.hpp"

struct ThreadInformation {
    SharedMemory shm;
    Semaphore sem;
};

static ThreadInformation info;
#if defined(__gnu_linux__) || defined(__linux__) || defined(linux) || defined(__linux)
void *read_shm(void* gfs) {
    while (true) {
        int count = info.shm.shm_segment->count;
        char *data = info.shm.shm_segment->buff;
        if (data[0] == '1') {
            std::vector<uint8_t> serialized;
            for (int i = 1; i <= count; i++) {
                serialized.push_back((uint8_t)data[i]);
            }
            FileSystem::FileContent deserialized;
            assert(FileSystem::NetworkFS::DeserializeToFileContent(serialized, deserialized));
            std::cout << deserialized << std::endl;
        }
    }
}

#endif