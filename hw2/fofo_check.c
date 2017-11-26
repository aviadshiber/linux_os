#include "hw2_syscalls.h"
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
 #include <sys/wait.h>
 #include <assert.h>
 #include <sched.h>

#define ASSERT_TIME(DELTA,RESULT) assert(delta-total_time_rq<=300)

int main(){
    struct sched_param param;
    param.sched_priority =1;
    assert(search_pool_level(1,-1)==-1);
    assert(search_pool_level(1,140)==-1);
    pid_t f=fork();
    if(f==0){
        int pid=getpid();
        assert(sched_setscheduler(pid,3,&param)==0);////????????????
        assert(search_pool_level(pid,1)==0);
        assert(search_pool_level(pid+1,1)==-1);
        assert(search_pool_level(pid,0)==-1);
        assert(search_pool_level(pid,2)==-1);
        pid_t f2=fork();
        if(f2==0){
            int pid2=getpid();
            assert(sched_setscheduler(pid2,3,&param)==0);
            assert(search_pool_level(pid,1)==0);
            assert(search_pool_level(pid2,1)==1);
        }
    }
    // time_t start=time(NULL);
   
    // pid_t f=fork();
    // if(f==0){
    //     pid_t current_pid=getpid();
    //     sleep(1000);
    //     unsigned long total_time_rq=get_total_time_in_runqueue(current_pid);
    //     time_t end=time(NULL);
    //     unsigned long delta= end-start;
    //     ASSERT_TIME(delta,total_time_rq);
    // }else{
    //     unsigned long fork_start=time(NULL);
    //     wait(NULL);
    //     unsigned long fork_end=time(NULL);
    //     unsigned long usage=get_total_processor_usage(f);
    //     unsigned long delta=fork_end-fork_start;
    //     ASSERT_TIME(delta,usage);
    // }
    // get_remaining_timeslice(0);
    // get_total_processor_usage(0);
  
    // sacrifice_timeslice(0);
return 0;
}

