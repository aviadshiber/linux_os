#include "ReceptionHour.h"

ReceptionHour::ReceptionHour(unsigned int max_waiting_students) {
}

ReceptionHour::~ReceptionHour() {
}

void ReceptionHour::startStudent(unsigned int id) {
}

StudentStatus ReceptionHour::finishStudent(unsigned int id) {
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

