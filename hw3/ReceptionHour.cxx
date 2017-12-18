#include "ReceptionHour.h"

ReceptionHour::ReceptionHour(unsigned int max_waiting_students) {
	class_size=max_waiting_students;
	current_students_num = 0;
	pthread_cond_init(&ta_available_condition, NULL);
	pthread_mutex_init(&lock, NULL);
	//pthread_create();
	while(1){
	//waitForStudent();
	//waitForQuestion();
	//giveAnswer();
	}
}

ReceptionHour::~ReceptionHour() {
	//pthread_exit();
}

void ReceptionHour::startStudent(unsigned int id) {
	//pthread_create("id");
	// waitForTeacher();
	// askQuestion();
	// waitForAnswer();
}

StudentStatus ReceptionHour::finishStudent(unsigned int id) {
	//pthread_exit("id");
	//return "student_status";
	return StudentStatus::ENTERED; //FIXME
}

void ReceptionHour::closeTheDoor() {
}

bool ReceptionHour::waitForStudent() {
	return false; //FIXME
}

void ReceptionHour::waitForQuestion() {
}

void ReceptionHour::giveAnswer() {
}

StudentStatus ReceptionHour::waitForTeacher() {
	return StudentStatus::ENTERED; //FIXME
}

void ReceptionHour::askQuestion() {
}

void ReceptionHour::waitForAnswer() {
}

