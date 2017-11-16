
#include "hw1_syscalls.h"
#include "staticQueue.hpp"
#include "testsMacros.h"
#include <climits>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sched.h>

// Global variables. Used to perform various system calls.
static const int PID = getpid();
struct sched_param GLOBAL_SCHED_PARAM;
FILE* FILE_PTR = NULL;
int FILE_COUNTER =0;

// Functions that call a single system call
static const int FORK =				2;
static const int FOPEN =			5;
static const int FCLOSE =			6;
static const int GETPID =			20;
static const int GETPPID =			64;
static const int GETPGRP =			65;
static const int SCHED_GETPARAM =	155;
static const int SCHED_YIELD =		158;

// Functions that call multiple system calls
static const int SLEEP =			1000;
	

// This makes sure every syscall_log_info created in test begins it's life with Nullified elements.
class enveloped_syscall_log_info : public syscall_log_info{
	public:
		enveloped_syscall_log_info(){
			syscall_num = -1;
			syscall_res = -1;
			time = -1;
		}
};

// Performs system call and pushes it's num+res
void performSystemCall(int syscallNum, staticQueue<struct syscall_log_info>& queue){
	// Saves syscall info. Pushed into the queue.
	struct syscall_log_info info;

	switch(syscallNum){
		case FORK:
			info.syscall_num = FORK;
			info.syscall_res = fork();
			
			// Parent
			if (info.syscall_res != 0)
				queue.push(info);
			break;
		
		case FOPEN:								// Don't call this from a random test !
			assert(FILE_COUNTER == 0);			
			FILE_COUNTER++;						// Just to avoid bugs in test
			info.syscall_num = FOPEN;
			FILE_PTR = fopen("file.txt", "r");	// 5
			info.syscall_res = (int)FILE_PTR;
			queue.push(info);
			break;
		
		case FCLOSE:							// Don't call this from a random test !
			assert(FILE_COUNTER == 1);
			FILE_COUNTER--;						// Just to avoid bugs in test
			info.syscall_num = FCLOSE;
			info.syscall_res = fclose (FILE_PTR);// 6
			queue.push(info);
			break;	
		
		case GETPID:
			info.syscall_num = GETPID;
			info.syscall_res = getpid();
			queue.push(info);
			break;
			
		case GETPPID:
			info.syscall_num = GETPPID;
			info.syscall_res = getppid();
			queue.push(info);
			break;
			
		case GETPGRP:
			info.syscall_num = GETPGRP;
			info.syscall_res = getpgrp();
			queue.push(info);
			break;
			
		case SCHED_GETPARAM:
			info.syscall_num = SCHED_GETPARAM;
			info.syscall_res = sched_getparam(PID, &GLOBAL_SCHED_PARAM);
			queue.push(info);
			break;
			
		case SCHED_YIELD:
			info.syscall_num = SCHED_YIELD;
			info.syscall_res = sched_yield();
			queue.push(info);
			break;
			
		case SLEEP:
			info.syscall_num = 175;
			info.syscall_res = sleep(1);
			queue.push(info);
			
			info.syscall_num = 174;
			queue.push(info);
			
			info.syscall_num = 175;
			queue.push(info);
			
			info.syscall_num = 162;
			queue.push(info);
			break;
			
		default:
			std::cout << "performSystemCall defaulted." << std::endl;
			assert(0);
	}
}

// Performs system call and pushes it's num+res
void intToSystemCall(unsigned int x, staticQueue<struct syscall_log_info>& queue){
	switch(x%5){
		case 0:
			performSystemCall(GETPID, queue);
			break;
			
		case 1:
			performSystemCall(GETPPID, queue);
			break;
		
		case 2:
			performSystemCall(GETPGRP, queue);
			break;
			
		case 3:
			performSystemCall(SCHED_GETPARAM, queue);
			break;
			
		case 4:	
			performSystemCall(SCHED_YIELD, queue);
			break;
			
		default:
			assert(0);
	}
}

