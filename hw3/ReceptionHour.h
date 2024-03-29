#ifndef RECEPTIONHOUR_H_
#define RECEPTIONHOUR_H_

#include <unordered_map>
#include <pthread.h>
#include <stdexcept>

// typedef pair<ReceptionHour*,int> studentArg;

using namespace std;
enum StudentStatus {
	ENTERED = 0,
	LEFT_BECAUSE_NO_SEAT,
	LEFT_BECAUSE_DOOR_CLOSED
};

// struct Student{
// 	int id;
// 	StudentStatus status;
// 	pthread_t thread;
// 	Student(int _id){
// 		id=_id;
// 	}
// };


class LocalMutex{
	public:
	LocalMutex(pthread_mutex_t &lock);
	~LocalMutex();
	private:
	pthread_mutex_t local_mutex;
};

class ReceptionHour {
public:
	ReceptionHour(unsigned int max_waiting_students);
	virtual ~ReceptionHour();

virtual	void startStudent(unsigned int id);
virtual	StudentStatus finishStudent(unsigned int id);
	
virtual	void closeTheDoor();

virtual	bool waitForStudent();
virtual	void waitForQuestion();
virtual	void giveAnswer();

virtual	StudentStatus waitForTeacher();
virtual	void askQuestion();
virtual	void waitForAnswer();
bool getDoorState();
protected:
	static void* taFunction(void* obj);
	static void* studentFunction(void* obj);


	bool isDoorClosed;
	bool isQuestionAsked;
	bool isQuestionAnswered;
	pthread_mutex_t numOfStudentLock;
	pthread_mutex_t DoorLock;
	pthread_mutex_t mapLock;
	const unsigned int maxStudents;
	unsigned int numOfStudents;
	unordered_map<int,pthread_t> idToThread;
	pthread_cond_t studentArrived;
	pthread_mutex_t studentArriveLock;
	pthread_cond_t questionAsked;
	pthread_mutex_t questionAskedLock;
	pthread_cond_t taAnswered;
	pthread_mutex_t taAnsweredLock;

	pthread_mutex_t tryToEnterClassLock;

	pthread_t taThread;
	pthread_mutexattr_t mutex_attr;


	pthread_mutex_t taAvailableForQuesiton;
private:
static StudentStatus collectStudentStatus(pthread_t studentThread);
static void* allocateStudentStatus(StudentStatus status);
pthread_t findStudent(unsigned int id);
void removeStudent(unsigned int id);
void IncNumOfStudents();
void DecNumofStudents();
bool needToWaitForStudents();
bool canFinishReceptionHour();
bool DoorClosed();
bool isClassFull();
bool isClassEmpty();
void destoryMap();



	// Remember: you can only use mutexes and condition variables!
};

#endif // RECEPTIONHOUR_H_

