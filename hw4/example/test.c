#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sched.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include "template.h"

int main() {
	char buf[4];

	int fdTemplate = open("/dev/template", O_RDWR);
	int fdTemplate1 = open("/dev/template1", O_RDWR);
	write(fdTemplate, buf, 4);
	read(fdTemplate, buf, 4);
	write(fdTemplate1, buf, 4);
	read(fdTemplate1, buf, 4);
	close(fdTemplate);
	close(fdTemplate1);

	return 0;
}
