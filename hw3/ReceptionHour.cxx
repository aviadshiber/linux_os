#include "ReceptionHour.h"
#include "stdio.h"

LocalMutex::LocalMutex(pthread_mutex_t& lock): local_mutex(lock) {
	pthread_mutex_lock(&local_mutex);
}

LocalMutex::~LocalMutex() {
	pthread_mutex_unlock(&local_mutex);
}

bool ReceptionHour::getDoorState(){
	return DoorClosed();
}

ReceptionHour::ReceptionHour(unsigned int max_waiting_students):
maxStudents(max_waiting_students),numOfStudents(0),isDoorClosed(false),isQuestionAsked(false),isQuestionAnswered(false) {
	
	pthread_mutexattr_init(&mutex_attr);
	pthread_mutexattr_settype(&mutex_attr,PTHREAD_MUTEX_ERRORCHECK);

	pthread_mutex_init(&studentArriveLock,&mutex_attr);
	pthread_cond_init(&studentArrived,NULL);
	pthread_mutex_init(&questionAskedLock,&mutex_attr);
	pthread_cond_init(&questionAsked,NULL);
	pthread_mutex_init(&taAnsweredLock,&mutex_attr);
	pthread_cond_init(&taAnswered,NULL);


	pthread_mutex_init(&numOfStudentLock,&mutex_attr);
	pthread_mutex_init(&DoorLock,&mutex_attr);
	pthread_mutex_init(&mapLock,&mutex_attr);
	pthread_mutex_init(&tryToEnterClassLock,&mutex_attr);

	pthread_mutex_init(&taAvailableForQuesiton,&mutex_attr);

	
	pthread_create(&taThread,NULL,taFunction,this);
	//printf("TA thread created %d\n",(int)taThread);
}

void* ReceptionHour::taFunction(void* obj){
	ReceptionHour* reception=(ReceptionHour *)obj;
	while(reception->waitForStudent()){
		reception->waitForQuestion();
		reception->giveAnswer();
	}
	//printf("taFunction will now exit\n");
	return NULL;
}


void ReceptionHour::startStudent(unsigned int id) {
	pthread_t thread;
	pthread_create(&thread,NULL,studentFunction,this);
	//printf("student %d thread was created\n");
	{
		LocalMutex LocalMutex(mapLock);
		//printf("student %d thread was added to map with thread id %d\n",(int)id,(int)thread);
		idToThread.insert({id,thread}); //here we copy the student into the map
	}
}


void* ReceptionHour::studentFunction(void* obj){
	
	ReceptionHour* reception=(ReceptionHour*)obj;
	StudentStatus status= reception->waitForTeacher();
	if(StudentStatus::ENTERED != status){		//student didn't enter
		return allocateStudentStatus(status);
	}
	reception->askQuestion();
	reception->waitForAnswer();

	return allocateStudentStatus(StudentStatus::ENTERED);
}
/**
 * the method wait for the student to return status, and collects the student status from his thread.
 * */
StudentStatus ReceptionHour::collectStudentStatus(pthread_t studentThread){
	void* status_vp;
	//printf("waiting for %d student Thread to finish\n",(int)studentThread);
	pthread_join(studentThread,&status_vp);
	StudentStatus* studentStatusPointer=static_cast<StudentStatus*>(status_vp);
	StudentStatus studentStatus=*(studentStatusPointer);
	delete studentStatusPointer;
	return studentStatus;
}


void* ReceptionHour::allocateStudentStatus(StudentStatus status){
	StudentStatus* statusPointer=new StudentStatus;
	*statusPointer=status;
	//printf("status was allocated with %d\n",status);
	return static_cast<void*>(statusPointer);
}

pthread_t ReceptionHour::findStudent(unsigned int id){
	pthread_t studentThread;
	{
		LocalMutex localMutex(mapLock);
		//printf("Finsihed student method was called with id=[%d] . trying to collect his status (thread %d)\n",id,(int)pthread_self());
		std::unordered_map<int,pthread_t>::const_iterator findResult = idToThread.find(id);
		if ( findResult == idToThread.end() ){
				//fprintf(stderr,"failed to find student id=%d\n",(int)id);
				throw std::runtime_error("could not find student!");
		}
		else{
			studentThread=findResult->second;
		}
	}
	return studentThread;
}

