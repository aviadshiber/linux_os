#include "ReceptionHourStudentBeforeTA.h"
using namespace std;

ReceptionHourStudentBeforeTA::ReceptionHourStudentBeforeTA(
        unsigned int classSize,unsigned int barrierSize)
	: ReceptionHour(classSize),count(0) {
        pthread_barrier_init(&barrier, NULL, barrierSize);
        sem_init(&sem,0,1);

}

ReceptionHourStudentBeforeTA::~ReceptionHourStudentBeforeTA() {
    pthread_barrier_destroy(&barrier);
    sem_destroy(&sem);
}

// a more accurate name would be wakeUpTeacher()
StudentStatus ReceptionHourStudentBeforeTA::waitForTeacher() {
	StudentStatus status = ReceptionHour::waitForTeacher();
    pthread_barrier_wait(&barrier);
    sem_post(&sem);
	return status;
}

bool ReceptionHourStudentBeforeTA::waitForStudent(){
    
    if(count==0){
        sem_wait(&sem);
        count++;
    }
    return ReceptionHour::waitForStudent();
}