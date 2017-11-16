
#include "hw1_syscalls.h"
#include "staticQueue.hpp"
#include "testsMacros.h"
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>


#include <assert.h>

#define KGRN "\x1B[32m"
#define KNRM "\x1B[0m"
//macro that invokes syscall when error occures in syscall in kernel , you will message in user mode 
#define DO_SYS(syscall) \
do \
{ \
  if (-1 == (syscall)) \
  { \
	printf("Line: %d\n", __LINE__);\
    perror(#syscall); \
    exit(1); \
  } \
} \
while(0);

#define WAIT_FOR_ALL_CHILDREN_TO_END() \
while(wait(NULL) != -1)

/*
		sched_yield();						// 158
		getpid(); 							// 20
		getppid(); 							// 64
		FILE *fp = fopen("file.txt", "r");	// 5
		fclose (fp);						// 6
		fork() 								// 2
		waitpid() 							// 7
*/

int tryToGetSyscallsAfterDisablingLogger(){
	// Test variables
	int result = 1;
	int pid= getpid();
	int res;
	struct syscall_log_info buff[9];
	// Begin logging. Add call #0
	DO_SYS(enable_syscalls_logging(pid, 9));		// 243
	enable_syscalls_logging(pid, 9);
	assert(errno == EINVAL);
	DO_SYS(res = disable_syscalls_logging(pid));
	assert(res == 0);
	res = get_syscalls_log(pid, 9, buff);	// 245
	assert(res == -1);
	assert(errno == EINVAL);
	return result;

}

int enableSyscallsLoggingDoubleInitializationFail(){
	// Test variables
	int result = 1;
	int pid= getpid();
	int res;
	
	// Begin logging. Add call #0
	DO_SYS(enable_syscalls_logging(pid, 9));		// 243
	enable_syscalls_logging(pid, 9);
	assert(errno == EINVAL);
	DO_SYS(res = disable_syscalls_logging(pid));
	assert(res == 0);
	
	return result;

}

int forkTestGetLogOfZombieChild(){
	int result = 1;
	struct syscall_log_info buff[11];
	int i=0;
	

	int childProcessCaller;
	int childProcessGetter;

	DO_SYS(childProcessCaller = fork());
	if(childProcessCaller == 0)// CALLER SON
	{
		// Begin logging. Add call #0
		DO_SYS(enable_syscalls_logging(getpid(), 11));		// 243
		// Add calls #1-9 	 
		for(i=0 ; i<4 ; ++i){	
			sched_yield();						// 158
			getppid();							// 64
		}
		exit(0);	
	} 
	if (childProcessCaller > 0) // FATHER in case when only Caller is borned
	{
		DO_SYS(childProcessGetter = fork());
		if(childProcessGetter == 0)// GETTER SON
		{
			sleep(3);
			int result2=-2;
			while(result2 != 0)// Wait untill you will success
			{
				result2 = get_syscalls_log(childProcessCaller, 10, buff);
			}
			int res = disable_syscalls_logging(childProcessCaller);
			assert(res == 0);
			exit(0);	
		} 
		if(childProcessGetter > 0) // FATHER in case when both CALLER and GETTER were borned
		{
			int status = -2;
			while(status != childProcessGetter) //Wait untill Getter will succeed and end
			{
				status = waitpid(childProcessGetter,NULL,0);
			}

			WAIT_FOR_ALL_CHILDREN_TO_END(); // Wait for everyone to finish is someone left running
		}
	}

	return result;
}

int tryReadToBufferWithOneCellSize(){
	// Test variables
	int result = 1;
	struct syscall_log_info buff[9];
	int pid= getpid();
	int i=0;
	int res;
	
	// Begin logging. Add call #0
	DO_SYS(enable_syscalls_logging(pid, 9));		// 243
		
	// Add calls #1-9 	 
	for(i=0 ; i<4 ; ++i){	
		sched_yield();						// 158
		getppid();							// 64
	}
	
	getpid(); 								// 20
	for(int i = 0; i < 9; i++)
	{
		res = get_syscalls_log(pid, 1, buff);	// 245
		assert(res == 0);
		if(i == 8){
			assert(buff[0].syscall_num == 20);
			break;
		}
		if(i%2 == 0)
			assert(buff[0].syscall_num == 158);
		else
			assert(buff[0].syscall_num == 64);
	}

	res = get_syscalls_log(pid, 1, buff);
	assert(res == 0);

	res = disable_syscalls_logging(pid);
	assert(res == 0);

	return result;
}

int forkTestSingleSimplestSonCreateCheckLogStructOfSon()
{
	int result = 1;
	
	int childProcess;
	
	DO_SYS(childProcess = fork());
	if(childProcess == 0) // son
	{
		exit(0);
	}
	if (childProcess > 0) // Im your Father 
	{
		WAIT_FOR_ALL_CHILDREN_TO_END();
	}
	
	return result;
}

int zeroSizeBufferEnablingAndGetSyscalls()
{
	int result = 1;
	struct syscall_log_info buff[9];
	int pid= getpid();

	enable_syscalls_logging(pid, 0);
	int res = get_syscalls_log(pid, 0, buff);	// 245
	assert(res == 0);
	
	res = disable_syscalls_logging(pid);
	assert(res == 0);
	
	return result;
}

int forkTestCreateSonAndGrandsonGetLogFromGrandSonAndSonFailed()
{
	int result = 1;
	struct syscall_log_info buff[9];
	int pid= getpid();
	int i=0;
	int res;

	// Begin logging. Add call #0
	enable_syscalls_logging(pid, 9);		// 243

	// Add calls #1-9
	for(i=0 ; i<4 ; ++i){
		sched_yield();						// 158
		getppid();							// 64
	}

	getpid(); 								// 20

	// Try reading 9 calls (Should succeed)
	res = get_syscalls_log(pid, 9, buff);	// 245
	assert(res == 0);


	int childProcess;
	int grandChildProcess;
	
	DO_SYS(childProcess = fork());
	if(childProcess == 0) // son created
	{
		DO_SYS(grandChildProcess = fork());
		if(grandChildProcess == 0) // grandchild created 
		{
			res = get_syscalls_log(getpid(), 9, buff); // Fail Case , forked son borned in intit-clear state
			assert(res == -1);
			assert(errno == EINVAL);
			exit(0);
		}
		if(grandChildProcess > 0) // son case
		{
			res = get_syscalls_log(getpid(), 9, buff); // Fail Case , forked son borned in intit-clear state
			assert(res == -1);
			assert(errno == EINVAL);
			exit(0);
		}
	}
	if (childProcess > 0) // Father Case
	{
		WAIT_FOR_ALL_CHILDREN_TO_END();
	}
	
	return result;
}

int forkTestWrapAroundBufferWithGetLogFromSonFailedAndGetLogFromFatherSucceded(){
	// Test variables
	int result = 1;
	struct syscall_log_info buff[9];
	int pid= getpid();
	int i=0;
	int res;

	// Begin logging. Add call #0
	enable_syscalls_logging(pid, 9);		// 243

	// Add calls #1-9
	for(i=0 ; i<4 ; ++i){
		sched_yield();						// 158
		getppid();							// 64
	}

	getpid(); 								// 20

	// Try reading 9 calls (Should succeed)
	res = get_syscalls_log(pid, 9, buff);	// 245
	assert(res == 0);

	// Check log
	for(i=0 ; i<4 ; ++i){
		assert(buff[2*i].syscall_num == 158);
		assert(buff[1+2*i].syscall_num == 64);
	}
	assert(buff[8].syscall_num == 20);

	int childProcess;
	childProcess = fork();
	if(childProcess == 0) // son
	{
		res = get_syscalls_log(getpid(), 9, buff); // Fail Case , forked son borned in intit clear state
		assert(res == -1);
		assert(errno == EINVAL);
		exit(0);
	}
	if (childProcess > 0) // Im your Father 
	{
		res = get_syscalls_log(pid, 2, buff); // read father`s buffer. Need to be with 2 logs only: [245, 2] (get_syscalls_log - 245; fork of child - 2)
		assert(buff[0].syscall_num == 245); 
		assert(buff[1].syscall_num == 2); // fork of child
		WAIT_FOR_ALL_CHILDREN_TO_END();
	}

	res = disable_syscalls_logging(pid);
	assert(res == 0);

	return result;
}

int main(){
	
	printf("Test begins\n"); 
 
	RUN_TEST(enableSyscallsLoggingDoubleInitializationFail);
	RUN_TEST(zeroSizeBufferEnablingAndGetSyscalls);
	RUN_TEST(tryToGetSyscallsAfterDisablingLogger);
	RUN_TEST(tryReadToBufferWithOneCellSize);

	RUN_TEST(forkTestWrapAroundBufferWithGetLogFromSonFailedAndGetLogFromFatherSucceded);
	RUN_TEST(forkTestCreateSonAndGrandsonGetLogFromGrandSonAndSonFailed);
	RUN_TEST(forkTestSingleSimplestSonCreateCheckLogStructOfSon); 
	RUN_TEST(forkTestGetLogOfZombieChild); 

	printf("%sTest passed! :)%s\n", KGRN, KNRM);  
	
	return 0;
}
