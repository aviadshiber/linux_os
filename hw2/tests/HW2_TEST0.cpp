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

bool checkSearchPoolLevel_Part2() {
	int res = search_pool_level(-1, 25);
	ASSERT_EQUAL (res, -1, 45);
	ASSERT_EQUAL (errno, 3, 46);
	res = search_pool_level(1, 140);
	ASSERT_EQUAL (res, -1, 48);
	ASSERT_EQUAL (errno, 22, 49);
	res = search_pool_level(1, -1);
	ASSERT_EQUAL (res, -1, 51);
	ASSERT_EQUAL (errno, 22, 52);

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
			int res, res1;

			res1 = get_remaining_timeslice(child1); // time after fork before pool
			sched_setscheduler(child1, SCHED_POOL, &param);
			res = search_pool_level (child1, 20);
			ASSERT_TRUE (res >= 0, 81);
			
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

	RUN_TEST("checkSearchPoolLevel_Part2", checkSearchPoolLevel_Part2());

	return 0;
}
