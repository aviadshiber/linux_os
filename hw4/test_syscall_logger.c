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

	// int fdTemplate = open("/dev/template", O_RDWR);
	// write(fdTemplate, buf, 4);
	// read(fdTemplate, buf, 4);
	// close(fdTemplate);

	return 0;
}
