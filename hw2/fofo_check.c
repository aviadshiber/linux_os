#include "hw2_syscalls.h"
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
 #include <sys/wait.h>
 #include <assert.h>
 #include <sched.h>
 #include <stdio.h>

 int main(){
    int pid=getpid();
    get_remaining_timeslice(pid);
    get_total_processor_usage(pid);
    get_total_time_in_runqueue(pid);
    sacrifice_timeslice(pid);
    search_pool_level(pid,0);
     return 0;
 }