bool stressTest(){
	// Test variables
	int bResult = true;
	struct syscall_log_info info;
	
	// Used for performing wraparound
	const int LOG_SIZE = 50;
	const double arrConstants[] = {1, 1.3, 1.7, 2, 2.4, 3.5, 4.1, 5.9, 6.7, 10, 30, 57 };	// Largest must be on the right
	const int arrConstantsSize = 12;														// Size of arrConstants
	const int iMaxNumOfOps  = LOG_SIZE * (int)arrConstants[arrConstantsSize-1];
	
	int* arrOps = new int[iMaxNumOfOps];
	enveloped_syscall_log_info* pBuff = new enveloped_syscall_log_info[iMaxNumOfOps];
	
	// Iterate over arrConstants
	for (int iConstantIndex=0 ; iConstantIndex<arrConstantsSize ; ++iConstantIndex){
		
		// Get random operations
		int numOfOps = (int)((double)LOG_SIZE * arrConstants[iConstantIndex]);
		for (int i=0 ; i<numOfOps ; ++i)
			arrOps[i] = rand();
		
		// Will allocate memory. Must be performed before enabling logging.
		staticQueue<struct syscall_log_info> queue(LOG_SIZE);
	
		// Push first call and enable logging
		info.syscall_num = 243;
		info.syscall_res = enable_syscalls_logging(PID, LOG_SIZE); // 243
		queue.push(info);
		
		// Perform ops
		for (int i=0 ; i<numOfOps ; ++i)
			intToSystemCall(arrOps[i], queue);
		
		// Read LOG_SIZE calls
		const int iGetRes = get_syscalls_log(PID, LOG_SIZE, pBuff);
		if (iGetRes != 0)
		{
			std::cout << std::endl << "Some DEBUG info that might help you:" << std::endl;
			std::cout << "numOfOps = " << numOfOps << std::endl;		
			std::cout << "iConstantIndex = " << iConstantIndex << std::endl;
			std::cout << "LOG_SIZE = " << LOG_SIZE << std::endl;
			std::cout << "errno = " << errno << std::endl;
		}
		ASSERT_EQ(iGetRes, 0);	// 245
		
		// Compare LOG_SIZE calls
		for (int i=0 ; i<LOG_SIZE ; ++i){
			info = queue.pop();
			
			if (pBuff[i].syscall_num != info.syscall_num)
			{
			std::cout << std::endl << "Some DEBUG info that might help you:" << std::endl;
			std::cout << "numOfOps = " << numOfOps << std::endl;		
			std::cout << "iConstantIndex = " << iConstantIndex << std::endl;
			std::cout << "i = " << i << std::endl;
			}
			
			ASSERT_EQ(pBuff[i].syscall_num, info.syscall_num); 
			ASSERT_EQ(pBuff[i].syscall_res, info.syscall_res);
		}
		
		// Push get_syscalls_log AFTER verifying log
		info.syscall_num = 245;
		info.syscall_res = 0;
		queue.push(info);
		
		ASSERT_EQ(disable_syscalls_logging(PID), 0);
	}

	// Deallocate memory
	delete[] (pBuff);
	delete[] (arrOps);
	
	return bResult;
}

// Performs NUM_OF_OPS random syscalls and logs them.
// Assumes enable_syscalls_logging has been called and was pushed into queue.
bool logSystemCall(int* arrOps, const int NUM_OF_OPS, staticQueue<struct syscall_log_info>& queue){
	// Test variables
	bool bResult = true;
	
	// Get random operations
	for (int i=0 ; i<NUM_OF_OPS ; ++i)
		arrOps[i] = rand();
	
	// Perform ops
	for (int i=0 ; i<NUM_OF_OPS ; ++i)
		intToSystemCall(arrOps[i], queue);
	
	return bResult;
}

