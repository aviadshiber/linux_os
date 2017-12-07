
#include <linux/sched.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

void move_elements(task_t* found_task,int size);
int sys_enable_syscalls_logging(pid_t pid, int size);
int sys_disable_syscalls_logging(pid_t pid);
int sys_get_syscalls_log(pid_t pid, int size, syscall_log_info* user_mem);

int sys_enable_syscalls_logging(pid_t pid, int size){
    if(pid<0){
        return -ESRCH;
    }
    task_t* found_task=find_task_by_pid(pid);
    if(!found_task){
        return -ESRCH;
    }
    if(size<0 || found_task->logger_enabled){
        return -EINVAL;
    }

    found_task->logger_queue=kmalloc(sizeof(*found_task->logger_queue)*size,GFP_KERNEL);
    if(!found_task->logger_queue){
        return -ENOMEM;
    }

    found_task->logger_enabled=1;
    found_task->logger_max_size=size;
    found_task->logger_next_log_index=0;
    return 0;
}

int sys_disable_syscalls_logging(pid_t pid){
     printk("disable is working\n");
    if(pid<0){
        return -ESRCH;
    }
    task_t* found_task=find_task_by_pid(pid);
    if(!found_task){
        return -ESRCH;
    }
    if(!found_task->logger_enabled){
        return -EINVAL;
    }

    kfree(found_task->logger_queue);
    found_task->logger_queue=NULL; //overkill- should never access logger queue when it is not enabled
    found_task->logger_enabled=0;
    found_task->logger_max_size=0;
    found_task->logger_next_log_index=0;

    return 0;
}


int sys_get_syscalls_log(pid_t pid, int size, syscall_log_info* user_mem){
    if(pid<0){
        return -ESRCH;
    }
    task_t* found_task=find_task_by_pid(pid);
    if(!found_task){
        return -ESRCH;
    }
    if( size > found_task->logger_next_log_index || size < 0 || !found_task->logger_enabled || !user_mem){
        return -EINVAL;
    }
    if( 0==size ){ //nothing to do here
        return 0;
    }
    if(copy_to_user(user_mem,found_task->logger_queue,size * sizeof(*found_task->logger_queue) )){ //return 0 on sucess
        return -ENOMEM;
    }

    //here should be the logic of cyclic array
    move_elements(found_task,size);
   
    return 0;
}


void move_elements(task_t* found_task,int size){
      int i;
      int remainingElements= found_task->logger_next_log_index - size;
      for(i=0;i<remainingElements;i++){
          found_task->logger_queue[i]=found_task->logger_queue[i+size];
      }
      found_task->logger_next_log_index-=size;
}
