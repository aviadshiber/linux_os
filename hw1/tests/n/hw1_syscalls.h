#ifndef HW1_WRAPPER
#define HW1_WRAPPER

#include <errno.h>
#include <termios.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct syscall_log_info{
int syscall_num;
int syscall_res;
int time;
} syscall_log_info;

int enable_syscalls_logging(pid_t  pid, int size) {
	int __res;
	__asm__(
		"int $0x80;"
		: "=a" (__res)
		: "0" (243), "b" (pid), "c"(size)
		:"memory"
		);
	if ((__res) < 0) {
		errno = (-__res); 
		return -1;
	}
	return __res;
}

int disable_syscalls_logging(pid_t  pid){
    int __res;
	__asm__(
		"int $0x80;"
		: "=a" (__res)
		: "0" (244), "b" (pid)
		:"memory"
		);
	if ((__res) < 0) {
		errno = (-__res); 
		return -1;
	}
	return __res;
}

int get_syscalls_log(pid_t  pid, int size, syscall_log_info* user_mem){
     int __res;
	__asm__(
		"int $0x80;"
		: "=a" (__res)
		: "0" (245), "b" (pid), "c" (size), "d" (user_mem)
		:"memory"
		);
	if ((__res) < 0) {
		errno = (-__res); 
		return -1;
	}
	return __res;
}
#endif
