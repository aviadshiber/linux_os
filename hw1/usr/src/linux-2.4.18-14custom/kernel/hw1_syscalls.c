
#include <linux/sched.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

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
    found_task->logger_enabled=0;
    found_task->logger_max_size=0;
    found_task->logger_next_log_index=0;

    return 0;
}


int sys_get_syscalls_log(pid_t pid, int size, syscall_log_info* user_mem){
     printk("get is working\n");
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
    if(size=0){ //nothing to do here
        return 0;
    }
    if(copy_to_user(user_mem,found_task->logger_queue,size * sizeof(*found_task->logger_queue) )){ //return 0 on sucess
        return -ENOMEM;
    }

    //here should be the logic of cyclic array

    return 0;
}

