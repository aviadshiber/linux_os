#include "Barrier.h"

Barrier::Barrier(unsigned int num_of_threads) {
    thread_limit = num_of_threads;
    sem_init(&sem1, 0, 0);
    sem_init(&sem2, 0, thread_limit);
}

void Barrier::wait() {
    sem_wait(&sem2); 
    int value;
    sem_getvalue(&sem2, &value);
    if(value == 0){      //when the n'th process entered
        for(int i=0;i<thread_limit;i++){
            sem_post(&sem1);
        }
    }
    sem_wait(&sem1);     //n-1 first prcoesses are waiting here for the last process to arrive
}

Barrier::~Barrier() {
    sem_destroy(&sem1);
    sem_destroy(&sem2);
}

