#include <cstdio>
#include <string_view>
#include <errno.h>
#include <stdlib.h>
#include <inttypes.h>
#include <cstring>

#ifndef SIZE_SHARED_MEM
    #define SIZE_SHARED_MEM 4096
#else
    static_assert SIZE_SHARED_MEM < 65536, "Size of set shared mem greater than 65536 is not allowed";
#endif

    

#ifdef _MSC_VER
#include <Windows.h>
    struct ShmSegment {
        uint16_t size = SIZE_SHARED_MEM;
        uint16_t count;
        char buff[SIZE_SHARED_MEM];
    };

    struct SharedMemory {
    private:
        HANDLE hnd;

    public:
        ShmSegment* shm_segment;

        SharedMemory() {
            hnd = CreateFileMapping(
                INVALID_HANDLE_VALUE,
                NULL,
                PAGE_READWRITE,
                0,
                sizeof(ShmSegment),
                "Guthi_Shared_memory"
            );
        }

        void read_data();
        void write_data();
    };

#endif

#define SHM_KEY 69



#if defined(__gnu_linux__) || defined(__linux__) || defined(linux) || defined(__linux)
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>



struct SharedMemory {
private:
    int id; // file descriptor
    ShmSegment *shm_segment; // memory which will be mapped to shared memory

public:
    SharedMemory() {
        id = shmget(SHM_KEY, sizeof(ShmSegment), IPC_CREAT | (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP));
        if (id == -1) {
            perror("Shared Memory Creation error");
            exit(-1);
        }
        shm_segment = (ShmSegment *)shmat(id, NULL, 0);   
        if (shm_segment == NULL) {
            perror("Memory attach error");
        }
    }

    void write_data(const char *data, int position = 0) {
        shm_segment->count = strlen(data);
        strcpy(shm_segment->buff + position, data);
    }

    void read_data() {
        printf("%s\n", shm_segment->buff);
    }

    ~ SharedMemory() {
        shmdt((const void *)shm_segment);
        shmctl(id, IPC_RMID, NULL);
    }
};
#endif