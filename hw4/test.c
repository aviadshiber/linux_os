#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <assert.h>
#include <time.h>
#include <sys/wait.h>
#include <stdlib.h>

#define MAX_LOG_ENTRIES (1024)
#define DEVICE_NAME ("systemcalls_logger")
#define DEVICE_PATH ("/dev/systemcalls_logger")
#define MAJOR_ID (254)
#define MAX_PROCS (20)

struct log {
   long sys_id;
   long time;
   long timeslice;
};


void sanity_test() {
   	/* Basic open & close */
	int fd = open(DEVICE_PATH, 0);	
	assert(-1 != fd);
	assert(0 == close(fd));

	/* Open error - just for sanity */
	fd = open("/dev/doesntexist", 0);	
	assert(-1 == fd);
}

void basic_log_test() {
   	pid_t forked = fork();
	if (forked == 0) {
	   int fd = open(DEVICE_PATH, 0);	
	   assert(-1 != fd);

	   struct log logs[MAX_LOG_ENTRIES];
	   for (int i = 0; 15 > i; i++) {
	      getpid(); // 20
	      time(NULL); // 13
	   }

	   // Do 30 getpid
	   for (int i = 0; 30 > i; i++) {
	      getpid(); // 20
	   }

	   assert(30 == read(fd, logs, 30)); //3

	   for (int i = 0; 30 > i; i+=2) {
		assert(logs[i].sys_id == 20);
		assert(logs[i + 1].sys_id == 13);
	   }

	   // Fill the whole log
	   for (int i = 0; MAX_LOG_ENTRIES - 61 > i; i++) {
	      time(NULL);//13
	   }		
		//20,13,....20,13|20,...20|3,13,13,|...13
	   assert(-1 != read(fd, logs, MAX_LOG_ENTRIES)); //3 will not enter to the logs since it is full

	   for (int i = 0; 30 > i; i+=2) {
		assert(logs[i].sys_id == 20);
		assert(logs[i + 1].sys_id == 13);
	   }   

	   for (int i = 30; 60 > i; i++) {
		assert(logs[i].sys_id == 20);
	   }
	
	   assert(logs[60].sys_id == 3);

	   for (int i = 61; MAX_LOG_ENTRIES > i; i++) {
		printf("log[%d].sys_id=%d\n",i,logs[i].sys_id);
		assert(logs[i].sys_id == 13);
	   }

	   /* Test if the driver returns consistent logs */
	   assert(-1 != read(fd, logs, MAX_LOG_ENTRIES));

	   for (int i = 0; 30 > i; i+=2) {
		assert(logs[i].sys_id == 20);
		assert(logs[i + 1].sys_id == 13);
	   }   

	   for (int i = 30; 60 > i; i++) {
		assert(logs[i].sys_id == 20);
	   }
	
	   assert(logs[60].sys_id == 3);

	   for (int i = 61; MAX_LOG_ENTRIES > i; i++) {
		assert(logs[i].sys_id == 13);
	   }

	   assert(0 == close(fd));
	   exit(0);
	} else if (forked > 0) {
	   wait(NULL);
	}
}

void log_over_the_limit_test() {
   	pid_t forked = fork();
	if (forked == 0) {
	   int fd = open(DEVICE_PATH, 0);	
	   assert(-1 != fd);

	   struct log logs[MAX_LOG_ENTRIES];
	   for (int i = 0; 15 > i; i++) {
	      getpid(); // 20
	      time(NULL); // 13
	   }

	   // Do 30 getpid
	   for (int i = 0; 30 > i; i++) {
	      getpid(); // 20
	   }

	   assert(30 == read(fd, logs, 30));

	   for (int i = 0; 30 > i; i+=2) {
		assert(logs[i].sys_id == 20);
		assert(logs[i + 1].sys_id == 13);
	   }

	   // Fill the whole log
	   for (int i = 0; MAX_LOG_ENTRIES - 61 > i; i++) {
	      time(NULL);
	   }		
	
	   for (int i = 0; MAX_LOG_ENTRIES > i; i++) {
	      getuid(); // 199 
	   }

	   assert(-1 != read(fd, logs, MAX_LOG_ENTRIES));

	   for (int i = 0; 30 > i; i+=2) {
		assert(logs[i].sys_id == 20);
		assert(logs[i + 1].sys_id == 13);
	   }   

	   for (int i = 30; 60 > i; i++) {
		assert(logs[i].sys_id == 20);
	   }
	
	   assert(logs[60].sys_id == 3);

	   for (int i = 61; MAX_LOG_ENTRIES > i; i++) {
		assert(logs[i].sys_id == 13);
	   }

	   assert(0 == close(fd));
	   exit(0);
	} else if (forked > 0) {	
	   wait(NULL);
	}
}



void log_multiple_procs() {
   for (int j = 0; MAX_PROCS > j; j++) {
	printf("[*] Forking %d process \n", j);
   	pid_t forked = fork();
	if (forked == 0) {
	   printf("[*] Proc %d is sleeping for 3 seconds\n", j);
	   sleep(3);
	   int fd = open(DEVICE_PATH, 0);	
	   assert(-1 != fd);

	   struct log logs[MAX_LOG_ENTRIES];
	   for (int i = 0; 15 > i; i++) {
	      getpid(); // 20
	      time(NULL); // 13
	   }

	   // Do 30 getpid
	   for (int i = 0; 30 > i; i++) {
	      getpid(); // 20
	   }

	   assert(30 == read(fd, logs, 30));

	   for (int i = 0; 30 > i; i+=2) {
		assert(logs[i].sys_id == 20);
		assert(logs[i + 1].sys_id == 13);
	   }

	   // Fill the whole log
	   for (int i = 0; MAX_LOG_ENTRIES - 61 > i; i++) {
	      time(NULL);
	   }		
	
	   for (int i = 0; MAX_LOG_ENTRIES > i; i++) {
	      getuid(); // 199 
	   }

	   assert(-1 != read(fd, logs, MAX_LOG_ENTRIES));

	   for (int i = 0; 30 > i; i+=2) {
		assert(logs[i].sys_id == 20);
		assert(logs[i + 1].sys_id == 13);
	   }   

	   for (int i = 30; 60 > i; i++) {
		assert(logs[i].sys_id == 20);
	   }
	
	   assert(logs[60].sys_id == 3);

	   for (int i = 61; MAX_LOG_ENTRIES > i; i++) {
		assert(logs[i].sys_id == 13);
	   }

	   assert(0 == close(fd));
	   exit(0);
	} 
   }

   for (int i = 0; MAX_PROCS > i; i++) {
      wait(NULL);
   }

}


int main(int argc, char * argv[]) {
   printf("[*] Test running\n");
   printf("[*] Sanity check \n");
   sanity_test();
   printf("[*] SUCCESS\n\n");

   printf("[*] Basic one process logging \n");
   basic_log_test();
   printf("[*] SUCCESS\n\n");

   printf("[*] Test no logs are written when passing the limit \n");
   log_over_the_limit_test();
   printf("[*] SUCCESS\n\n");
   
   printf("[*] Test logging with multiple processes \n");
   log_multiple_procs();
   printf("[*] SUCCESS\n\n");

   return 0;
}
