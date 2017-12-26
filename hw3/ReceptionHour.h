#ifndef RECEPTIONHOUR_H_
#define RECEPTIONHOUR_H_

#include <unordered_map>
#include <pthread.h>

enum StudentStatus {
	ENTERED = 0,
	LEFT_BECAUSE_NO_SEAT,
	LEFT_BECAUSE_DOOR_CLOSED
};

class ReceptionHour {
public:
	ReceptionHour(unsigned int max_waiting_students);
	~ReceptionHour();

	void startStudent(unsigned int id);
	StudentStatus finishStudent(unsigned int id);
	
	void closeTheDoor();

	bool waitForStudent();
	void waitForQuestion();
	void giveAnswer();

	StudentStatus waitForTeacher();
	void askQuestion();
	void waitForAnswer();

protected:
	int class_size;
	int current_students_num;
	pthread_cond_t ta_gave_answer;
	mutex_t lock;
	bool isDoorOpen;

	//some data structure to hold the students data
	//array for keeping their status?
	
	// TODO: define the member variables
	// Remember: you can only use mutexes and condition variables!
};

#endif // RECEPTIONHOUR_H_

