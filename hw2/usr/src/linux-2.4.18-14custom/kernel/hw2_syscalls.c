
#include <linux/sched.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
// task_t* found_task=find_task_by_pid(pid);
int sys_get_remaining_timeslice(pid_t pid){
    printk("\n sys_get_remaining_timeslice called \n");
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
    return found_task->time_slice;
}

int sys_get_total_processor_usage(pid_t pid){
    printk("\n sys_get_total_processor_usage called \n");
     if(pid<0){
        return -ESRCH;
    }
    task_t* found_task=find_task_by_pid(pid);
    if(!found_task){
        return -ESRCH;
    }
    return found_task->total_processor_usage_time;
}

int sys_get_total_time_in_runqueue(pid_t pid){
    printk("\n sys_get_total_time_in_runqueue called \n");
    if(pid<0){
        return -ESRCH;
    }
    task_t* found_task=find_task_by_pid(pid);
    if(!found_task){
        return -ESRCH;
    }
    return found_task->total_time_in_runqueue;
}

int sys_sacrifice_timeslice(pid_t pid){
    printk("\n sys_sacrifice_timeslice called \n");
    if(pid<0){
        return -ESRCH;
    }
    task_t* found_task=find_task_by_pid(pid);
    if(!found_task){
        return -ESRCH;
    }
    if( (pid == current->pid) || (found_task->policy == SCHED_FIFO) || (found_task->state == TASK_ZOMBIE)  ){
        return -EINVAL;
    }
    if( current->policy == SCHED_FIFO ){
        return -EPERM;
    }
    int currentTimeSlice=current->time_slice;
    current->sacrafice=1; //the current_time slice will be nullified in tick according to this flag
    found_task->time_slice+=currentTimeSlice;
    return currentTimeSlice;
}




