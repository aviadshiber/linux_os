#ifndef HW2_WRAPPER
#define HW2_WRAPPER

#include <errno.h>
#include <termios.h>
#include <sys/types.h>
#include <unistd.h>


int get_remaining_timeslice(pid_t pid){
    int __res;
	__asm__(
		"int $0x80;"
		: "=a" (__res)
		: "0" (246), "b" (pid)
		:"memory"
		);
	if ((__res) < 0) {
		errno = (-__res); 
		return -1;
	}
	return __res;
}


int get_total_processor_usage(pid_t pid){
     int __res;
	__asm__(
		"int $0x80;"
		: "=a" (__res)
		: "0" (247), "b" (pid)
		:"memory"
		);
	if ((__res) < 0) {
		errno = (-__res); 
		return -1;
	}
	return __res;
}

int get_total_time_in_runqueue(pid_t pid){
      int __res;
	__asm__(
		"int $0x80;"
		: "=a" (__res)
		: "0" (248), "b" (pid)
		:"memory"
		);
	if ((__res) < 0) {
		errno = (-__res); 
		return -1;
	}
	return __res;
}


int sacrifice_timeslice(pid_t pid){
          int __res;
	__asm__(
		"int $0x80;"
		: "=a" (__res)
		: "0" (249), "b" (pid)
		:"memory"
		);
	if ((__res) < 0) {
		errno = (-__res); 
		return -1;
	}
	return __res;
}

#endif
