#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "tests_macros.h"

// syscall nums without params
#define calls 0,13,20,24,96,132,157
#define CALLS_COUNT 7
#define MAX_LOG 1024
#define NORMAL_S 10
#define MEDIUM_S 500
#define LARGE_S 1000
#define DEVICE_PATH ("/dev/systemcalls_logger")
#define DEVICE_NAME ("systemcalls_logger")
#define forr(ite,stop) for(int ite = 0; ite < stop; ite++)
#define RUN_TEST_LOGS(cmd,count) printf("[%d logs]\n", count);\
                                RUN_ARG_TEST(cmd,count)
#define SON(son) if (!son)


int print = 0;

typedef struct log {
    int sys_id;
    int time;
    int timeslice;
} log;

/*
 * This function performs the system call associated with the given number
 * @param num - the syscall num
 * @param current_log - the log to update
 * @param count - the current size of the log
 */
void translate(int num){
    int __res;
    __asm__(
    "int $0x80;"
    : "=a" (__res)
    : "0" (num)
    :"memory"
    );
}

int* createRandomCallList(int n){
    int calls_list[CALLS_COUNT] = {calls};
    int* res = (int*) malloc(sizeof(int)*n);
    forr(i,n){
        res[i] = calls_list[rand() % CALLS_COUNT];
    }

    return res;
}

int test1(){
    int fd = open(DEVICE_PATH, 0);
    ASSERT_NEQ(fd, -1);
    ASSERT_ZERO(close(fd));

    return 1;
}

bool compare_logs(log* a, log* b, int n_a, int n_b){
    if (n_a != n_b) return 0;

    forr(i,n_a){
        if (print) printf("a: %d, b: %d\n", a[i].sys_id, b[i].sys_id);
        if (a[i].sys_id != b[i].sys_id) return 0;
    }

    return 1;
}

log * generateTrueLog(int * in_calls, int n){
    log * res = (log*) malloc(sizeof(log)*n);
    forr(i,n){
        res[i].timeslice = 1;
        res[i].time = 1;
        res[i].sys_id = in_calls[i];
    }

    return res;
}

int test_size(int size){
    int* tmp_calls = createRandomCallList(size);
    log buf[size];

    int fd = open(DEVICE_PATH, 0);
    // These are the only syscalls we call while the file is open
    forr(i,size){
        translate(tmp_calls[i]);
    }
    ASSERT_EQ(read(fd, buf,size), size);

    ASSERT_ZERO(close(fd));
    log * true_calls = generateTrueLog(tmp_calls, size);

    int test = (1 == compare_logs(true_calls, buf, size,size));

    free(tmp_calls);
    free(true_calls);
    ASSERT_EQ(1,test);
    ASSERT_EQ(-1, read(fd, buf,size));

    return true;
}

int moreThanLogged(){
    int* tmp_calls = createRandomCallList(NORMAL_S);
    log buf[MAX_LOG];
    log true_with_read[NORMAL_S+1];

    int fd = open(DEVICE_PATH, 0);
    // These are the only syscalls we call while the file is open
    forr(i,NORMAL_S){
        translate(tmp_calls[i]);
    }
    // Try to read more than logged (logged)
    ASSERT_EQ(read(fd, buf,LARGE_S), NORMAL_S+1);

    ASSERT_ZERO(close(fd));
    log * true_calls = generateTrueLog(tmp_calls, NORMAL_S);
    // adding the Read syscall
    memcpy(true_with_read, true_calls, sizeof(log)*NORMAL_S);
    true_with_read[NORMAL_S].sys_id = 3;

    int test = (1 == compare_logs(true_with_read, buf, NORMAL_S+1,NORMAL_S+1));

    free(tmp_calls);
    free(true_calls);
    ASSERT_EQ(1,test);
    ASSERT_EQ(-1, read(fd, buf,MAX_LOG));

    return true;
}

int lessThanLogged(){
    int* tmp_calls = createRandomCallList(MEDIUM_S);
    log buf[NORMAL_S];

    int fd = open(DEVICE_PATH, 0);
    // These are the only syscalls we call while the file is open
    forr(i,NORMAL_S){
        translate(tmp_calls[i]);
    }
    // Try to read more than logged (logged)
    ASSERT_EQ(read(fd, buf,NORMAL_S), NORMAL_S);

    ASSERT_ZERO(close(fd));
    log * true_calls = generateTrueLog(tmp_calls, NORMAL_S);

    int test = (1 == compare_logs(true_calls, buf, NORMAL_S,NORMAL_S));

    free(tmp_calls);
    free(true_calls);
    ASSERT_EQ(1,test);
    ASSERT_EQ(-1, read(fd, buf,MAX_LOG));

    return true;
}

int forkingLog(){
    int son = fork();

    SON(son){
        // Let the son log some syscalls
        int* tmp_calls = createRandomCallList(MEDIUM_S);
        int fds = open(DEVICE_PATH, 0);
        forr(i,NORMAL_S){
            translate(tmp_calls[i]);
        }
        // not closing. will close when son dies
        //close(fds);
        free(tmp_calls);
        exit(0);
    } else{
        int fdf = open(DEVICE_PATH, 0);
        log fbuf[2];
        wait(NULL);
        // Only performed waitpid and read
        ASSERT_EQ(read(fdf, fbuf,MAX_LOG), 2);
        ASSERT_EQ(fbuf[0].sys_id,114);
        ASSERT_EQ(fbuf[1].sys_id,3);
        close(fdf);
    }

    return true;
}

#define ITERATIONS 15
int main(int argc, char *argv[]) {
    int tmp;
    if (argc == 2 && 0 == strcmp(argv[1],"y")) print = 1;

    RUN_TEST(test1);
    RUN_TEST_LOGS(test_size,NORMAL_S);
    RUN_TEST_LOGS(test_size,MEDIUM_S);
    RUN_TEST_LOGS(test_size,LARGE_S);
    RUN_TEST(moreThanLogged);
    RUN_TEST(lessThanLogged);
    RUN_TEST(forkingLog);

    printf(CYAN_TEXT("~~~~~~~~~~~~~~~~~~Running random tests~~~~~~~~~~~~~~~~~~\n"));

    forr(i, ITERATIONS){
        tmp = rand() % MAX_LOG;
        RUN_TEST_LOGS(test_size,tmp);
    }




    /*
     * Check if the log isn't erased between tests
     * that doesn't close the file
     * */

    return 0;
}
