#ifndef _HW4_H_
#define _HW4_H_

#include <linux/ioctl.h>

#define EXAMPLE_MAGIC '4'
#define EXAMPLE_SET_KEY _IOW(EXAMPLE_MAGIC,0,int)

#endif
