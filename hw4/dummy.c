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
    int fdTemplate = open("/dev/syscals_logger", O_RDWR);
    int id=fork();
    if(id!=0){
        wait(NULL);
    }
    return 0;
}
