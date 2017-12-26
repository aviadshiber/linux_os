#include "Barrier.h"
#include <iostream>
using namespace std;
Barrier::Barrier(unsigned int num_of_threads) {
    thread_limit = num_of_threads;
    sem_init(&sem1, 0, 0);
    sem_init(&sem2, 0, thread_limit);
    sem_init(&sem_mutex,0,1);
    finished_threads=0;
}

void Barrier::wait() {
    sem_wait(&sem2); //sem to filter only N threads
   
   
    int sem2_value;
    sem_getvalue(&sem2, &sem2_value); //get value is atomic
    //only the last thread will free them all
    if(sem2_value == 0){ 
        for(int i=0;i<thread_limit;i++){
            sem_post(&sem1); //here we open the lower gate
        }
    }
  

    sem_wait(&sem1);     //n-1 first prcoesses are waiting here for the last process to arrive

    //---------to make the barrer reusable again we need to make sem2=N again ----------//
    sem_wait(&sem_mutex); //locking the mutex since we access finished_threads as shared memory
    finished_threads++;
    if(finished_threads==thread_limit){
        for(int i=0;i<thread_limit;i++){
            sem_post(&sem2); //here we open the lower gate
        }
        finished_threads=0;
    }
    sem_post(&sem_mutex); //unlock the mutex
}

Barrier::~Barrier() {
    sem_destroy(&sem1);
    sem_destroy(&sem2);
    sem_destroy(&sem_mutex);
}

