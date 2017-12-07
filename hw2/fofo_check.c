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
    int timeslice=get_remaining_timeslice(pid);
    printf("\n father process number %d timeslice is %d\n",pid,timeslice);
    int total_cpu_usage=get_total_processor_usage(pid);
    printf("\n father process number %d cpu usage is %d\n",pid,total_cpu_usage);
    int total_rq_time=get_total_time_in_runqueue(pid);
    printf("\n father process number %d rq time is %d\n",pid,total_rq_time);
    int status;
    status=fork();
     if(status ==0){     //son's code
        int sonpid=getpid();
        int sontimeslice=get_remaining_timeslice(sonpid);       //getting son's timeslice
        printf("\n son process number %d timeslice is %d\n",sonpid,sontimeslice);
    //     int sac_time=sacrifice_timeslice(pid);                   //son sacrifice it's timeslice to main process
    //     printf("\n son process number %d sacraficed %d time to father(%d) \n",sonpid,sac_time,pid);
    //     int newtimeslice=get_remaining_timeslice(pid);          //checking the main process new timeslice
    //     printf("\n father process number %d new timeslice is %d\n",pid,newtimeslice);
     }
    // printf("\n process number %d timeslice is %d\n",0,get_remaining_timeslice(0));
    // printf("\n process number %d timeslice is %d\n",1,get_remaining_timeslice(1));
    search_pool_level(pid,0);
     return 0;
 }
