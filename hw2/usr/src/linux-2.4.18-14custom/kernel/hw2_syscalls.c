#include <linux/sched.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

int sys_get_remaining_timeslice(pid_t pid){
      printk("\n sys_get_remaining_timeslice called \n");
      if(pid<0){
        return -ESRCH;
      }
      task_t* found_task=find_task_by_pid(pid);
      if(!found_task){
        return -ESRCH;
      }
      if(found_task->policy == SCHED_FIFO){           //process has no timeslice
            return -EINVAL;
      }
      if(TASK_ZOMBIE == found_task->state ){
             return 0;
      }
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
     if(current->pid == pid || SCHED_FIFO == found_task->policy  || TASK_ZOMBIE==found_task->state ){
          return -EINVAL;
    }
    if(current->policy == SCHED_FIFO){          //invoking process policy is fifo
          return -EPERM;
    }
   
    unsigned int timeSliceSacraficed = current->time_slice;
    current->was_sacraficed=1;
    found_task->time_slice+=timeSliceSacraficed;
    current->time_slice=0;
    return timeSliceSacraficed;
}
