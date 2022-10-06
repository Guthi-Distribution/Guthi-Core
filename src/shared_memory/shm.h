#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <cstdio>
#include <string_view>
#include <errno.h>
#include <stdlib.h>
#include <semaphore.h>
#include <inttypes.h>

#ifndef SIZE_SHARED_MEM
    #define SIZE_SHARED_MEM 4096
#else
    static_assert SIZE_SHARED_MEM < 65536, "Size of set shared mem greater than 65536 is not allowed";
#endif

#ifdef _MSC_VER
TODO: Windows version
#endif

#define SHM_KEY 69

struct ShmSegment {
    uint16_t count;
    char buff[SIZE_SHARED_MEM];
};

#if defined(__gnu_linux__) || defined(__linux__) || defined(linux) || defined(__linux)
struct SharedMemory {
    int id; // file descriptor
    ShmSegment *memory; // memory which will be mapped to shared memory
    sem_t sem;
    key_t key;

    SharedMemory(const char* _name, int flag, int mode) {
        id = shmget(SHM_KEY, sizeof(ShmSegment), IPC_CREAT | (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP));
        if (id == -1) {
            perror("Shared Memory Creation error");
            if (errno == EACCES) printf("A shared  memory identifier exists for key but operation permission as specified by the low-order nine bits of shmflg would not be granted.\n");
            if (errno == EEXIST) printf("A shared memory identifier exists for the argument key but (shmflg &IPC_CREAT) &&(shmflg &IPC_EXCL) is non-zero.\n");
            if (errno == EINVAL) printf("A shared memory segment is to be created and the value of size is less than the system-imposed minimum or greater than the system-imposed maximum.\n");
            if (errno == EINVAL) printf("No shared memory segment is to be created and a shared memory segment exists for key but the size of the segment associated with it is less than size.\n");
            if (errno == ENOENT) printf("A shared memory identifier does not exist for the argument key and (shmflg &IPC_CREAT) is 0.\n");
            if (errno == ENOMEM) printf("A shared memory identifier and associated shared memory segment are to be created, but the amount of available physical memory is not sufficient to fill the request.\n");
            if (errno == ENOSPC) printf("A shared memory identifier is to be created, but the system-imposed limit on the maximum number of allowed shared memory identifiers system-wide would be exceeded.\n");
            exit(-1);
        }
        memory = (ShmSegment *)shmat(id, NULL, 0);   
        if (memory == NULL) {
            perror("Memory attach error\n");
        }
    }

    void write_data(const char *data, int position = 0) {
        
    }

    const char* read_data(int position = 0) {
    }

    ~ SharedMemory() {
        shmdt((const void *)memory);
        
    }
};
#endif