// Reads iSizeToRead ops and compares to 
bool getSystemCall(const int iSizeToRead, enveloped_syscall_log_info* pBuff, staticQueue<struct syscall_log_info>& queue){
	// Test variables
	bool bResult = true;
	struct syscall_log_info info;
	
	// Try reading iSizeToRead calls
	const int iGetRes = get_syscalls_log(PID, iSizeToRead, pBuff);
	ASSERT_EQ(iGetRes, 0); // 245
	
	// verify log BEFORE push(get_syscalls_log) !
	for (int i=0 ; i<iSizeToRead ; ++i){
		info = queue.pop();
		if (pBuff[i].syscall_num != info.syscall_num)
		{
			std::cout << std::endl << "pBuff[" << i << "].syscall_num = " << pBuff[i].syscall_num << std::endl;
			std::cout << "info.syscall_num = " << info.syscall_num << std::endl;
		}
		ASSERT_EQ((pBuff[i].syscall_num), (info.syscall_num)); 
		ASSERT_EQ(pBuff[i].syscall_res, info.syscall_res);
	}
	
	// Push get_syscalls_log AFTER verifying log
	info.syscall_num = 245;
	info.syscall_res = iGetRes;
	queue.push(info);
	
	return bResult;
}

// Logs syscalls and checks final. Does not remove elements intermediately.
bool checkWrapAroundHelper(const int LOG_SIZE, const int NUM_OF_OPS){
	// Test variables
	bool bResult = true;
	struct syscall_log_info info;
	
	// Memory allocations must be performed before enabling logging.
	int* arrOps = new int[NUM_OF_OPS];
	enveloped_syscall_log_info* pBuff = new enveloped_syscall_log_info[LOG_SIZE];
	staticQueue<struct syscall_log_info>* pQueue = new staticQueue<struct syscall_log_info>(LOG_SIZE);
	
	// Push first call and enable logging
	info.syscall_num = 243;
	info.syscall_res = enable_syscalls_logging(PID, LOG_SIZE); // 243
	pQueue->push(info);
	
	// Logs NUM_OF_OPS system call
	logSystemCall(arrOps, NUM_OF_OPS, *pQueue);
	
	// Calculate final size of log
	int iFinalLogSize = min(LOG_SIZE, NUM_OF_OPS);
	
	// Gets and verifies iFinalLogSize system calls
	getSystemCall(iFinalLogSize, pBuff, *pQueue);

	// Disable logging
	ASSERT_EQ( disable_syscalls_logging(PID), 0);

	// Deallocate memory
	delete (pQueue);
	delete[] pBuff;
	delete[] arrOps;
	
	return bResult;
}

