
#include "hw1_syscalls.h"
#include "staticQueue.hpp"
#include "testsMacros.h"
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sched.h>
#include <sys/wait.h>


int enable_disable_test(){
	struct syscall_log_info buff[10];
	int pid= getpid();
	int result = 1;
	int i=0;
	int res;
	// enable after already invoking enable 
	enable_syscalls_logging(pid, 10);
	enable_syscalls_logging(pid, 10);
	enable_syscalls_logging(pid, 10);
	get_syscalls_log(pid, 3, buff);

	for(i=0 ; i<3 ; ++i){	//sys_call num check
		assert(buff[i].syscall_num == 243);
	}

	//sys_call res check
	assert(buff[0].syscall_res == 0);
	assert(buff[1].syscall_res == -EINVAL);
	assert(buff[2].syscall_res == -EINVAL);

	// disable after already invoking disable
	disable_syscalls_logging(pid);
	res = disable_syscalls_logging(pid);
	assert(res==-1);
	assert(errno==EINVAL);

	// sys_enable_tests
	enable_syscalls_logging(pid, 10);
	res = enable_syscalls_logging(-1, 10); //negative pid
	assert(res==-1);
	assert(errno==ESRCH);
	res = enable_syscalls_logging(33000, 10); //no such proc
	assert(res==-1);
	assert(errno==ESRCH);
	res = enable_syscalls_logging(pid, -1); //negative size
	assert(res==-1);
	assert(errno==EINVAL);
	res = enable_syscalls_logging(-1, -1); //negative pid and size
	assert(res==-1);
	assert(errno==ESRCH);
	get_syscalls_log(pid, 4, buff);
	for(i=0 ; i<4 ; ++i){	//sys_call num check
		assert(buff[i].syscall_num == 243);
	}

	//sys_call res check
	assert(buff[0].syscall_res == 0);
	assert(buff[1].syscall_res == -ESRCH);
	assert(buff[2].syscall_res == -ESRCH);
	assert(buff[3].syscall_res == -EINVAL);
	disable_syscalls_logging(pid);

	//size=0 test
	res = enable_syscalls_logging(pid, 0);
	assert(res==0);
	get_syscalls_log(pid, 0, buff);
	disable_syscalls_logging(pid);

	// sys_disable_tests
	res = disable_syscalls_logging(-1); // negative pid
	assert(res==-1);
	assert(errno==ESRCH);
	res = disable_syscalls_logging(33000); //no such proc
	assert(res==-1);
	assert(errno==ESRCH);
	res = disable_syscalls_logging(pid); //disable after invoking disable
	assert(res==-1);
	assert(errno==EINVAL);

	return result;
}

int get_test(){
	struct syscall_log_info buff[10];
	int pid= getpid();
	int result = 1;
	int i=0;
	int res;

	res = get_syscalls_log(-1, 5, buff); //negative pid
	assert(res==-1);
	assert(errno==ESRCH);
	res = get_syscalls_log(33000, 5, buff); //no such proc
	assert(res==-1);
	assert(errno==ESRCH);
	res = get_syscalls_log(pid, 11, buff); //size>log_size
	assert(res==-1);
	assert(errno==EINVAL);
	res = get_syscalls_log(pid, -1, buff); //negative size
	assert(res==-1);
	assert(errno==EINVAL);
	res = get_syscalls_log(pid, 5, buff); //logging isn't enabled
	assert(res==-1);
	assert(errno==EINVAL);
	res = get_syscalls_log(-1, -1, buff); //negative pid and size
	assert(res==-1);
	assert(errno==ESRCH);
	res = get_syscalls_log(-1, 11, buff); //negative pid and size bigger than log_size
	assert(res==-1);
	assert(errno==ESRCH);

	//not enough logs yet
	enable_syscalls_logging(pid, 10);
	for(i=0; i<5; i++){
		getpid();
	}
	res = get_syscalls_log(pid, 7, buff);
	assert(res==-1);
	assert(errno==EINVAL);

	disable_syscalls_logging(pid);
	//2 systeam calls in a row and get after get should return 1 log record - 245, itself
	enable_syscalls_logging(pid, 4);
	getpid();
	get_syscalls_log(pid, 2, buff);
	assert(buff[0].syscall_num == 243);
	assert(buff[1].syscall_num == 20);
	assert(buff[0].syscall_res == 0);
	assert(buff[1].syscall_res == pid);
	get_syscalls_log(pid, 1, buff);
	assert(buff[0].syscall_num == 245);
	assert(buff[0].syscall_res == 0);

	disable_syscalls_logging(pid);
	//cyclic
	struct syscall_log_info buff_small[4];
	enable_syscalls_logging(pid,4);
	getpid();
	int ppid = getppid(); //64
	enable_syscalls_logging(pid,4);
	enable_syscalls_logging(pid,4);
	get_syscalls_log(pid, 4, buff);
	disable_syscalls_logging(pid);
	assert(buff[0].syscall_num == 20);
	assert(buff[1].syscall_num == 64);
	assert(buff[2].syscall_num == 243);
	assert(buff[3].syscall_num == 243);
	assert(buff[0].syscall_res == pid);
	assert(buff[1].syscall_res == ppid);
	assert(buff[2].syscall_res == -EINVAL);
	assert(buff[3].syscall_res == -EINVAL);
	return result;
}

