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

bool checkPoolPriorities() {
	struct sched_param param;
	param.sched_priority = 139;

	int child = fork();
	if(child == 0) {
		sleep(2);

		exit(0);
	}
	else {
		int pid = getpid();
		sched_setscheduler(child, SCHED_POOL, &param);
		for(int i = 0; i < 20; i++)
			sacrifice_timeslice(child); // add time to pool

		param.sched_priority = 0; // get child out of pool
		sched_setscheduler(child, SCHED_OTHER, &param);

		int res;
		for(int i = 0; i < 140; i++) {
			param.sched_priority = i;
			res = sched_setscheduler(pid, SCHED_POOL, &param); // add to pool in prio = i
			ASSERT_EQUAL(res, 0, 66);
			if (res != 0)
				cout << " failed at iteration " << i << flush;
			res = search_pool_level(pid,i);
			ASSERT_EQUAL(res, 0, 70); // process is only one in pool should be at position 0
			if (res != 0)
				cout << " failed at iteration " << i << flush;
		}
		wait(NULL);
	}


	return true;
}

int main() {
	int pid = getpid();

	RUN_TEST("checkPoolPriorities", checkPoolPriorities());

	return 0;
}