int checkWrapAround_repeated(){
	// Test variables
	int bResult = true;
	
	// (LOG_SIZE, NUM_OF_OPS)
	ASSERT_EQ(true, checkWrapAroundHelper(1, 1)		);
	ASSERT_EQ(true, checkWrapAroundHelper(1, 10)		);
	ASSERT_EQ(true, checkWrapAroundHelper(1, 100)	);
	
	ASSERT_EQ(true, checkWrapAroundHelper(2, 1)		);
	ASSERT_EQ(true, checkWrapAroundHelper(2, 2)		);
	ASSERT_EQ(true, checkWrapAroundHelper(2, 3)		);
	ASSERT_EQ(true, checkWrapAroundHelper(2, 4)		);
	ASSERT_EQ(true, checkWrapAroundHelper(2, 5)		);
	ASSERT_EQ(true, checkWrapAroundHelper(2, 6)		);
	ASSERT_EQ(true, checkWrapAroundHelper(2, 11)		);
	ASSERT_EQ(true, checkWrapAroundHelper(2, 12)		);
	ASSERT_EQ(true, checkWrapAroundHelper(2, 13)		);
	ASSERT_EQ(true, checkWrapAroundHelper(2, 21)		);
	ASSERT_EQ(true, checkWrapAroundHelper(2, 9999)	);
	
	ASSERT_EQ(true, checkWrapAroundHelper(3, 1)		);
	ASSERT_EQ(true, checkWrapAroundHelper(3, 2)		);
	ASSERT_EQ(true, checkWrapAroundHelper(3, 3)		);
	ASSERT_EQ(true, checkWrapAroundHelper(3, 4)		);
	ASSERT_EQ(true, checkWrapAroundHelper(3, 5)		);
	ASSERT_EQ(true, checkWrapAroundHelper(3, 6)		);
	ASSERT_EQ(true, checkWrapAroundHelper(3, 11)		);
	ASSERT_EQ(true, checkWrapAroundHelper(3, 12)		);
	ASSERT_EQ(true, checkWrapAroundHelper(3, 13)		);
	ASSERT_EQ(true, checkWrapAroundHelper(3, 21)		);
	ASSERT_EQ(true, checkWrapAroundHelper(3, 9999)	);
	
	ASSERT_EQ(true, checkWrapAroundHelper(4, 1)		);
	ASSERT_EQ(true, checkWrapAroundHelper(4, 2)		);
	ASSERT_EQ(true, checkWrapAroundHelper(4, 3)		);
	ASSERT_EQ(true, checkWrapAroundHelper(4, 4)		);
	ASSERT_EQ(true, checkWrapAroundHelper(4, 5)		);
	ASSERT_EQ(true, checkWrapAroundHelper(4, 6)		);
	ASSERT_EQ(true, checkWrapAroundHelper(4, 11)		);
	ASSERT_EQ(true, checkWrapAroundHelper(4, 12)		);
	ASSERT_EQ(true, checkWrapAroundHelper(4, 13)		);
	ASSERT_EQ(true, checkWrapAroundHelper(4, 21)		);
	ASSERT_EQ(true, checkWrapAroundHelper(4, 9999)	);
		
	ASSERT_EQ(true, checkWrapAroundHelper(12, 999)	);
	ASSERT_EQ(true, checkWrapAroundHelper(17, 99999) );
	ASSERT_EQ(true, checkWrapAroundHelper(23, 99999) );
	ASSERT_EQ(true, checkWrapAroundHelper(33, 99999) );
	ASSERT_EQ(true, checkWrapAroundHelper(51, 99999) );
	
	return bResult;
}

int checkWrapAround_edgeCases(){
	// Test variables
	int bResult = true;
	
	// (LOG_SIZE, NUM_OF_OPS)
	ASSERT_EQ(true, checkWrapAroundHelper(232, 513)	);
	ASSERT_EQ(true, checkWrapAroundHelper(113, 1071)	);
	ASSERT_EQ(true, checkWrapAroundHelper(300, 298)	);
	ASSERT_EQ(true, checkWrapAroundHelper(300, 299)	);
	ASSERT_EQ(true, checkWrapAroundHelper(300, 300)	);
	ASSERT_EQ(true, checkWrapAroundHelper(300, 301)	);
	ASSERT_EQ(true, checkWrapAroundHelper(300, 302)	);
	
	return bResult;
}

// Performs random syscalls and verifies log intermediately.
bool checkWrapAround_intermediateGetHelper(const int LOG_SIZE){
	// Test variables
	bool bResult = true;
	struct syscall_log_info info;	
	const int NUM_OF_OPS = 99971;
	int iOpsPerformed = 0;
	assert(LOG_SIZE>=11);
	const int iOpsToRead = 11;
	const int iOpsToPerform = 17;
	
	// Memory allocations must be performed before enabling logging.
	int* arrOps = new int[NUM_OF_OPS];
	enveloped_syscall_log_info* pBuff = new enveloped_syscall_log_info[LOG_SIZE];
	staticQueue<struct syscall_log_info>* pQueue = new staticQueue<struct syscall_log_info>(LOG_SIZE);
	
	// Push first call and enable logging
	info.syscall_num = 243;
	info.syscall_res = enable_syscalls_logging(PID, LOG_SIZE); // 243
	pQueue->push(info);
	++iOpsPerformed;
	
	//ASSERT_EQ(logSystemCall(arrOps, LOG_SIZE, *pQueue), true);
	//iOpsPerformed += LOG_SIZE;
	
	while (iOpsPerformed + iOpsToPerform < NUM_OF_OPS){
		// Logs NUM_OF_OPS system call
		ASSERT_EQ(logSystemCall(arrOps, iOpsToPerform, *pQueue), true);
		iOpsPerformed += iOpsToPerform;
		
		// Gets and verifies iFinalLogSize system calls
		ASSERT_EQ(getSystemCall(iOpsToRead, pBuff, *pQueue), true);
	}
	
	// Disable logging
	ASSERT_EQ(disable_syscalls_logging(PID), 0);

	// Deallocate memory
	delete (pQueue);
	delete[] pBuff;
	delete[] arrOps;
	
	return bResult;
}

