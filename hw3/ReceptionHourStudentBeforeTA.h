#ifndef RECEPTIONHOURSTUDENTBEFORETA
#define RECEPTIONHOURSTUDENTBEFORETA
#include "ReceptionHour.h"
#include <semaphore.h>
class ReceptionHourStudentBeforeTA: public ReceptionHour {
public:
    ReceptionHourStudentBeforeTA(unsigned int classSize,unsigned int barrierSize);
	~ReceptionHourStudentBeforeTA();

	StudentStatus waitForTeacher() override;
    bool waitForStudent() override;

protected:
    pthread_barrier_t barrier;
    sem_t sem;
    volatile int count;
};

#endif