void ReceptionHour::removeStudent(unsigned int id){
	LocalMutex localMutex(mapLock);
	idToThread.erase(id);
}

StudentStatus ReceptionHour::finishStudent(unsigned int id) {
	pthread_t studentThread=findStudent(id);

	StudentStatus status = collectStudentStatus(studentThread);
	//printf("Student %d that was collected with %d status (thread %d)\n",(int)id,status,(int)pthread_self());
	removeStudent(id);

	return status;
}

bool ReceptionHour::isClassFull(){
	bool result;
	{
		LocalMutex localMutex(numOfStudentLock);
		result=numOfStudents>=maxStudents;
	}
	return result;
}

void ReceptionHour::IncNumOfStudents(){
	LocalMutex localMutex(numOfStudentLock);
	numOfStudents++;
	//printf("Student entered the room, and got a seat (thread %d)\n",(int)pthread_self());
}
/**
 * try to enter the room
 * */
StudentStatus ReceptionHour::waitForTeacher() {
	StudentStatus status = StudentStatus::ENTERED;
	{
		LocalMutex enterMutex(tryToEnterClassLock);
		if(DoorClosed()){//if the door is closed student leaves
			status = StudentStatus::LEFT_BECAUSE_DOOR_CLOSED;
			//printf("Student can't enter door is closed (thread %d)\n",(int)pthread_self());
			return status;					
		}else if(isClassFull()){//if the room is full student leaves
			status=StudentStatus::LEFT_BECAUSE_NO_SEAT;
			//printf("Student can't enter room is full (thread %d)\n",(int)pthread_self());
			return status; 					
		}
		{// we now try to enter by sending signal to waitForStudent
			LocalMutex localMutex(studentArriveLock);
			IncNumOfStudents();
			//printf("Student trying to signal is arrival,now num of student is %d (thread %d)\n",numOfStudents,(int)pthread_self());
			pthread_cond_signal(&studentArrived);
		}
	}
	return status; 
}

bool ReceptionHour::DoorClosed(){
	bool result;
	{
		LocalMutex localMutex(DoorLock);
		result=isDoorClosed;
		// if(isDoorClosed){
		// 	//printf("door is closed, can't accept students \n");
		// }else{
		// 	//printf("door is opened, can accept students \n");
		// }
	}
	return result;
}

void ReceptionHour::closeTheDoor() {
	LocalMutex localMutex(DoorLock);
	isDoorClosed=true;
	//printf("TA closed the door (thread %d)\n",(int)pthread_self());
}

bool ReceptionHour::isClassEmpty(){
	bool result;
	{ //check if number of student is zero
		LocalMutex localMutex(numOfStudentLock);
		result = (0 == numOfStudents);
		//printf("num of students:%d\n",numOfStudents);
		//if(result)
			//printf("TA has no students in the room (need to go to sleep).\n");
	}
	return result;
}

bool ReceptionHour::needToWaitForStudents(){

	bool result=isClassEmpty();
	if(DoorClosed() && result){
		//printf("door is closed and class is empty, no need to wait for students\n");
		return false;
	}
	return result;
}
/**
 * The TA can finish his reception hour if there are no students, and the door is closed.
 * */
bool ReceptionHour::canFinishReceptionHour(){
	return DoorClosed() && isClassEmpty();
}

/**
 wait for student to arrive.
  if the door is closed or the room is full return false. otherwise true.
*/
bool ReceptionHour::waitForStudent() {

	//ta should conditionaly wait until a student arrive
	LocalMutex localMutex(studentArriveLock);
	//printf("TA is tying to be blocked until a student arrive or door is closed (thread %d)\n",(int)pthread_self());
	while(needToWaitForStudents()){
		pthread_cond_wait(&studentArrived,&studentArriveLock);
	}
	if(canFinishReceptionHour()){
		return false;
	}
	
	return true; 
}

void ReceptionHour::waitForQuestion() {

	LocalMutex localMutex(questionAskedLock);
	//printf("TA is tying to be blocked untill he gets a question (thread %d)\n",(int)pthread_self());
	while(!isQuestionAsked){
		
		pthread_cond_wait(&questionAsked,&questionAskedLock);
	}
	//printf("TA IS NO LONGER BLOCKED, he got a question (thread %d)\n",(int)pthread_self());
	isQuestionAsked=false;
}

