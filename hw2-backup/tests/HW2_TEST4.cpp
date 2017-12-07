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

bool checkSchedYieldInPool() {
	int parent = getpid();
	struct sched_param param;
	param.sched_priority = 20;

	int child = fork();
	if (child == 0) {
		int pid = getpid();
		// enter child to pool
		sched_setscheduler(pid, SCHED_POOL, &param);

		int grandson = fork();
		if(grandson == 0) {
			int res = search_pool_level(getppid(), 20);
			ASSERT_EQUAL(res, 1, 57); // child yielded, should be after grandson in queue now
			res = search_pool_level(getpid(), 20);
			ASSERT_EQUAL(res, 0, 59); // grandson should be first in queue now
			exit(0);
		}
		else { // note: pool if FIFO so child should run before grandson
			sched_yield();

			wait(NULL); // wait for grandson
			exit(0);
		}
	}
	else {
		for (int i = 0; i < 10; i++) // wait for child to enter itself to pool
			sched_yield();
		sacrifice_timeslice(child);
		sacrifice_timeslice(child);
		wait(NULL); //wait for child

		// enter several processes to pool check yield
		int children[10] = { 0 };
		param.sched_priority = 10;
		for (int i = 0; i < 10; i++) {
			children[i] = fork();
			if (children[i] == 0) { // child process
				sleep(2); // enter the pool when wake up
				break;
			}
		}
		if (getpid() == parent) {
			for (int i = 0; i < 20; i++)
				sched_yield();
			for (int i = 0; i < 10; i++) // enter children to pool
				sched_setscheduler(children[i], SCHED_POOL, &param);
			sacrifice_timeslice(children[1]); // add time to pool
			sacrifice_timeslice(children[1]); // add time to pool
			for(int i = 0, j = 0; i < 2000000000; i++) // wait for sons to wake up
				j++;
			for(int i = 0; i < 10; i++){
				int pos = search_pool_level(children[i], 10);
				ASSERT_EQUAL(pos, i, 97); // all children entered in order
				if (pos != i)
					cout << " failed at iteration " << i << flush;
			}
			sleep(3); // let first child do yield
			for(int i = 1; i < 10; i++){
				int pos = search_pool_level(children[i], 10);
				ASSERT_EQUAL(pos, i-1, 104); // all children moved 1 place forward after 1st yielded
				if (pos != i-1)
					cout << " failed at iteration " << i << flush;
			}
			int pos = search_pool_level(children[0], 10);
			ASSERT_EQUAL(pos, 9, 109); // first childed became last in queue after yield;
			param.sched_priority = 0;
			for (int i = 0; i < 10; i++) {
				sched_setscheduler(children[i], SCHED_OTHER, &param); // get all children out of pool
			}
			for (int i = 0; i < 10; i++)
				wait(NULL); // wait for all children to exit

		}
		else if (children[0] == 0) { // move first child to end of queue
			sched_yield();

			exit(0);
		}
		else if (children[1] == 0){ // let 2nd child do some work until parent wakes up
			int y = 0;
			for (int i = 0; i < 2000000000; i++)
				y++;
			y = 0;
			for (int i = 0; i < 2000000000; i++)
				y++;
			y = 0;
			exit(0);
		}
		else { // all other children just do exit
			exit(0);
		}
	}
	return true;
}

int main() {
	int pid = getpid();

	RUN_TEST("checkSchedYieldInPool", checkSchedYieldInPool());

	return 0;
}
