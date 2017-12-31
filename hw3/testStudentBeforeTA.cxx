#include "ReceptionHourStudentBeforeTA.h"

#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <cassert>
using namespace std;

int main() {
    cout<<"choose a class size:"<<endl;
    unsigned int classSize = 2;//default
    cin>>classSize;
	
    cout<<"how many student do you want?:"<<endl;
    unsigned int barrierSize=4;//default
    cin>>barrierSize;

	ReceptionHourStudentBeforeTA rh(classSize,barrierSize);

    for (unsigned int i = 0; i < barrierSize; ++i) {
        rh.startStudent(i);
    }

    unsigned int entered_count = 0;
    unsigned int left_because_no_seat_count = 0;
    for (unsigned int i = 0; i < barrierSize; ++i) {
        StudentStatus status = rh.finishStudent(i);
	    if (status == ENTERED) {
            entered_count++;
        } else if (status == LEFT_BECAUSE_NO_SEAT) {
            left_because_no_seat_count++;
        }
    }
    assert(entered_count == classSize);
	assert(left_because_no_seat_count == (barrierSize-classSize));

	rh.closeTheDoor();
    
	return 0;
}

