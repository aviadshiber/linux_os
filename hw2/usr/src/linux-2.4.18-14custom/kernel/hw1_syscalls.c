#include <linux/sched.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

int sys_get_remaining_timeslice(pid_t pid);
int sys_get_total_processor_usage(pid_t pid);
int sys_get_total_time_in_runqueue(pid_t pid);
int sys_sacrifice_timeslice(pid_t pid);
int sys_search_pool_level(pid_t pid,int level);

int sys_get_remaining_timeslice(pid_t pid){
printk("\n sys_get_remaining_timeslice called \n");
 return 0;
 
}

int sys_get_total_processor_usage(pid_t pid){
    printk("\n sys_get_total_processor_usage called \n");
     return 0;
}

int sys_get_total_time_in_runqueue(pid_t pid){
     printk("\n sys_get_total_time_in_runqueue called \n");
      return 0;
}

int sys_sacrifice_timeslice(pid_t pid){
     printk("\n sys_sacrifice_timeslice called \n");
      return 0;
}

int sys_search_pool_level(pid_t pid,int level){
     printk("\n sys_search_pool_level called \n");
     return 0;
}
