#include "hw2_syscalls.h"
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
 #include <sys/wait.h>
 #include <assert.h>
 #include <sched.h>
 #include <stdio.h>

#define ASSERT_TIME(DELTA,RESULT) assert(delta-RESULT<=300)
#define N 100000000

int do_some_work(int num){
    int array[N];
    int i;
    int sum=0;
    for(i=0;i<num;i++){
        sum+=array[i];
    }
    for(i=num-1;i>0;i--){
        sum+=array[i];
    }
    return sum;
}

void test_total_cpu_usage(){
    // int pid=getpid();
    // sleep(3);
    // printf("\nusage time: %d\n",get_total_processor_usage(0));
    // printf("\nusage time: %d\n",get_total_processor_usage(1));
    // sleep(3);
    // printf("\nusage time: %d\n",get_total_processor_usage(1));
    // sleep(3);
    // printf("\nusage time: %d\n",get_total_processor_usage(0));
    // printf("\nusage time: %d\n",get_total_processor_usage(1));
    // printf("\nusage time: %d\n",get_total_processor_usage(pid));
    pid_t son=fork();
    if(son==0){
        int sum;
        int sonpid=getpid();
        int usage;
        // int usage=get_total_processor_usage(sonpid); 
        // printf("\n son's first usage time: %d\n",usage);
        // usage=get_total_processor_usage(sonpid); 
        // printf("\n son's first222222 usage time: %d\n",usage);
        unsigned long son_start=time(NULL);
        printf("\n son's starting time: %lu\n",son_start);
        sum=do_some_work(N);
        unsigned long son_end=time(NULL);
        printf("\n son's ending time: %lu\n",son_end);
        unsigned long delta=son_end-son_start;
        printf("\n son's delta time: %lu\n",delta);
        usage=get_total_processor_usage(sonpid); //we do not get the right time here
         printf("\n son's usage time1: %d\n",usage);
    }else{
        //int usage=get_total_processor_usage(son); 
        //printf("\n system time: %lu\n",time(NULL));
        //printf("\n son usage time2: %d\n",usage);
        unsigned long fork_start=time(NULL);
        printf("\n fork start time: %lu\n",fork_start);
        wait(NULL);
        unsigned long fork_end=time(NULL);
        printf("\n fork end time: %lu\n",fork_end);
        int usage=get_total_processor_usage(son);   //we do not get the right time here
        //printf("\n system time: %lu\n",time(NULL));
        printf("\n son usage time3: %d\n",usage);
        unsigned long delta=fork_end-fork_start;
        printf("\n father's delta time: %lu\n",delta);
        ASSERT_TIME(delta,usage);
    }
    // printf("\n system time: %lu\n",time(NULL));
    // printf("\n son usage time4: %d\n",get_total_processor_usage(son));
    // printf("\n init usage time: %d\n",get_total_processor_usage(1));
}



//what happens when we try to add the same pid to other level in pool? or same place in pool?
//what happens to a task pool when its dead?
void test_pool(){
    struct sched_param param;
        param.sched_priority =1;
        assert(search_pool_level(1,-1)==-1);        //failure level
        assert(search_pool_level(1,140)==-1);       //failure level
        int pid2copy;
        pid_t f=fork();
        if(f==0){
            int pid=getpid();
            assert(sched_setscheduler(pid,3,&param)==0);    //success
            assert(search_pool_level(pid,1)==0);            //success
            assert(search_pool_level(pid+1,1)==-1);         //failure
            assert(search_pool_level(pid,0)==-1);           //failure
            assert(search_pool_level(pid,2)==-1);           //failure
            pid_t f2=fork();
            if(f2==0){
                int pid2=getpid();
                assert(sched_setscheduler(pid2,3,&param)==0);   //success
                assert(search_pool_level(pid,1)==0);            //success
                assert(search_pool_level(pid2,1)==1);           //success
                param.sched_priority =0;
                assert(sched_setscheduler(pid,0,&param)==0);    //success sched other
                assert(search_pool_level(pid,1)==-1);            //failure - we moved it
                assert(search_pool_level(pid2,1)==1);           //success
                assert(sched_setscheduler(pid2,0,&param)==0);    //success sched other
                assert(search_pool_level(pid2,1)==-1);           //failure - we moved it
                assert(sched_setscheduler(pid2,3,&param)==0);    //success sched pool (0)
                assert(search_pool_level(pid2,0)==0);           //success
                pid2copy=pid2;
            }
        }
        //assert(search_pool_level(pid2copy,0)==0);           //success ??
}

int main(){
    

    test_total_cpu_usage();
    //test_pool();
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

