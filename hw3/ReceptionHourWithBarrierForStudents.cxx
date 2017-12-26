#include "ReceptionHourWithBarrierForStudents.h"
using namespace std;

ReceptionHourWithBarrierForStudents::ReceptionHourWithBarrierForStudents(
        unsigned int max_waiting_students)
	: ReceptionHour(max_waiting_students) {
        pthread_barrier_init(&barrier, NULL, max_waiting_students+1);
}

ReceptionHourWithBarrierForStudents::~ReceptionHourWithBarrierForStudents() {
    pthread_barrier_destroy(&barrier);
}

// a more accurate name would be wakeUpTeacher()
StudentStatus ReceptionHourWithBarrierForStudents::waitForTeacher() {
	StudentStatus status = ReceptionHour::waitForTeacher();
    pthread_barrier_wait(&barrier);
	return status;
}

