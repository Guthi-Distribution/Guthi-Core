#pragma once
#include <cstddef>
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


typedef uint8_t MESSAGE_TYPE;
typedef uint8_t EVENT_TYPE;

// various types of message 
#define MESSAGE_EVENT '0'
#define MESSAGE_FILESYSTEM '1'

struct SharedMemoryInformation {
    MESSAGE_TYPE message_type; 
    char *message = nullptr;
    EVENT_TYPE event_type;
};

static SharedMemoryInformation parse_shared_memory(char *ptr, int count = 0) {
    if (count < 1) {
        return {};
    }
    SharedMemoryInformation info;
    if (ptr[1] == MESSAGE_EVENT) {
        
    } else if (ptr[1] == MESSAGE_FILESYSTEM) {
        
    }
    info.message_type = ptr[1];
    info.message = ptr + 2;


    return info;
}

void *read_shm(void* gfs) {
    while (true) {
        int count = info.shm.shm_segment->count;
        char *data = info.shm.shm_segment->buff;
        if (data[0] == '1') {
            SharedMemoryInformation info = parse_shared_memory(data, count);
            if (info.message_type == MESSAGE_FILESYSTEM) {
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
}
