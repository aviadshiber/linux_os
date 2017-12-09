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
			cout << endl << "assertion failed at line " << line<<"!("<<a<<"!="<<b<<")"<<endl << flush; \
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

bool checkGetTimeslice_Part3() {
	struct sched_param param;
	param.sched_priority = 20;

	int child = fork();
	if (child == 0) {
		sleep(3); // wait to enter the pool

		exit(0);
	}
	else {

		// first child was entered to pool just so we can add some time to timepool
		sched_setscheduler(child, SCHED_POOL, &param);
		sacrifice_timeslice(child); // add time to the pool

		int child1 = fork();
		if(child1 == 0) {
			sleep(3); // wait to enter to pool;

			exit(0);
		}
		else {
			int res, res1, res2, res3;

			res1 = get_remaining_timeslice(child1); // time after fork before pool
			sched_setscheduler(child1, SCHED_POOL, &param);
			res2 = get_remaining_timeslice(child1); // time in timepool
			ASSERT_DIFFERENT(res1, res2, 71);
			res = sacrifice_timeslice(child); // res is parent timeslice which was sacrificed
			res3 = get_remaining_timeslice(child1); // new time in timepool
			ASSERT_EQUAL(res3, (res + res2), 74); // current timepool = old timepool + sacrifice amount
			sleep(10);
			param.sched_priority = 0;
			sched_setscheduler(child, SCHED_OTHER, &param);
			sched_setscheduler(child1, SCHED_OTHER, &param);
			wait(NULL);
			wait(NULL);
		}
	}
	return true;
}

int main() {
	int pid = getpid();

	RUN_TEST("checkGetTimeslice_Part3", checkGetTimeslice_Part3());

	return 0;
}
