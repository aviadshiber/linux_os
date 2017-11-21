
#include <linux/sched.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

int sys_get_remaining_timeslice(pid_t pid){
printk("\n sys_get_remaining_timeslice called \n");
//TODO: RETURN REMANING TIME SLICE
return 0;
}

int sys_get_total_processor_usage(pid_t pid){
printk("\n sys_get_total_processor_usage called \n");
//TODO: RETURN REMANING TIME SLICE
return 0;
}

int sys_get_total_time_in_runqueue(pid_t pid){
printk("\n sys_get_total_time_in_runqueue called \n");
//TODO: RETURN total amound of time
    return 0;
}

int sys_sacrifice_timeslice(pid_t pid){
printk("\n sys_sacrifice_timeslice called \n");
    //TODO: return amound of time sacrficed time
    return 0;
}