int checkWrapAround_intermediateGet(){
	bool bResult = true;
	
	for (int i=0 ; i<10 ; ++i) {
		checkWrapAround_intermediateGetHelper(11);
		checkWrapAround_intermediateGetHelper(13);
		checkWrapAround_intermediateGetHelper(14);
		checkWrapAround_intermediateGetHelper(15);
		checkWrapAround_intermediateGetHelper(16);
		checkWrapAround_intermediateGetHelper(17);
		checkWrapAround_intermediateGetHelper(18);
		checkWrapAround_intermediateGetHelper(19);
		checkWrapAround_intermediateGetHelper(20);
		checkWrapAround_intermediateGetHelper(21);
		checkWrapAround_intermediateGetHelper(22);
		checkWrapAround_intermediateGetHelper(23);
		checkWrapAround_intermediateGetHelper(24);
		checkWrapAround_intermediateGetHelper(25);
		checkWrapAround_intermediateGetHelper(26);
		checkWrapAround_intermediateGetHelper(27);
		checkWrapAround_intermediateGetHelper(28);
		checkWrapAround_intermediateGetHelper(29);
		checkWrapAround_intermediateGetHelper(30);
		checkWrapAround_intermediateGetHelper(31);
		checkWrapAround_intermediateGetHelper(32);
		checkWrapAround_intermediateGetHelper(33);
		checkWrapAround_intermediateGetHelper(34);
		checkWrapAround_intermediateGetHelper(35);
		
		checkWrapAround_intermediateGetHelper(50);
		checkWrapAround_intermediateGetHelper(100);
		checkWrapAround_intermediateGetHelper(130);
		checkWrapAround_intermediateGetHelper(170);
	}
	
	return bResult;
}

int basicScenario_2(){
	// Test variables
	bool bResult = true;
	const int iLogSize = 20;
	const int iNumOfSyscalls = 3;
	int arrRes[iNumOfSyscalls];
	enveloped_syscall_log_info buff[iLogSize];	
	
	// Begin logging. Add call #0
	ASSERT_EQ(enable_syscalls_logging(PID, iLogSize), 0);	// 243
	
	// Add calls #1-8 	
	arrRes[0] = getpid();	// 20
	arrRes[1] = getppid();
	arrRes[2] = getpgrp();
	
	// Try reading 4 calls (Should succeed)
	ASSERT_EQ(get_syscalls_log(PID, iNumOfSyscalls+1, buff), 0);	// 245
	
	ASSERT_EQ(buff[0].syscall_num, 243);
	ASSERT_EQ(buff[0].syscall_res, 0);
	
	ASSERT_EQ(buff[1].syscall_num, GETPID);
	ASSERT_EQ(buff[1].syscall_res, arrRes[0]);
	
	ASSERT_EQ(buff[2].syscall_num, GETPPID);
	ASSERT_EQ(buff[2].syscall_res, arrRes[1]);
	
	ASSERT_EQ(buff[3].syscall_num, GETPGRP);
	ASSERT_EQ(buff[3].syscall_res, arrRes[2]);
	

 	// Disable logging
	ASSERT_EQ(disable_syscalls_logging(PID), 0);
	
	return bResult;
}

