#include "shm.h"
#include "filesystem/fs.hpp"

void SharedMemory::read_data() {
    printf("%s", shm_segment->buff);
}

void SharedMemory::write_data(const char* data, int size, int position) {
    if (size < 1) {
        shm_segment->count = strlen(data + 1);
    } else {
        shm_segment->count = size;
    }
    shm_segment->buff[0] = '0';
    safe_memcpy(shm_segment->buff + position + 1, shm_segment->count, data, shm_segment->count);
}

