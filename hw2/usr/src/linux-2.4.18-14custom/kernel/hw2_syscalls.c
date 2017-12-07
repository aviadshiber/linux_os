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
    if(current->policy == SCHED_FIFO){          //invoking process policy is fifo
          return -EPERM;
    }
    if(current->pid == pid || found_task->policy == SCHED_FIFO){
          return -EINVAL;
    }
    unsigned int timeSliceSacraficed = current->time_slice;
    current->was_sacraficed=1;
    found_task->time_slice+=timeSliceSacraficed;
    current->time_slice=0;
    return timeSliceSacraficed;
}

int sys_search_pool_level(pid_t pid,int level){
     printk("\n sys_search_pool_level called \n");
     return 0;
}
