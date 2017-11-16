#include <linux/sched.h>

#define LOGGER_IS_FULL current->logger_next_log_index==current->logger_max_size
#define LOGGER_QUEUE current->logger_queue
#define LOGGER_MAX_SIZE current->logger_max_size
void add_log(int syscall_result,int syscall_number);


void add_log(int syscall_result,int syscall_number){
   
    if(current->logger_enabled){
        syscall_log_info logInfo;
        logInfo.time=jiffies;
        loginfo.syscall_num=syscall_number;
        logInfo.syscall_res=syscall_result;

        if(LOGGER_IS_FULL){
            int i;
            for(i=1;i<LOGGER_MAX_SIZE;i++){
                LOGGER_QUEUE[i-1]=LOGGER_QUEUE[i];
            }
            LOGGER_QUEUE[i-1]=logInfo;
        }else{ //logger is not full
            LOGGER_QUEUE[current->logger_next_log_index]=logInfo;
            current->logger_next_log_index++;
        }//end else

    }//end outer if
}


