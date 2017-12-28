#include "ReceptionHour.h"

ReceptionHour::ReceptionHour(unsigned int max_waiting_students) {
	maxStudents=max_waiting_students;
	numOfStudents=0;
	isDoorClosed=false;
	isQuestionAsked=false;
	isQuestionAnswered=false;
	isStudentFinished=false;
	pthread_mutexattr_init(&mutex_attr);
	pthread_mutexattr_settype(&mutex_attr,PTHREAD_MUTEX_ERRORCHECK);

	// pthread_mutex_init(&studentArriveLock,&mutex_attr);
	// pthread_cond_init(&studentArrived,NULL);
	pthread_mutex_init(&questionAskedLock,&mutex_attr);
	pthread_cond_init(&questionAsked,NULL);
	pthread_mutex_init(&taAnsweredLock,&mutex_attr);
	pthread_cond_init(&taAnswered,NULL);

	// pthread_mutex_init(&studentFinishedLock,&mutex_attr);
	// pthread_cond_init(&studentFinished,NULL);

	pthread_mutex_init(&lock,&mutex_attr);

	pthread_mutex_init(&taAvailableForQuesiton,&mutex_attr);


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
	int fail=0;
	// fail|= pthread_mutex_destroy(&studentArriveLock);
	// fail|=pthread_cond_destroy(&studentArrived);
	fail|=pthread_mutex_destroy(&questionAskedLock);
	fail|=pthread_cond_destroy(&questionAsked);
	fail|=pthread_mutex_destroy(&taAnsweredLock);
	fail|=pthread_cond_destroy(&taAnswered);
	// fail|=pthread_mutex_destroy(&studentFinishedLock);
	// fail|=pthread_cond_destroy(&studentFinished);

	fail|=pthread_mutex_destroy(&lock);
	fail|=pthread_mutex_destroy(&taAvailableForQuesiton);
	
	fail|=pthread_mutexattr_destroy(&mutex_attr);
	if(fail){
		fprintf(stderr,"failed to desotroy reception hour");
	}
}

void ReceptionHour::startStudent(unsigned int id) {
	pthread_t thread;
	pthread_mutex_lock(&lock);
	idToThread.insert({id,thread}); //here we copy the student into the map
	pthread_mutex_unlock(&lock);
	pthread_create(&thread,NULL,studentFunction,this);
	
}
//@Deprecated
void  ReceptionHour::waitForStudentToFinish(){
		// pthread_mutex_lock(&studentFinished);
		// while(!isStudentFinished){
		// 	pthread_cond_wait(&studentFinished);
		// }
		// pthread_mutex_unlock(&studentFinished);
}

void* ReceptionHour::studentFunction(void* obj){
	
	ReceptionHour* reception=(ReceptionHour*)obj;
	StudentStatus status= reception->waitForTeacher();
	if(StudentStatus::ENTERED != status){
		waitForStudentToFinish();
		return new StudentStatus(status);
	}
	pthread_mutex_lock(&lock);
	//pthread_mutex_lock(&reception->studentArriveLock);
	reception->numOfStudents++;
	//pthread_cond_signal(&reception->studentArrived);
	//pthread_mutex_unlock(&reception->studentArriveLock);
	pthread_mutex_unlock(&lock);
	
	
	reception->askQuestion();
	reception->waitForAnswer();
	waitForStudentToFinish();
	return new StudentStatus(StudentStatus::ENTERED);
}

StudentStatus ReceptionHour::finishStudent(unsigned int id) {
	//we should use join here
	pthread_mutex_lock(&lock);
	pthread_t studentThread= idToThread.find(id)->second;
	pthread_mutex_unlock(&lock);
	StudentStatus *status;
	//now we wait for student to finish his thread and we get his status
	pthread_join(studentThread,(void**)&status);
	pthread_mutex_lock(&lock);
	StudentStatus finalStatus=*status;
	if(StudentStatus::ENTERED == finalStatus){
		numOfStudents--;
	}
	delete status;
	pthread_mutex_unlock(&lock);
	
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
	// pthread_mutex_lock(&studentArriveLock);
	// while(0 == numOfStudents && !isDoorClosed){
	// 	pthread_cond_wait(&studentArrived,&studentArriveLock);
	// }
	// pthread_mutex_unlock(&studentArriveLock);
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
	// we lock here mutex untill ta give answer
	pthread_mutex_lock(&taAvailableForQuesiton);
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
	//we unlock because the ta gave the answer
	pthread_mutex_unlock(&taAvailableForQuesiton);
	pthread_mutex_unlock(&taAnsweredLock);
}

