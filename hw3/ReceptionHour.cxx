#include "ReceptionHour.h"

ReceptionHour::ReceptionHour(unsigned int max_waiting_students) {
	maxStudents=max_waiting_students;
	numOfStudents=0;
	isDoorClosed=false;
	isQuestionAsked=false;
	isQuestionAnswered=false;
	pthread_mutexattr_init(&mutex_attr);
	pthread_mutexattr_settype(&mutex_attr,PTHREAD_MUTEX_ERRORCHECK);

	pthread_mutex_init(&studentArriveLock,&mutex_attr);
	pthread_cond_init(&studentArrived,NULL);
	pthread_mutex_init(&questionAskedLock,&mutex_attr);
	pthread_cond_init(&questionAsked,NULL);
	pthread_mutex_init(&taAnsweredLock,&mutex_attr);
	pthread_cond_init(&taAnswered,NULL);
	pthread_mutex_init(&lock,&mutex_attr);

	pthread_create(&taThread,NULL,taFunction,this);
}

void* ReceptionHour::taFunction(void* obj){
	ReceptionHour* ta=(ReceptionHour *)obj;
	while(true){
		if(!ta->waitForStudent()){
			break;
		}
		ta->waitForQuestion();
		ta->giveAnswer();
	}

	return NULL;
}


ReceptionHour::~ReceptionHour() {
	pthread_mutex_destroy(&studentArriveLock);
	pthread_cond_destroy(&studentArrived);
	pthread_mutex_destroy(&questionAskedLock);
	pthread_cond_destroy(&questionAsked);
	pthread_mutex_destroy(&taAnsweredLock);
	pthread_cond_destroy(&taAnswered);
	pthread_mutex_destroy(&lock);
	
	pthread_mutexattr_destroy(&mutex_attr);

}

void ReceptionHour::startStudent(unsigned int id) {
	pthread_t thread;
	pthread_mutex_lock(&lock);
	idToThread.insert({id,thread}); //here we copy the student into the map
	pthread_mutex_unlock(&lock);
	pthread_create(&thread,NULL,studentFunction,this);
	
}

void* ReceptionHour::studentFunction(void* obj){
	
	ReceptionHour* reception=(ReceptionHour*)obj;
	StudentStatus status= reception->waitForTeacher();
	if(StudentStatus::ENTERED != status){
		return new StudentStatus(status);
	}
	pthread_mutex_lock(&lock);
	pthread_mutex_lock(&reception->studentArriveLock);
	reception->numOfStudents++;
	pthread_cond_signal(&reception->studentArrived);
	pthread_mutex_unlock(&lock);
	pthread_mutex_unlock(&reception->studentArriveLock);
	
	reception->askQuestion();
	reception->waitForAnswer();
	return new StudentStatus(StudentStatus::ENTERED);
}

StudentStatus ReceptionHour::finishStudent(unsigned int id) {
	//we should use join here
	pthread_mutex_lock(&lock);
	pthread_t studentThread= idToThread.find(id)->second;
	pthread_mutex_unlock(&lock);
	StudentStatus *status;
	pthread_join(studentThread,(void**)&status);
	
	StudentStatus finalStatus=*status;
	pthread_mutex_lock(&lock);
	if(StudentStatus::ENTERED == finalStatus){
		numOfStudents--;
	}
	pthread_mutex_unlock(&lock);
	delete status;
	return finalStatus;
}

void ReceptionHour::closeTheDoor() {
	pthread_mutex_lock(&lock);
	isDoorClosed=true;
	pthread_mutex_unlock(&lock);
}
/**
 wait for student to arrive.
  if the door is closed or the room is full return false. otherwise true.
*/
bool ReceptionHour::waitForStudent() {
	
	pthread_mutex_lock(&lock);
	if(isDoorClosed && numOfStudents==0){
		pthread_mutex_unlock(&lock);
		return false;
	}
	pthread_mutex_unlock(&lock);
	pthread_mutex_lock(&studentArriveLock);
	while(0 == numOfStudents && !isDoorClosed){
		pthread_cond_wait(&studentArrived,&studentArriveLock);
	}
	pthread_mutex_unlock(&studentArriveLock);
	return true; 
}

void ReceptionHour::waitForQuestion() {
	pthread_mutex_lock(&questionAskedLock);
	while(!isQuestionAsked){
		pthread_cond_wait(&questionAsked,&questionAskedLock);
	}
	isQuestionAsked=false;
	pthread_mutex_unlock(&questionAskedLock);

}

void ReceptionHour::giveAnswer() {
	pthread_mutex_lock(&taAnsweredLock);
	isQuestionAnswered=true;
	pthread_cond_signal(&taAnswered);
	pthread_mutex_unlock(&taAnsweredLock);
}
/**
 * try to enter the room
 * */
StudentStatus ReceptionHour::waitForTeacher() {
	pthread_mutex_lock(&lock);
	StudentStatus status = StudentStatus::ENTERED;
	if(isDoorClosed){
		status = StudentStatus::LEFT_BECAUSE_DOOR_CLOSED;
	}else if(numOfStudents>=maxStudents){
		status=StudentStatus::LEFT_BECAUSE_NO_SEAT;
	}
	pthread_mutex_unlock(&lock);
	return status; 
}

void ReceptionHour::askQuestion() {
	pthread_mutex_lock(&questionAskedLock);
	isQuestionAsked=true;
	pthread_cond_signal(&questionAsked);
	pthread_mutex_unlock(&questionAskedLock);
}

void ReceptionHour::waitForAnswer() {
	pthread_mutex_lock(&taAnsweredLock);
	while(!isQuestionAnswered){
		pthread_cond_wait(&taAnswered,&taAnsweredLock);
	}
	isQuestionAnswered=false;
	pthread_mutex_unlock(&taAnsweredLock);
}

