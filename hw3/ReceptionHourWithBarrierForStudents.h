#ifndef RECEPTIONHOURWITHBARRIERFORSTUDENTS_H_
#define RECEPTIONHOURWITHBARRIERFORSTUDENTS_H_

#include "ReceptionHour.h"

class ReceptionHourWithBarrierForStudents: public ReceptionHour {
public:
    ReceptionHourWithBarrierForStudents(unsigned int max_waiting_students,int inc);
	~ReceptionHourWithBarrierForStudents();

	StudentStatus waitForTeacher() override;

protected:
    pthread_barrier_t barrier;
};

#endif // RECEPTIONHOURWITHBARRIERFORSTUDENTS_H_

