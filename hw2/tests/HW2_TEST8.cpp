#include <errno.h>
#include <sched.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/resource.h>

#include "hw2_syscalls.h"

#define SCHED_OTHER		0
#define SCHED_FIFO		1
#define SCHED_RR		2
#define SCHED_POOL		3

#define ASSERT_EQUAL(a,b,line) \
		if(a != b) { \
			cout << endl << "assertion failed at line " << line << flush; \
		}
#define ASSERT_DIFFERENT(a,b,line) \
		if(a == b) { \
			cout << endl << "assertion failed at line " << line << flush; \
		}
#define ASSERT_TRUE(condition,line) \
		if(!(condition)) { \
			cout << endl << "assertion failed at line " << line << flush; \
		}
#define RUN_TEST(name, test) \
		cout << "Starting test " << name << "... " << flush; \
		if (test) \
			cout << endl << "[SUCCESS]" << endl; \
		else \
			cout << endl << "[FAILED]" << endl; \
		if (getpid() != pid) \
			return 0;

using namespace std;

bool checkPoolPrint() {
	struct sched_param param;
	param.sched_priority = 1;

	int child = fork();
	if(child == 0) {
		sleep(2);

		exit(0);
	}
	else {			//father's code
		int pid = getpid();
		sched_setscheduler(child, SCHED_POOL, &param);
		print_pool_level(0); //empty list							
		print_pool_level(1); //child in list level 1 (child)->
		for(int i = 0; i < 20; i++)
			sacrifice_timeslice(child); // add time to pool

		sched_setscheduler(pid, SCHED_POOL, &param);
		print_pool_level(1);		//level 1 (child)->(father)

		 param.sched_priority = 0; // get child out of pool
		 sched_setscheduler(child, SCHED_OTHER, &param);
		 print_pool_level(0); //empty list
		 print_pool_level(1);		//level 1 (father)->
		 sched_setscheduler(pid, SCHED_OTHER, &param);
		 print_pool_level(1);		//empty list
		 sched_setscheduler(pid, SCHED_POOL, &param);
		 print_pool_level(0);		//level 0 (father)->
		 sched_setscheduler(child, SCHED_POOL, &param);
		 print_pool_level(0);		//level 0 (father)->(child)->
		 param.sched_priority = 1; 
		 sched_setscheduler(child, SCHED_POOL, &param);
		 print_pool_level(0);		//level 0 (father)->
		 print_pool_level(1);		//level 1 (child)->
		param.sched_priority = 10; // 
		 sched_setscheduler(child, SCHED_POOL, &param);
		 print_pool_level(0);		//level 0 (father)->
		 print_pool_level(1);		//empty
		 print_pool_level(10);		//level 10 (child)->
		param.sched_priority = 12; // 
		 sched_setscheduler(pid, SCHED_POOL, &param);
		 print_pool_level(0);		//level 0 (father)->
		 print_pool_level(10);		//level 10 (child)->
		 print_pool_level(12);		//level 12 (father)->

		  for(int i = 0; i < 13; i++) {
		  	param.sched_priority = i;
		  	sched_setscheduler(pid, SCHED_POOL, &param); // add to pool in prio = i
		  	print_pool_level(i); //1 in each list
		// 	ASSERT_EQUAL(res, 0, 66);
		// 	if (res != 0)
		// 		cout << " failed at iteration " << i << flush;
		// 	res = search_pool_level(pid,i);
		// 	ASSERT_EQUAL(res, 0, 70); // process is only one in pool should be at position 0
		// 	if (res != 0)
		// 		cout << " failed at iteration " << i << flush;
		}
		wait(NULL);
	}


	return true;
}

int main() {
	int pid = getpid();
	RUN_TEST("checkPoolPrint", checkPoolPrint());

	return 0;
}
