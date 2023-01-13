#pragma once
#include <cstdio>
#include <string_view>
#include <errno.h>
#include <stdlib.h>
#include <inttypes.h>
#include <cstring>
#include "./core/error.hpp"

#ifndef SIZE_SHARED_MEM
#define SIZE_SHARED_MEM 4096
#else
static_assert(SIZE_SHARED_MEM < 65536, "Size of set shared mem greater than 65536 is not allowed");
#endif

struct ShmSegment
{
    uint16_t count;
    char     buff[SIZE_SHARED_MEM];
};

#if defined(_MSC_VER) || defined(_WIN32)
#include <Windows.h>

struct SharedMemory
{
  private:
    HANDLE hnd;

  public:
    ShmSegment *shm_segment;

        SharedMemory() {
            hnd = CreateFileMapping(
                INVALID_HANDLE_VALUE,
                NULL,
                PAGE_READWRITE,
                0,
                4100,
                "Guthi_Shared_memory"
            );
            if (hnd == NULL) {
                print_error("File mapping creation error\n");
                return;
            }

            shm_segment = (ShmSegment*)MapViewOfFile(hnd, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(ShmSegment));
            if (shm_segment == NULL) {
                print_error("Map View of File error\n");
                CloseHandle(hnd);
                return; 
            }
        }


    ~SharedMemory()
    {
        UnmapViewOfFile(reinterpret_cast<void *>(shm_segment));
        CloseHandle(hnd);
    }
		
		void write_data(const char* data, int size = 0, int position = 0);

		void read_data();
	};

#else 

#define SHM_KEY 69
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>

    struct SharedMemory
    {
      private:
        int id; // file descriptor

public:
    ShmSegment *shm_segment; // memory which will be mapped to shared memory
    SharedMemory() {
        id = shmget(SHM_KEY, SIZE_SHARED_MEM + sizeof(uint16_t), IPC_CREAT | (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP));
        if (id == -1) {
            perror("Shared Memory Creation error");
            exit(-1);
        }
        shm_segment = (ShmSegment *)shmat(id, NULL, 0);   
        if (shm_segment == NULL) {
            perror("Memory attach error");
        }
    }
	
	void write_data(const char* data, int size = 0, int position = 0);
	void read_data();

    ~SharedMemory() {
        shmdt((const void*)shm_segment);
        shmctl(id, IPC_RMID, NULL);
    }
};

#endif