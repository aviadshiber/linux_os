#include "hw2_syscalls.h"
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
 #include <sys/wait.h>
 #include <assert.h>
 #include <sched.h>
 #include <stdio.h>

 #define SCHED_POOL 3



int testPool(){
    struct sched_param param;
     param.sched_priority=0;
    int pid1=getpid();
    print_pool_level(0);
    sched_setscheduler(pid1,SCHED_POOL,&param);
    print_pool_level(0);
    int son1=fork();
    int son2=fork();
    if(son1 != 0 && son2 != 0 ){
    //      int i;
          sched_setscheduler(son1,SCHED_POOL,&param);
          print_pool_level(0);
    //      assert(search_pool_level(son1,8)==1);  
        // printf("\n %d is in the first place now",son1);
        // sched_setscheduler(son2,SCHED_POOL,&param); // now the pool array at level 8 should look like:8: (son1)->(son2)
        // printf("\n %d is in the second second now",son2);
        //     assert(search_pool_level(-1,0)==-1);
        //     for(i=0;i<140;i++){
        //         assert(search_pool_level(pid1,i)==-1);      //father is not in the pool
        //     }
        //     printf("\n father is not in the pool");
        //     assert(search_pool_level(son1,-1)==-1);
        //     assert(search_pool_level(son1,140)==-1);
        //     assert(search_pool_level(son1,7)==-1);
        //     assert(search_pool_level(son1,9)==-1);  
        //     printf("\n errors returns are ok");
        //     assert(search_pool_level(son2,8)==1);           // son2 is in the 2th place
        //     param.sched_priority=7;
        //     sched_setscheduler(son1,SCHED_POOL,&param);     //level 7 : (son1)
        //     printf("\n son1 moved to level 7");
        //     assert(search_pool_level(son2,8)==0);
        //     assert(search_pool_level(son1,7)==0);
        //     param.sched_priority=8;
        //     sched_setscheduler(son1,SCHED_POOL,&param); 
        //     printf("\n son1 moved to back to level 8");
        //     assert(search_pool_level(son2,8)==0);
        //     assert(search_pool_level(son1,8)==1);
     }
        // int status2=fork();
        // if(status2 == 0){        //grandson's code
        //     int pid3=getpid();
        //     sched_setscheduler(pid3,3,&param);        
        //     printf("\n %d is in the first second now",pid2);     
        //         //all should fail until the next line
        //     assert(search_pool_level(pid3,8)==1);           // pid3 is in the 2th place
        //     sched_setscheduler(pid2,2,&param);            //chaning pid2 policy - level 8: (pid3)
        //     assert(search_pool_level(pid3,8)==0);       //checking that pid3 moved to 1th place
        //     sched_setscheduler(pid2,3,&param); 
        //     param.sched_priority=7;
        //     sched_setscheduler(pid3,3,&param);            //7: (pid3)
        //     assert(search_pool_level(pid3,8)==-1);      //we are checking that pid3 has been deleted from 8 level
        //     assert(search_pool_level(pid2,8)==0);          //8: (pid2)->
        //     assert(search_pool_level(pid3,7)==0);       //moved to his new level 7
        //     sched_setscheduler(pid2,3,&param);            //7: (pid3)->(pid2)
        //     assert(search_pool_level(pid3,7)==0);
        //     assert(search_pool_level(pid2,7)==1);
        //     assert(search_pool_level(pid2+1,7)==-1);        //should not be in here
        //}
    // }else{
    //    while(1){};
    // }
    return 0;
}





 int main(){
     testPool();
     //print_pool_level(0);
    // int pid=getpid();
    // int timeslice=get_remaining_timeslice(pid);
    // printf("\n father process number %d timeslice is %d\n",pid,timeslice);
    // int total_cpu_usage=get_total_processor_usage(pid);
    // printf("\n father process number %d cpu usage is %d\n",pid,total_cpu_usage);
    // int total_rq_time=get_total_time_in_runqueue(pid);
    // printf("\n father process number %d rq time is %d\n",pid,total_rq_time);
    // int status;
    // status=fork();
    //  if(status ==0){     //son's code
    //     int sonpid=getpid();
    //     int sontimeslice=get_remaining_timeslice(sonpid);       //getting son's timeslice
    //     printf("\n son process number %d timeslice is %d\n",sonpid,sontimeslice);
    // //     int sac_time=sacrifice_timeslice(pid);                   //son sacrifice it's timeslice to main process
    // //     printf("\n son process number %d sacraficed %d time to father(%d) \n",sonpid,sac_time,pid);
    // //     int newtimeslice=get_remaining_timeslice(pid);          //checking the main process new timeslice
    // //     printf("\n father process number %d new timeslice is %d\n",pid,newtimeslice);
    //  }
    // // printf("\n process number %d timeslice is %d\n",0,get_remaining_timeslice(0));
    // // printf("\n process number %d timeslice is %d\n",1,get_remaining_timeslice(1));
    // search_pool_level(pid,0);
     return 0;
 }