int basicScenario_1(){
	// Test variables
	bool bResult = true;
	const int iLogSize = 20;
	enveloped_syscall_log_info buff[iLogSize];	
	
	// Begin logging. Add call #0
	ASSERT_EQ(enable_syscalls_logging(PID, 10), 0);	// 243
		
	// Add calls #1-8 	
	for(int i=0 ; i<4 ; ++i){	
		sched_yield();								// 158
		getpid(); 									// 20
	}
		
	// Try reading 10 calls (Should fail)
	ASSERT_EQ(get_syscalls_log(PID, 10, buff), -1);	// 245
	ASSERT_EQ(errno, EINVAL);
	
	// Try reading 10 calls (Should succeed)
	ASSERT_EQ(get_syscalls_log(PID, 10, buff), 0);	// 245
	
	// Check log
	ASSERT_EQ(buff[0].syscall_num, 243); 	
	for(int i=0 ; i<4 ; ++i){	
		ASSERT_EQ(buff[1+2*i].syscall_num, 158); 
		ASSERT_EQ(buff[2+2*i].syscall_num, 20); 
	}
	ASSERT_EQ(buff[9].syscall_num, 245); 	
 	
	ASSERT_EQ(disable_syscalls_logging(PID), 0);
	
	return bResult;
}

bool checkFailValues_enable(){
	// Test variables
	bool bResult = true;
	const int iLogSize = 20;
	
	// pid<0
	ASSERT_EQ(enable_syscalls_logging(-1, iLogSize), -1);
	ASSERT_EQ(errno, ESRCH);
	ASSERT_EQ(enable_syscalls_logging(-256, iLogSize), -1);
	ASSERT_EQ(errno, ESRCH);
	ASSERT_EQ(enable_syscalls_logging(INT_MIN, iLogSize), -1);
	ASSERT_EQ(errno, ESRCH);
	
	// Process doesn't exist
	ASSERT_EQ(enable_syscalls_logging(INT_MAX, iLogSize), -1);
	ASSERT_EQ(errno, ESRCH);
	
	// SIZE<0
	ASSERT_EQ(enable_syscalls_logging(PID, -1), -1);
	ASSERT_EQ(errno, EINVAL);
	ASSERT_EQ(enable_syscalls_logging(PID, -256), -1);
	ASSERT_EQ(errno, EINVAL);
	ASSERT_EQ(enable_syscalls_logging(PID, INT_MIN), -1);
	ASSERT_EQ(errno, EINVAL);
	
	// enable_syscalls_logging already invoked
	ASSERT_EQ(enable_syscalls_logging(PID, iLogSize), 0); 	// First should succeed
	ASSERT_EQ(enable_syscalls_logging(PID, iLogSize), -1); 	// Second should fail
	ASSERT_EQ(errno, EINVAL);
	ASSERT_EQ(disable_syscalls_logging(PID), 0);			// Delete log
	
	// memory allocation failure
	ASSERT_EQ(enable_syscalls_logging(PID, INT_MAX), -1); 	// If you did pass this- congrats. you have a super computer
	ASSERT_EQ(errno, ENOMEM);

	return bResult;
}

bool checkFailValues_disable(){
	// Test variables
	bool bResult = true;
	
	// pid<0
	ASSERT_EQ(disable_syscalls_logging(-1), -1);
	ASSERT_EQ(errno, ESRCH);
	ASSERT_EQ(disable_syscalls_logging(-256), -1);
	ASSERT_EQ(errno, ESRCH);
	ASSERT_EQ(disable_syscalls_logging(INT_MIN), -1);
	ASSERT_EQ(errno, ESRCH);
	
	// Process doesn't exist
	ASSERT_EQ(disable_syscalls_logging(INT_MAX), -1);
	ASSERT_EQ(errno, ESRCH);
	
	// enable_syscalls_logging wasn't invoked
	ASSERT_EQ(disable_syscalls_logging(PID), -1);
	ASSERT_EQ(errno, EINVAL);

	return bResult;
}

