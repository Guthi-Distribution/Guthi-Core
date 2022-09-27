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

#ifndef SIZE_SHARED_MEM
#define SIZE_SHARED_MEM 4096
#endif

#ifdef _MSC_VER
TODO: Windows version
#endif

#if defined(__gnu_linux__) || defined(__linux__) || defined(linux) || defined(__linux)
struct SharedMemory {
    int fd; // file descriptor
    std::string_view name; // name of the shared_memory, ofc
    char *memory; // memory which will be mapped to shared memory
    sem_t sem;
    key_t key;

    SharedMemory(const char* _name, int flag, int mode): name(_name) {
        fd = shm_open(_name, flag | O_RDWR, mode);
        if (fd < 0) {
            if (errno == EACCES) {
                printf("Permission to shm_unlink() the shared memory object was denied.\n");
            } else if (errno == EACCES) {
                printf("Permission was denied to shm_open() name in the specified mode, or O_TRUNC was specified and the caller does not have write permission on the object.\n");
            } else if (errno == EEXIST) {
                printf("Both O_CREAT and O_EXCL were specified to shm_open() and the shared memory object specified by name already exists.\n");
            } else if (errno == EINVAL) {
                printf("The name argument to shm_open() was invalid\n");
            } else if (errno == ENOENT) {
                printf("An attempt was made to shm_open() a name that did not exist, and O_CREAT was not specified\n");
            }
            exit(-1);
        }
        ftruncate(fd, SIZE_SHARED_MEM);
        memory = (char *)mmap(NULL, SIZE_SHARED_MEM, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
    }

    void write_data(const char *data, int position = 0) {
        if (position >= SIZE_SHARED_MEM) {
            //TODO: Error handling
            return; 
        }
        flock(fd, LOCK_EX);
        sprintf(memory + position, data);
        flock(fd, LOCK_UN);
    }

    const char* read_data(int position = 0) {
        if (position >= SIZE_SHARED_MEM) {
            //TODO: Error handling
            return nullptr;
        }
        return (const char *)(memory + position);
    }

    ~ SharedMemory() {
        shm_unlink(name.data());
        munmap(memory, SIZE_SHARED_MEM);
    }
};
#endif