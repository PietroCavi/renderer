
#include "parallel_unit.h"

// In the whole file there are no checks for calloc, sem_init e pthread_create 

typedef struct{
    sem_t* gate;
    sem_t* done;
    void*(*function)(void*);
    void* arg;
}internal_thread_args;


static void sem_wait_retry(sem_t *s) {
    while (sem_wait(s) == -1 && errno == EINTR)
        continue;
}

static void* parallel_unit_wrapper(void *arg) {
    
    internal_thread_args* prov = (internal_thread_args*) arg;

    sem_t* gate = prov->gate;
    sem_t* done = prov->done;
    void*(*function)(void*) = prov->function;
    void** actual_arg = prov->arg;

    while (1) {
        
        sem_post(done);
        sem_wait_retry(gate);
        
        function(actual_arg[0]);
    
    }
    return NULL;
}
    
static void wait_all_idle(sem_t* done, size_t num_threads) {
    for (size_t i = 0; i < num_threads; i++)
        sem_wait_retry(done);
}

static void unlock_all(sem_t* gate, size_t num_threads){
    for (size_t i = 0; i < num_threads; i++)
        sem_post(&gate[i]);
}

parallel_unit create_parallel_unit(size_t num_threads, void*(*function)(void*)){
    
    sem_t* gate;
    sem_t* done;
    
    pthread_t* threads;
    
    void** args;

    gate    = calloc(num_threads, sizeof(sem_t)     );
    done    = calloc(1,           sizeof(sem_t)      );
    threads = calloc(num_threads, sizeof(pthread_t) );
    args    = calloc(num_threads, sizeof(void*)     );
   
    internal_thread_args setup_args[num_threads];

    sem_init(done, 0, 0);
    for (size_t i = 0; i < num_threads; i++) {
        sem_init(&gate[i], 0, 0);
        
        setup_args[i] = (internal_thread_args){&gate[i],done,function,args+i};

        pthread_create(&threads[i], NULL, parallel_unit_wrapper, &setup_args[i]);
    }
    
    wait_all_idle(done,num_threads);

    return (parallel_unit){
                           .gate = gate,
                           .done = done,
                           .threads = threads,
                           .args = args,
                           .num_threads = num_threads
                          };

}

void execute_parallel_unit(parallel_unit pu){
    unlock_all(pu.gate,pu.num_threads);
    wait_all_idle(pu.done, pu.num_threads);
}

void destroy_parallel_unit(parallel_unit pu){
    
    for (size_t i = 0; i < pu.num_threads; i++)
        pthread_cancel(pu.threads[i]);
    
    for (size_t i = 0; i < pu.num_threads; i++) {
        pthread_join(pu.threads[i], NULL);
        sem_destroy(&pu.gate[i]);
    }
    sem_destroy(pu.done);
    
    free(pu.gate);
    free(pu.done);
    free(pu.threads);
    free(pu.args);
}