bool checkFailValues_get(){
	// Test variables
	bool bResult = true;
	const int iLogSize = 20;
	enveloped_syscall_log_info buff[iLogSize];
	
	// pid<0
	ASSERT_EQ(get_syscalls_log(-1, 1, buff), -1);
	ASSERT_EQ(errno, ESRCH);
	ASSERT_EQ(get_syscalls_log(-256, 1, buff), -1);
	ASSERT_EQ(errno, ESRCH);
	ASSERT_EQ(get_syscalls_log(INT_MIN, 1, buff), -1);
	ASSERT_EQ(errno, ESRCH);
	
	// Process doesn't exist
	ASSERT_EQ(get_syscalls_log(INT_MAX, 1, buff), -1);
	ASSERT_EQ(errno, ESRCH);
	
	// size>log_size
	ASSERT_EQ(enable_syscalls_logging(PID, iLogSize), 0); 	// Should succeed
	ASSERT_EQ(get_syscalls_log(PID, 2, buff), -1);
	ASSERT_EQ(errno, EINVAL);
	ASSERT_EQ(get_syscalls_log(PID, 10, buff), -1);
	ASSERT_EQ(errno, EINVAL);
	ASSERT_EQ(get_syscalls_log(PID, INT_MAX, buff), -1);
	ASSERT_EQ(errno, EINVAL);
	ASSERT_EQ(disable_syscalls_logging(PID), 0);			// Delete log
		
	// SIZE<0
	ASSERT_EQ(get_syscalls_log(PID, -1, buff), -1);
	ASSERT_EQ(errno, EINVAL);
	ASSERT_EQ(get_syscalls_log(PID, -256, buff), -1);
	ASSERT_EQ(errno, EINVAL);
	ASSERT_EQ(get_syscalls_log(PID, INT_MIN, buff), -1);
	ASSERT_EQ(errno, EINVAL);
	
	// enable_syscalls_logging wasn't invoked
	ASSERT_EQ(get_syscalls_log(PID, 1, buff), -1);
	ASSERT_EQ(errno, EINVAL);
	
	// copy_to_user failure
	ASSERT_EQ(enable_syscalls_logging(PID, iLogSize), 0); 	// Should succeed
	ASSERT_EQ(get_syscalls_log(PID, 1, NULL), -1);
	ASSERT_EQ(errno, EINVAL);
	ASSERT_EQ(get_syscalls_log(PID, 1, (struct syscall_log_info*)(1)), -1);	// Illegal address
	ASSERT_EQ(errno, ENOMEM);
	ASSERT_EQ(disable_syscalls_logging(PID), 0);			// Delete log
	
	return bResult;
}

// Use this to find out what system calls a function calls.
bool discoverNewSystemCalls(){
	// Test variables
	bool bResult = true;
	const int iLogSize = 20;
	enveloped_syscall_log_info buff[iLogSize];	
	std::queue<int> syscalls;
	
	// Begin logging
	ASSERT_EQ(enable_syscalls_logging(PID, 10), 0);	// 243
	
	//////////////////////////////
	// Call desired function	//
	//sched_getparam(PID, &GLOBAL_SCHED_PARAM);
	int num = 5;
	printf("how many logs do you want? (out of %d)\n",num);
	scanf("%d",&num);
	//////////////////////////////
	
	// A syscall was performed
	if(get_syscalls_log(PID, 2, buff) == 0){				// 245
		
		// Just for a simpler while loop
		buff[0].syscall_num = buff[1].syscall_num;
		
		int syscalls[5];
		int numOfCalls=0;
		
		// Store system call numbers
		while (buff[0].syscall_num != 245){
			syscalls[numOfCalls++] = buff[0].syscall_num;
			ASSERT_EQ(get_syscalls_log(PID, 1, buff), 0);
		}
		
		// Print system call numbers
		for (int i=0 ; i<numOfCalls ; ++i)
			std::cout << syscalls[i] << std::endl;
	}
	
	// No syscall was performed
	else{
		// Sanity check: get_syscalls_log should have logged 245
		ASSERT_EQ(get_syscalls_log(PID, 2, buff), 0);
		ASSERT_EQ(buff[0].syscall_num, 243);
		ASSERT_EQ(buff[1].syscall_num, 245);
		
		// Print message
		std::cout << "No syscalls performed." << std::endl;
	}
 	
	// Disable logging
	ASSERT_EQ(disable_syscalls_logging(PID), 0);

	return bResult;
}

