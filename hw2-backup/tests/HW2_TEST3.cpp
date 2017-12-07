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

bool checkForkInPool() {
	int parent = getpid();
	struct sched_param param;
	param.sched_priority = 20;

	int child = fork();
	if (child == 0) {
		int pid = getpid();
		// enter child to pool
		sched_setscheduler(pid, SCHED_POOL, &param);
		sleep(1); // wait to actually enter the pool and for parent to sacrifice time
		int res = get_remaining_timeslice(pid);

		int grandson = fork();
		if(grandson == 0) {
			exit(0);
		}
		else { // note: pool if FIFO so child should run before grandson
			// check timepool wasn't decreased
			int res2 = get_remaining_timeslice(pid);
			ASSERT_TRUE(res == res2 || res-1 == res2, 63); // res-1 is case we had a tick in the middle
			res = search_pool_level(pid, 20);
			ASSERT_EQUAL(res, 0, 65);
			res = search_pool_level(grandson, 20);
			ASSERT_EQUAL(res, 1, 67); // grandson is scheduled in same prio as child but after it in queue
			wait(NULL); // wait for grandson to finish
			exit(0);
		}
	}
	else {
		for (int i = 0; i < 10; i++) // wait for child to enter itself to pool
			sched_yield();
		sacrifice_timeslice(child); // add time to pool
		sacrifice_timeslice(child); // add time to pool
		sleep(3); // wait for child to finish
		param.sched_priority = 0;
		sched_setscheduler(child, SCHED_OTHER, &param);
		wait(NULL); // wait for child
		// check fork inside pool with more than process in same queue
		param.sched_priority = 15;
		int children[5] = { 0 };
		for (int i = 0; i < 5; i++) {
			children[i] = fork();
			if (children[i] == 0) { // child
				sleep(2); // enter the pool when wake up
				break;
			}
		}
		if (getpid() == parent) {
			for (int i = 0; i < 20; i++)
				sched_yield();
			for (int i = 0; i < 5; i++) // enter children to pool
				sched_setscheduler(children[i], SCHED_POOL, &param);
			sacrifice_timeslice(children[1]); // add time to pool
			sacrifice_timeslice(children[1]); // add time to pool
			for(int i = 0, j = 0; i < 2000000000; i++) // wait for sons to wake up
				j++;
			for(int i = 0; i < 5; i++){
				int pos = search_pool_level(children[i], 15);
				ASSERT_EQUAL(pos, i, 102); // all children entered in order
				if (pos != i)
					cout << " failed at iteration " << i << flush;
			}
			sleep(3); // let first child do fork
			param.sched_priority = 0;
			for (int i = 0; i < 5; i++) {
				sched_setscheduler(children[i], SCHED_OTHER, &param); // get all children out of pool
			}
			for (int i = 0; i < 5; i++)
				wait(NULL); // wait for all children to exit
		}
		else if (children[0] == 0) { // first child does fork
			int grandson = fork();
			if (grandson == 0) {
				exit(0);
			}
			else {
				int res = search_pool_level(grandson, 15);
				ASSERT_EQUAL(res, 5, 121); // grandson was supposed to be added at end of queue
				int y = 0;
				for (int i = 0; i < 2000000000; i++)
					y++;
				y = 0;
				for (int i = 0; i < 2000000000; i++)
					y++;
				param.sched_priority = 0;
				sched_setscheduler(grandson, SCHED_OTHER, &param);
				wait(NULL); // wait for grandson to finish
				exit(0);
			}
		}
		else { // all other children just do exit
			exit(0);
		}

	}
	return true;
}

int main() {
	int pid = getpid();

	RUN_TEST("checkForkInPool", checkForkInPool());

	return 0;
}
