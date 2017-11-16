#include <linux/sched.h>
void add_log(int syscall_result,int syscall_number);


void add_log(int syscall_result,int syscall_number){

    if(syscall_number<243)
    return;
   printk("syscall num:%d returned:%d",syscall_number,syscall_result);
}