//enabling logging without disable - exit() check
int enable_no_disable(){
	struct syscall_log_info buff[10];
	int pid= getpid();
	int result = 1;
	int i=0;
	int res;
	enable_syscalls_logging(pid,10);
	return result;
}

int test_fork(){
	struct syscall_log_info buff[4];
	int pid= getpid();
	int result = 1;
	int i=0;
	int res;
	enable_syscalls_logging(pid,4); //243
	getpid(); //20
	int ppid = getppid(); //64
	enable_syscalls_logging(pid,4); //243
	enable_syscalls_logging(pid,4); //243
	int fork_res1 = fork(); //2
	
	
	if(fork_res1==0){
		//first son
		int son1_pid = getpid();
		res = disable_syscalls_logging(son1_pid);
		assert(res==-1);
		assert(errno==EINVAL);
		res = get_syscalls_log(son1_pid, 4, buff);
		assert(res==-1);
		assert(errno==EINVAL);
		enable_syscalls_logging(son1_pid, 4);
		getpid();
		res = get_syscalls_log(son1_pid, 3, buff);
		assert(res==-1);
		assert(errno==EINVAL);
		res = get_syscalls_log(son1_pid, 2, buff);
		assert(res==0);
		disable_syscalls_logging(son1_pid);

		//cyclic
		struct syscall_log_info buff_small[4];
		enable_syscalls_logging(son1_pid,4);
		getpid();
		int s1ppid = getppid(); //64
		enable_syscalls_logging(son1_pid,4);
		enable_syscalls_logging(son1_pid,4);
		get_syscalls_log(son1_pid, 4, buff);
		disable_syscalls_logging(son1_pid);
		assert(buff[0].syscall_num == 20);
		assert(buff[1].syscall_num == 64);
		assert(buff[2].syscall_num == 243);
		assert(buff[3].syscall_num == 243);
		assert(buff[0].syscall_res == son1_pid);
		assert(buff[1].syscall_res == s1ppid);
		assert(buff[2].syscall_res == -EINVAL);
		assert(buff[3].syscall_res == -EINVAL);
		exit(0);
	}

	
	else{
		//back to the father
		waitpid(-1, NULL, 0); //7
		int fork_res2 = fork();
		
		if(fork_res2==0){
			//second son
			int son2_pid = getpid();
			res = disable_syscalls_logging(son2_pid);
			assert(res==-1);
			assert(errno==EINVAL);
			res = get_syscalls_log(son2_pid, 4, buff);
			assert(res==-1);
			assert(errno==EINVAL);
			enable_syscalls_logging(son2_pid, 4);
			getpid();
			res = get_syscalls_log(son2_pid, 3, buff);
			assert(res==-1);
			assert(errno==EINVAL);
			res = get_syscalls_log(son2_pid, 2, buff);
			assert(res==0);
			disable_syscalls_logging(son2_pid);
			exit(0);
		}
		else{
			//back to the father
			waitpid(-1, NULL, 0);
			get_syscalls_log(pid, 4, buff);
			disable_syscalls_logging(pid);
			assert(buff[0].syscall_num == 2);
			//assert(buff[1].syscall_num == 1);
			assert(buff[2].syscall_num == 2);
			//assert(buff[3].syscall_num == 1);
			assert(buff[0].syscall_res == fork_res1);
			//assert(buff[1].syscall_res == -EINVAL);
			assert(buff[2].syscall_res == fork_res2);
			//assert(buff[3].syscall_res != pid);
	
		}
	
	}

	return result;
}

