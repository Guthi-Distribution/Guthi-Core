#pragma once
#include "core/error.hpp"

// forward declaration of semaphore structure
struct Semaphore;

// define the semaphore 
#define N_SEMS 1 // total number of semaphore
#define SEM_NUM 0 // current sempahore number

enum SemaphoreResult {
    SUCESS = 0,
    TIMEOUT = -1,
    SEM_ERROR
};

#ifdef _WIN32
#include <Windows.h>

struct Semaphore {
    HANDLE semHnd;

    Semaphore() {
        semHnd = CreateSemaphore(NULL, 1, 1, "guthi_semaphore");
        if (semHnd == NULL) {
            print_error("Semaphore creation error, error code");
            return;
        }
    }

    /*
        Lock while entering the critical section
        Args:
            block_call(bool): flag to indicate whether the wait should be blocking or not
                - false means that the call does not block and returns immediately
                - true:  call waits for the semaphore to unlock
        Return:
            (int): if return value is 0, then the process can proceed to use critical region, else cannot
    */
    int lock(bool block_call = true) {
        DWORD timeout_time = block_call ? INFINITE : 0;
        DWORD wait_result;
        wait_result = WaitForSingleObject(semHnd, timeout_time);

        //TODO(Tilak): Do we need to check for WAIT_ABANDONED?
        if (wait_result == WAIT_OBJECT_0) {
            return 0;
        }
        else if (wait_result == WAIT_TIMEOUT) {
            return -1;
        }
        else if (wait_result == WAIT_FAILED) {
            print_error("Semaphore waiting failed");
        }

        return -1;
    }

    int unlock() {
        bool success = ReleaseSemaphore(semHnd, 1, NULL);
        if (success) {
            return 0;
        }
        print_error("Semaphore Release error");
        return -1;

    }

    ~Semaphore() {
        CloseHandle(semHnd);
    }
};

#else
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <errno.h>
#include <error.h>
#include <stdio.h>

#ifndef KEY
#define KEY 69
#endif

struct Semaphore {
    int id;

    Semaphore() {
        id = semget(KEY, N_SEMS, IPC_CREAT | (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP));
        if (id < 0) {
            perror("Semaphore creation error");
            return ;
        }

        int err = semctl(id, 0, SETVAL, 1); // set the value to 1, i.e unlocked
        if (err < 0) {
            perror("Semaphore control error while setting the value");
        }
    }

    /*
        Lock while entering the critical section
        Args:
            block_call(bool): flag to indicate whether the wait should be blocking or not 
                - false means that the call does not block and returns immediately
                - true:  call waits for the semaphore to unlock
        Return:
            (int): if return value is 0, then the process can proceed to use critical region, else cannot
    */
    int lock(bool block_call  = true) {
        sembuf buffer;
        buffer.sem_num = SEM_NUM;
        buffer.sem_op = -1;
        buffer.sem_flg = (block_call? 0: IPC_NOWAIT) | SEM_UNDO;
        int err = semop(id, &buffer, 1);
        if (err < 0) {
            if (errno == EINVAL && !block_call) {
                return 0;
            }
            perror("Semaphore operation error");
            return -1;
        }
    }

    int unlock() {
        sembuf buffer;
        buffer.sem_num = SEM_NUM;
        buffer.sem_op = 1;
        int err = semop(id, &buffer, 1);
        if (err < 0) {
            perror("Semaphore unlock operation error");
            return -1;
        }

        return 0;
    }

    ~ Semaphore() {
        semctl(id, SEM_NUM, IPC_RMID);
    }
};

#endif