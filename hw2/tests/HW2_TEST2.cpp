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

bool checkSacrificeTimeslice_Part3() {
	struct sched_param param;
	param.sched_priority = 20;

	int child = fork();

	if (child == 0) {
		sleep(3); // wait to enter the pool
		// process in SCHED_POOL try to sacrifice timeslice
		int res = sacrifice_timeslice(getppid());
		ASSERT_EQUAL(res, -1, 53);
		ASSERT_EQUAL(errno, EINVAL, 54);
		exit(0);
	}
	else {
		int res, timepool, timepool2;
		sched_setscheduler(child, SCHED_POOL, &param); // enter child to pool
		timepool = get_remaining_timeslice(child); // get current timepool
		res = sacrifice_timeslice(child); // add time to the pool
		timepool2 = get_remaining_timeslice(child); // get current timepool
		ASSERT_EQUAL(timepool2, (timepool + res), 63); // sacrificied time was supposed to be added to the pool

		int child1 = fork();
		if(child1 == 0) {
			sleep(3); // wait to enter to pool;

			exit(0);
		}
		else {
			sched_setscheduler(child1, SCHED_POOL, &param);
			timepool = get_remaining_timeslice(child1); // get current timepool
			res = sacrifice_timeslice(child); // sacrifice time to first son
			timepool2 = get_remaining_timeslice(child1); // new time in timepool
			ASSERT_EQUAL(timepool2, (timepool + res), 76);
			sacrifice_timeslice(child);
			sleep(10); // let child do code

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

	RUN_TEST("checkSacrificeTimeslice_Part3", checkSacrificeTimeslice_Part3());

	return 0;
}
