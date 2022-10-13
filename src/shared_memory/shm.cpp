#include "shm.h"

#ifdef _MSC_VER
void SharedMemory::read_data() {
    printf("%s", shm_segment->buff);
}

void SharedMemory::write_data(char* data, int position) {
    shm_segment->count = strlen(data);
    strcpy(shm_segment->buff + position, data);
}

#endif // _MSC_VER