void ReceptionHour::askQuestion() {
	// we lock here mutex untill ta give answer
	pthread_mutex_lock(&taAvailableForQuesiton);
	//printf("Student is locking the taAvailableForQuesiton(thread %d)\n",(int)pthread_self());
	
	{ //signal that question been asked
		LocalMutex localMutex(questionAskedLock);
		isQuestionAsked=true;
		//printf("Student is trying to signal questionAsked(thread %d)\n",(int)pthread_self());
		pthread_cond_signal(&questionAsked);
	}
	
}

void ReceptionHour::DecNumofStudents(){
	LocalMutex localMutex(numOfStudentLock);
	--numOfStudents;
	//printf("Student exited the room (he got an answer :). now num of students is:[%d] ) (thread %d)\n",numOfStudents,(int)pthread_self());
}

void ReceptionHour::giveAnswer() {
	LocalMutex localMutex(taAnsweredLock);
	//printf("TA giving answer(thread %d)\n",(int)pthread_self());
	isQuestionAnswered=true;
	DecNumofStudents();
	//printf("Ta trying to signal taAnswered (thread %d)\n",(int)pthread_self());
	pthread_cond_signal(&taAnswered);
}

void ReceptionHour::waitForAnswer() {
	//conidion wait for TA to answer
	LocalMutex localMutex(taAnsweredLock);
	//printf("Student is tring to be blocked until he gets an answer from Ta(thread %d)\n",pthread_self());
	while(!isQuestionAnswered){
		pthread_cond_wait(&taAnswered,&taAnsweredLock);
	}
	isQuestionAnswered=false;
	//printf("Student IS NO LONGER BLOCKED, he got his answer from TA. now other student can come along[unlocking taAvailableForQuesiton] (thread %d)\n",pthread_self());
	//we unlock because the ta gave the answer
	pthread_mutex_unlock(&taAvailableForQuesiton);
	
}
void ReceptionHour::destoryMap(){
	std::unordered_map<int, pthread_t>::iterator it = idToThread.begin();
	while(it != idToThread.end()){
		finishStudent(it->first);
		it = idToThread.begin();
	}
}
ReceptionHour::~ReceptionHour() {
	//we finish students that the user forgot to finish
	destoryMap();
	closeTheDoor();
	//wait for ta to finish before we destory the world
	pthread_join(taThread,NULL);
	int fail=0;
	fail=pthread_cond_destroy(&studentArrived);
	if(fail){
		//fprintf(stderr,"failed to desotroy studentArrived\n");
	}
	fail= pthread_mutex_destroy(&studentArriveLock);
	if(fail){
		//fprintf(stderr,"failed to desotroy studentArriveLock\n");
	}
	fail=pthread_cond_destroy(&questionAsked);
	if(fail){
		//fprintf(stderr,"failed to desotroy questionAskedLock\n");
	}
	fail=pthread_mutex_destroy(&questionAskedLock);
	if(fail){
		//fprintf(stderr,"failed to desotroy questionAskedLock\n");
	}
	fail=pthread_cond_destroy(&taAnswered);
	if(fail){
		//fprintf(stderr,"failed to desotroy taAnswered\n");
	}
	fail=pthread_mutex_destroy(&taAnsweredLock);	
	if(fail){
		//fprintf(stderr,"failed to desotroy taAnsweredLock\n");
	}

	fail=pthread_mutex_destroy(&taAvailableForQuesiton);
	if(fail){
		//fprintf(stderr,"failed to desotroy taAvailableForQuesiton\n");
	}
	fail=pthread_mutex_destroy(&mapLock);
	if(fail){
		//fprintf(stderr,"failed to desotroy mapLock\n");
	}
	fail=pthread_mutex_destroy(&DoorLock);
	if(fail){
		//fprintf(stderr,"failed to desotroy DoorLock\n");
	}
	fail=pthread_mutex_destroy(&numOfStudentLock);
	if(fail){
		//fprintf(stderr,"failed to desotroy numOfStudentLock\n");
	}
	fail=pthread_mutex_destroy(&tryToEnterClassLock);
	if(fail){
		//fprintf(stderr,"failed to desotroy tryToEnterClassLock\n");
	}
	fail=pthread_mutexattr_destroy(&mutex_attr);
	if(fail){
		//fprintf(stderr,"failed to desotroy mutex_attr\n");
	}
	
}
