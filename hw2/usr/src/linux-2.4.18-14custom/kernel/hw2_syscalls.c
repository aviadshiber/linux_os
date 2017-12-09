#include <linux/sched.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

int sys_get_total_processor_usage(pid_t pid){
    printk("\n sys_get_total_processor_usage called \n");
    if(pid<0){
          return -ESRCH;
    }
    task_t* found_task=find_task_by_pid(pid);
    if(!found_task){
      return -ESRCH;
    }
    return found_task->total_proccesor_usage_time;
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
    if(TASK_RUNNING == found_task->state ||  found_task->entered_to_runqueue_time){ // the task it is somewhere in the runqueue
       found_task->total_runqueue_time += (jiffies - found_task->entered_to_runqueue_time);
       found_task->entered_to_runqueue_time=jiffies; // it is still in runqueue so we sample again
    }
    return found_task->total_runqueue_time;
<<<<<<< HEAD
}
=======
}
>>>>>>> 2c89229194689b7662c16772c019964d8a92f7f6
