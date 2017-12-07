
#include <linux/sched.h>
#include <linux/slab.h>
#include <asm/uaccess.h>


int sys_get_total_processor_usage(pid_t pid){
    //printk("\n sys_get_total_processor_usage called \n");  
     if(pid<0){
        return -ESRCH;
    }
    task_t* found_task=find_task_by_pid(pid);
    if(!found_task){
        return -ESRCH;
    }
    unsigned long total = found_task->total_processor_usage_time;
    // printk("\ntotal_usage: %lu",total);     /////
    // if(total > INT_MAX){                                                    ///////////check added
    //     printk("OVERFLOW\n");           
    // }
    return (int)total;
}

int sys_get_total_time_in_runqueue(pid_t pid){
    //printk("\n sys_get_total_time_in_runqueue called \n");
    if(pid<0){
        return -ESRCH;
    }
    task_t* found_task=find_task_by_pid(pid);
    if(!found_task){
        return -ESRCH;
    }
    if(TASK_RUNNING == found_task->state){
        found_task->total_time_in_runqueue+= jiffies - found_task->entered_to_rq_time;
        found_task->entered_to_rq_time=jiffies;
    }
    return found_task->total_time_in_runqueue;
}




