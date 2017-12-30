#include "ReceptionHourWithBarrierForStudents.h"

#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <cassert>
using namespace std;

int main() {
	const unsigned int max_waiting_students = 2;
	ReceptionHourWithBarrierForStudents rh(max_waiting_students);

    for (unsigned int i = 0; i < max_waiting_students + 1; ++i) {
        rh.startStudent(i);
    }

    unsigned int entered_count = 0;
    unsigned int left_because_no_seat_count = 0;
    for (unsigned int i = 0; i < max_waiting_students + 1; ++i) {
        StudentStatus status = rh.finishStudent(i);
	    if (status == ENTERED) {
            entered_count++;
        } else if (status == LEFT_BECAUSE_NO_SEAT) {
            left_because_no_seat_count++;
        }
    }
    assert(entered_count == max_waiting_students);
	assert(left_because_no_seat_count == 1);

	rh.closeTheDoor();
    assert(rh.getDoorState()==true);
	return 0;
}

