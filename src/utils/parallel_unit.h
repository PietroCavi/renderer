#pragma once

#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct{

    sem_t* gate;
    sem_t* done;
    
    pthread_t* threads;

    void** args;

    size_t num_threads;

}parallel_unit;

parallel_unit create_parallel_unit(size_t num_threads, void*(*function)(void*));
void execute_parallel_unit(parallel_unit pu);
void destroy_parallel_unit(parallel_unit pu);
