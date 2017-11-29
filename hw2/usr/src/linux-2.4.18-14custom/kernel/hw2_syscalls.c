
#include <linux/sched.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

int sys_get_remaining_timeslice(pid_t pid){
    if(pid<0){
        return -ESRCH;
    }
    task_t* found_task=find_task_by_pid(pid);
    if(!found_task){
        return -ESRCH;
    }
    if(SCHED_FIFO == found_task->policy ){
        return -EINVAL;
    }
    //should we return zero timeslice on zombie?
    if(found_task->state==TASK_ZOMBIE){
        return 0;
    }
    if(SCHED_POOL == found_task->policy){
        //return our_time_pool;
    }

    return found_task->time_slice;
}

int sys_get_total_processor_usage(pid_t pid){
     if(pid<0){
        return -ESRCH;
    }
    task_t* found_task=find_task_by_pid(pid);
    if(!found_task){
        return -ESRCH;
    }
    unsigned long total = found_task->total_processor_usage_time;
    return (int)total;
}

int sys_get_total_time_in_runqueue(pid_t pid){
    if(pid<0){
        return -ESRCH;
    }
    task_t* found_task=find_task_by_pid(pid);
    if(!found_task){
        return -ESRCH;
    }
    if(TASK_RUNNING == found_task->state){ //to make sure entered_to_rq_time!=0 so the delta will be realistic
        found_task->total_time_in_runqueue+=jiffies- found_task->entered_to_rq_time;
        found_task->entered_to_rq_time=jiffies;
    }
    return found_task->total_time_in_runqueue;
}