int fork_advanced(){
	//grandfather
	struct syscall_log_info buff[4];
	int pid= getpid();
	int result = 1;
	int i=0;
	int res;
	enable_syscalls_logging(pid,4); //243
	getpid(); //20
	int ppid = getppid(); //64
	enable_syscalls_logging(pid,4); //243
	enable_syscalls_logging(pid,4); //243
	int fork_res1 = fork(); //2
	
	if(fork_res1==0){
		//father
		int son_pid = getpid(); 
		enable_syscalls_logging(son_pid,4); //243
		getpid(); //20
		int father_pid = getppid(); //64
		getpid(); //20
		int another_fork_res = fork(); //2

		if(another_fork_res == 0){
			//first grandson
			int grandson1_pid = getpid();
			res = disable_syscalls_logging(grandson1_pid);
			assert(res==-1);
			assert(errno==EINVAL);
			res = get_syscalls_log(grandson1_pid, 4, buff);
			assert(res==-1);
			assert(errno==EINVAL);
			enable_syscalls_logging(grandson1_pid, 4);
			getpid();
			res = get_syscalls_log(grandson1_pid, 3, buff);
			assert(res==-1);
			assert(errno==EINVAL);
			res = get_syscalls_log(grandson1_pid, 2, buff);
			assert(res==0);
			disable_syscalls_logging(grandson1_pid);

			//cyclic
			struct syscall_log_info buff_small[4];
			enable_syscalls_logging(grandson1_pid,4);
			getpid();
			int s1ppid = getppid(); //64
			enable_syscalls_logging(grandson1_pid,4);
			enable_syscalls_logging(grandson1_pid,4);
			get_syscalls_log(grandson1_pid, 4, buff);
			disable_syscalls_logging(grandson1_pid);
			assert(buff[0].syscall_num == 20);
			assert(buff[1].syscall_num == 64);
			assert(buff[2].syscall_num == 243);
			assert(buff[3].syscall_num == 243);
			assert(buff[0].syscall_res == grandson1_pid);
			assert(buff[1].syscall_res == s1ppid);
			assert(buff[2].syscall_res == -EINVAL);
			assert(buff[3].syscall_res == -EINVAL);
			exit(0);
		} else{
			//father
			waitpid(-1, NULL, 0); //7
			get_syscalls_log(son_pid, 4, buff);
			assert(buff[0].syscall_num == 64);
			assert(buff[1].syscall_num == 20);
			assert(buff[2].syscall_num == 2);
			//assert(buff[3].syscall_num == 1);
			assert(buff[0].syscall_res == father_pid);
			assert(buff[1].syscall_res == son_pid);
			assert(buff[2].syscall_res == another_fork_res);
			//assert(buff[3].syscall_res == -EINVAL);
			int fork_res2 = fork();
			if(fork_res2==0){
				//second grandson
				int grandson2_pid = getpid();
				res = disable_syscalls_logging(grandson2_pid);
				assert(res==-1);
				assert(errno==EINVAL);
				res = get_syscalls_log(grandson2_pid, 4, buff);
				assert(res==-1);
				assert(errno==EINVAL);
				enable_syscalls_logging(grandson2_pid, 4);
				getpid();
				res = get_syscalls_log(grandson2_pid, 3, buff);
				assert(res==-1);
				assert(errno==EINVAL);
				res = get_syscalls_log(grandson2_pid, 2, buff);
				assert(res==0);
				disable_syscalls_logging(grandson2_pid);
				exit(0);
			} else {
				//father
				waitpid(-1, NULL, 0); //7
				get_syscalls_log(son_pid, 3, buff);
				assert(buff[0].syscall_num == 245);
				assert(buff[1].syscall_num == 2);
				//assert(buff[2].syscall_num == 1);
				assert(buff[0].syscall_res == 0);
				assert(buff[1].syscall_res == fork_res2);
				//assert(buff[2].syscall_res == 0);
				disable_syscalls_logging(son_pid);
			}
		}
	
	} else {
		//grandfather
		waitpid(-1, NULL, 0); //7
		get_syscalls_log(pid, 4, buff);
		disable_syscalls_logging(pid);
		assert(buff[0].syscall_num == 243);
		assert(buff[1].syscall_num == 243);
		assert(buff[2].syscall_num == 2);
		//assert(buff[3].syscall_num == 1);
		assert(buff[0].syscall_res == -EINVAL);
		assert(buff[1].syscall_res == -EINVAL);
		assert(buff[2].syscall_res == fork_res1);
		//assert(buff[3].syscall_res == -EINVAL);
	}

	return result;
}

int zombie_check(){
	struct syscall_log_info buff[4];
	int pid= getpid();
	int result = 1;
	int i=0;
	int res;
	enable_syscalls_logging(pid,4); //243
	getpid(); //20
	int ppid = getppid(); //64
	enable_syscalls_logging(pid,4); //243
	enable_syscalls_logging(pid,4); //243
	int fork_res1 = fork(); //2

	if(fork_res1==0){
		int son1_pid=getpid();
		enable_syscalls_logging(son1_pid, 4);
		getppid();
		exit(0);
	}
	else{
		sleep(3); // wait till son dies and becomes zombie
		res = get_syscalls_log(fork_res1, 2, buff);
		assert(res==0);
		assert(buff[0].syscall_num==243);
		assert(buff[1].syscall_num==64);
		res = disable_syscalls_logging(fork_res1);
		assert(res==0);
		while(wait(NULL) != -1);
		res = enable_syscalls_logging(fork_res1, 10);
		assert(res==-1);
		assert(errno==ESRCH);
		res = disable_syscalls_logging(fork_res1);
		assert(res==-1);
		assert(errno==ESRCH);
		res = get_syscalls_log(fork_res1, 4, buff);
		assert(res==-1);
		assert(errno==ESRCH);

	}
	
	return result;
}


int main(){

	
	printf("Test begins\n");
	RUN_TEST(enable_disable_test)
	RUN_TEST(get_test)
	RUN_TEST(test_fork)
	RUN_TEST(fork_advanced)
	RUN_TEST(zombie_check)
	RUN_TEST(enable_no_disable) //MEMORY_CHECK

	printf("Test passed! :)\n");
	return 0;
}
