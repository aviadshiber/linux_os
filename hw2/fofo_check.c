#include "hw2_syscalls.h"
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
 #include <sys/wait.h>
 #include <assert.h>
#define ASSERT_TIME(DELTA,RESULT) assert(delta-total_time_rq<=300)

int main(){
    time_t start=time(NULL);
   
    pid_t f=fork();
    if(f==0){
        pid_t current_pid=getpid();
        sleep(1000);
        unsigned long total_time_rq=get_total_time_in_runqueue(current_pid);
        time_t end=time(NULL);
        unsigned long delta= end-start;
        ASSERT_TIME(delta,total_time_rq);
    }else{
        unsigned long fork_start=time(NULL);
        wait(NULL);
        unsigned long fork_end=time(NULL);
        unsigned long usage=get_total_processor_usage(f);
        unsigned long delta=fork_end-fork_start;
        ASSERT_TIME(delta,usage);
    }
    // get_remaining_timeslice(0);
    // get_total_processor_usage(0);
  
    // sacrifice_timeslice(0);
return 0;
}

