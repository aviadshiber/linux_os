#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sched.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>


int main(){
    int fdTemplate = open("/dev/syscalls_logger", O_RDWR);
    int fdTemplate2 = open("/dev/syscalls_logger", O_RDWR);
    printf("open1=%d\n",fdTemplate);
    printf("open2=%d\n",fdTemplate2);
    int id=fork();
    if(id!=0){
        wait(NULL);
    }
    return 0;
}
