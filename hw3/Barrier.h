#ifndef BARRIER_H_
#define BARRIER_H_

#include <semaphore.h>

class Barrier {
public:
	Barrier(unsigned int num_of_threads);
	void wait();
	~Barrier();

protected:
	sem_t sem1;
	sem_t sem2;
	sem_t sem_mutex;
	int finished_threads;
	int thread_limit;
};

#endif // BARRIER_H_

