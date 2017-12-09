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

bool general_pool_prio_check() {
	struct sched_param param;
    int i;
	int sacrafice_total_time=0;
	int child1 = fork();
  
    if(child1!=0){
        int child2= fork();
        if(child2!=0){ //the parent have two children
            int child3=fork();
            if(child3!=0){//the parent have three children
                int parent_pid = getpid();
                //printf("\nparent with pid %d have %d time_slice after 3 forks\n",get_remaining_timeslice(parent_pid));
                param.sched_priority = 0;
                ASSERT_DIFFERENT(sched_setscheduler(child1, SCHED_POOL, &param),-1,56);//move child1 to pool  at level 0
                ASSERT_DIFFERENT(sched_setscheduler(child2, SCHED_POOL, &param),-1,57);//move child2 to pool  at level 0
                ASSERT_DIFFERENT(sched_setscheduler(child3, SCHED_POOL, &param),-1,58);//move child3 to pool at level 0
                for(i=0;i<25;i++){// let's add to time pool 20 epochs of parent
                    sacrafice_total_time+=sacrifice_timeslice(child1);  //it does not matter which child we donate, they are all in the pool
                }
                //print_pool_level(0);
                // now we can verify the place of each child
                // state: level 0: child1->child2->child3
                ASSERT_EQUAL(search_pool_level(child2,0),1,65);
                ASSERT_EQUAL(search_pool_level(child1,0),0,66);
                ASSERT_EQUAL(search_pool_level(child3,0),2,67);
                param.sched_priority = 1;
                ASSERT_DIFFERENT(sched_setscheduler(child2, SCHED_POOL, &param),-1,69);//move child2 to level 1
                //state: level 0: child1->child3 ,  level 1: child2
                ASSERT_EQUAL(search_pool_level(child1,0),0,71);
                ASSERT_EQUAL(search_pool_level(child2,1),0,72);
                ASSERT_EQUAL(search_pool_level(child3,0),1,73);
                waitpid(child3,0,0);//waiting for child3 to die
                //printf("\nfather stoped to wait for child3\n");
                //inorder to verify that exit remove from pool
                ASSERT_EQUAL(search_pool_level(child3,50),-1,78);//child 3 should be dead and therefore not in the pool
                waitpid(child2,0,0);
                waitpid(child1,0,0);
                //printf("all children should be dead");
                //pool should now be empty
                 for(i=0;i<140;++i){
                     ASSERT_EQUAL(search_pool_level(child1,i),-1,84);//child 3 should be dead and therefore not in the pool
                     ASSERT_EQUAL(search_pool_level(child2,i),-1,85);//child 3 should be dead and therefore not in the pool
                }
            }else{//third son
                child3=getpid();
                sleep(3);
                //print_pool_level(0); 
                //printf("\nchild 3 is now running\n");
                int grandson_of_3=fork();
                if(grandson_of_3==0){
                    printf("\ngrandson_of_3 is now running\n");
                    int g3pid= getpid();
                   // printf("child1=%d child2=%d child3=%d g3=%d",child1,child2,child3,g3pid);

                   param.sched_priority = 50;
                   ASSERT_DIFFERENT(sched_setscheduler(child3, SCHED_POOL, &param),-1,99);//moving our father to 50
                   sleep(2);                
                   //printf("\ngrandson3 should now die\n");
                   exit(0);
                }else{
                    //print_pool_level(0); 
                    int child3_index=search_pool_level(child3,0);
                    printf("\n child3 index: %d\n",child3_index);
                    ASSERT_DIFFERENT(child3_index,-1,107);//checking that we found ourself where we left
                   // ASSERT_EQUAL(child3_index,1,108);//child3 should be placed at level 0 index 1
                    ASSERT_EQUAL(search_pool_level(grandson_of_3,0),child3_index+1,109);//checking that grandson is just after us(after fork)
                   waitpid(grandson_of_3,NULL,0);//wait until third son child will die (g3pid)
                   //printf("child3 stopped waiting to his son");
                   //print_pool_level(50);
                   ASSERT_EQUAL(search_pool_level(child3,50),0,113);// our son moved us to 50 before he died
                   //printf("\nchild3 should now die\n");
                   exit(0);
                }
            }
        }else{
            sleep(7);
            printf("\nchild2 is going to die\n");
            exit(0);
        }
    }else{
        sleep(7);
        printf("\nchild1 is going to die\n");
        exit(0);
    }

	return true;
}

int main() {
	int pid = getpid();

	RUN_TEST("general_pool_prio_check", general_pool_prio_check());

	return 0;
}