bool checkFork_isLoggedInParent(){
	// Test variables
	bool bResult = true;
	struct syscall_log_info info;	
	const int LOG_SIZE = 20;
	
	// Memory allocations must be performed before enabling logging.
	enveloped_syscall_log_info* pBuff = new enveloped_syscall_log_info[LOG_SIZE];
	staticQueue<struct syscall_log_info>* pQueue = new staticQueue<struct syscall_log_info>(LOG_SIZE);
	
	// Push first call and enable logging
	info.syscall_num = 243;
	info.syscall_res = enable_syscalls_logging(PID, LOG_SIZE); // 243
	ASSERT_EQ(info.syscall_res, 0);
	pQueue->push(info);
			
	int childProcess = fork();					
	
	// Son
	if(childProcess == 0)
	{
		//printf("Son dies.\n");
		exit(0);
	}
	
	// Parent
	else{
		info.syscall_num = FORK;
		info.syscall_res = childProcess;
		pQueue->push(info);
		
		// Get and verify 2 system calls
		ASSERT_EQ( getSystemCall(2, pBuff, *pQueue), true);
		
		///////////////////////////////////////////////////////////
		// Try reading iSizeToRead calls
		/*ASSERT_EQ(get_syscalls_log(PID, 2, pBuff), 0); // 245	
		
		// Compare get_syscalls_log output to simulated output
		for (int i=0 ; i<2 ; ++i){
			info = pQueue->pop();
			ASSERT_EQ(pBuff[i].syscall_num, info.syscall_num); 
			ASSERT_EQ(pBuff[i].syscall_res, info.syscall_res);
		}*/
		///////////////////////////////////////////////////////////
		
		
		// Disable logging
		ASSERT_EQ( disable_syscalls_logging(PID), 0);
		
		// Deallocate memory
		delete (pQueue);
		delete[] pBuff;
	}
	
	return bResult;
}


bool CheckFork_doesntCallKfree(){
	bool bResult = true;
	
	// Test variables	
	int childProcess = fork();					
	
	// Son
	if(childProcess == 0)
	{
		exit(0);
	}
	
	// Parent
	else{
		sleep(2);
	}
	
	return bResult;
}

int main(){
	// Use this to find which system calls are performed by a function
	//discoverNewSystemCalls();
	
	printf("Test begins\n");
	
	// Bad parameters and the most basic failures
	RUN_TEST(checkFailValues_enable);
	RUN_TEST(checkFailValues_disable);
	RUN_TEST(checkFailValues_get);
	
	// Perform syscalls and read them
	RUN_TEST(basicScenario_1);
	RUN_TEST(basicScenario_2);
	
	// Check wrap around of buffer
	RUN_TEST(checkWrapAround_repeated);
	RUN_TEST(checkWrapAround_edgeCases);
	RUN_TEST(checkWrapAround_intermediateGet);
	
	
	// Check fork behavior
	RUN_TEST(CheckFork_doesntCallKfree); // Primarily checks you didn't kfree's in fork;
	RUN_TEST(checkFork_isLoggedInParent);
	
	// To be implemented
	// Check memory leak
	//RUN_TEST(checkMemLeak_disable);
	//RUN_TEST(checkMemLeak_fork);
	//RUN_TEST(checkMemLeak_exit);
	
	
	// LOTS of random operations
	RUN_TEST(stressTest);
	
	printf("Finita la comedia\n");
	
	return 0;
}
