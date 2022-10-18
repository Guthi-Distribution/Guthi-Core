#ifdef _MSC_VER

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

// define the semaphore 
#define N_SEMS 1
#define SEM_NUM 0

enum SemOperation {
    GE
};

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