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

bool checkEndOfTimepool() {
	struct sched_param param;
	param.sched_priority = 20;

	int child = fork();
	if (child == 0) {
		sleep(3); // wait to enter the pool
		int pid = getpid();
		int y = 0;
		while(get_remaining_timeslice(pid) > 1) { // decrease timepool
			y++;
		}
		for(int i = 0; i < 2000000000; i++)
			y++;
		exit(0);
	}
	else {
		int child2 = fork();
		if (child2 == 0) {
			sleep(3); // wait to enter the pool

			int y = 0;
			for(int i = 0; i < 2000000; i++)
				y++;
			exit(0);
		}
		else {
			sched_setscheduler(child, SCHED_POOL, &param);
			sched_setscheduler(child2, SCHED_POOL, &param);
			sacrifice_timeslice(child); // add time to pool
			sacrifice_timeslice(child); // add time to pool

			int res = get_remaining_timeslice(child);
			ASSERT_DIFFERENT(res, 0, 76);
			res = (res/100) + 10;
			sleep(res); // sleep until timepool is finished.

			res = get_remaining_timeslice(child);
			ASSERT_EQUAL(res, 0, 81); // time_pool was supposed to end
			res = search_pool_level(child, 20);
			ASSERT_EQUAL(res, 0, 83); // child should still be in pool as timepool ended
			res = search_pool_level(child2, 20);
			ASSERT_EQUAL(res, 1, 85); // child2 should still be in pool as timepool ended
			param.sched_priority = 0;
			sched_setscheduler(child, SCHED_OTHER, &param); // return child to sched_other
			sched_setscheduler(child2, SCHED_OTHER, &param); // return child to sched_other
			wait(NULL);
			wait(NULL);
		}
	}

	return true;
}

int main() {
	int pid = getpid();

	RUN_TEST("checkEndOfTimepool", checkEndOfTimepool());

	return 0;
}
