#include <linux/errno.h>
#include <linux/module.h>
#include <asm/semaphore.h>
#include <linux/fs.h> //for struct file_operations
#include <linux/kernel.h> //for printk()
#include <asm/uaccess.h> //for copy_from_user()
#include <linux/sched.h>
#include <linux/slab.h>		// for kmalloc
#include <linux/spinlock.h>
#include <linux/wait.h>
#include "open_example.h"
MODULE_LICENSE("GPL");

#define BUF_SIZE (4*1024) 	/* The buffer size, and also the longest message */
#define MAX_MINOR 2			//This module allows for 2 different kinds of devices, distinguished by different minor values
#define MY_MODULE "open_example"

struct file_operations fops0;
struct file_operations fops1;
/*
 * What's this?
 * Oh, that's right: On one hand, some functions in the module such as open refer to the file_operations struct.
 * On the other hand, that struct needs to know those function, because they are referenced within it.
 * Aha. So this is a forward declaration
 */

//Some other global variables:
int major;
int num_of_0;		//How many processes opened the device under minor=0
int num_of_1;		//How many processes opened the device under minor=1
int global_key=0;

wait_queue_head_t wq_0;
wait_queue_head_t wq_1;
//Possible necessary wait queues

spinlock_t counters_lock;
//This lock protects any reference to num_of_i

struct semaphore
	read_lock,
	write_lock,
	index_lock;
//This lock helps synchronize read/write operations between processes using devices of this module

typedef struct
{
	int private_key;
	int minor;
} device_private_data;
//This is private data, unique to EACH process that opened a device of this module

int my_open(struct inode* inode, struct file* filp)
{
	/*
	 * At this point the OS already allocated the stuct file, we just have to update
	 * its fields accordingly:
	 */
	int minor = MINOR(inode->i_rdev);;
	//In this example different minors have different functionality, so first we'll retrive the device's minor
	//Then we'll decide what file-operataions we want to load into the struct file
	if (minor == 0) {
		filp->f_op = &fops0;
	} else if (minor == 1) {
		filp->f_op = &fops1;
	}
	//Notice there is no real need to account for any different scenario - if I manage to open a device with a different minor than those
	//listed above then that means I had to make an appropriate device with mknod - which I know I didn't.

	/*
	 * Another service provided by the OS is allowing every process which opens the file to have a personal place
	 * to save private information nececssary for the devices functionality. This allows several processes to work
	 * with a single device, and is imperative for this exercise.
	 * This is what we refer to as "Private Data", and is in fact another field in the struct file - meaning that
	 * and open file object will have private data, even if I opened the same file twice in the same process.
	 * Since the size of private data is dynamic and depends on the device, the field itself is a pointer to the actual
	 * data - which must be allocated dynamically.
	 */
	filp->private_data = kmalloc(sizeof(device_private_data), GFP_KERNEL);
	if (filp->private_data == NULL)
	{
		//The edge case, in case kmalloc fails
		return -ENOMEM;
	}
	device_private_data* data = (device_private_data*)(filp->private_data);
	data->minor = minor;
	data->private_key = global_key++;
	//In the above exaxmple my private data holds the device's minor, and a place for a personal key for the opening process

	/*
	 * Let's say I also happen to have some much needed global variables in my device to keep track of the number of opened
	 * devices of each type.
	 * I obviously have to make sure these variables are mutually exclusive (since they are mutually accessible).
	 * Since I want to account for a multi-processor system, and since this update is relatively quick, a spinlock will suffice:
	 */
	spin_lock(counters_lock);
	if (minor == 0)
	{
		num_of_0++;
	}
	if (minor == 1)
	{
		num_of_1++;
	}
	spin_unlock(counters_lock);

	/*
	 * Now, we know that when we invoke open from our user code we're supposed to receive the file-descriptor (assuming all went well)
	 * The open we invoke from our user code is not directly the one which we wrote here: First the open-system-call is invoked, which
	 * does what the OS needs first, and that will then invoke this open operation. So long as our open function returns a non-negative
	 * value, then the system call will return the FD, meaning returning 0 should suffice at this point.
	 */
	return 0;
}


/*
 * Release is simply the mirror of open:
 * Any memory allocated during open should be freed here.
 * Any counters updated during open should (most likely) be decremented here.
 */
int my_release(struct inode* inode, struct file* filp)
{
	kfree(filp->private_data);
	int minor = MINOR(inode->i_rdev);;
	//Update counters:
	spin_lock(counters_lock);
	if (minor == 0)
	{
		if (!--num_of_0)
		//Note that it could be someone was waiting for this particular process. In this case I wanted to wake them up - this
		//is of course dependant on the exercise in question. This isn't a hint towards what you are supposed to do.
			wake_up_interruptible(&wq_1);
	}
	if (minor == 1)
	{
		if (!--num_of_1)
			wake_up_interruptible(&wq_0);
	}
	spin_unlock(counters_lock);
	return 0;
}

ssize_t my_read_0(struct file *filp, char *buf, size_t count, loff_t *f_pos) {
	printk("my_read_0\n");
	return 0;
}

ssize_t my_read_1(struct file *filp, char *buf, size_t count, loff_t *f_pos) {
	printk("my_read_1\n");
	return 0;
}

ssize_t my_write_0(struct file *filp, const char *buf, size_t count, loff_t *f_pos) {
	printk("my_write_0\n");
	return 0;
}

ssize_t my_write_1(struct file *filp, const char *buf, size_t count, loff_t *f_pos) {
	printk("my_write_1\n");
	return 0;
}

loff_t my_llseek(struct file *filp, loff_t a, int num) {
	return -ENOSYS;
}

int my_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg) {
	switch(cmd) {
		case EXAMPLE_SET_KEY:
			((device_private_data *)((filp)->private_data))->private_key = arg;
			break;

		default: return -ENOTTY;
	}
	return 0;
}

struct file_operations fops0 = {		//Alon: FOPS for minor=0
	.open=		my_open,
	.release=	my_release,
	.read=		my_read_0,
	.write=		my_write_0,
	.llseek=	my_llseek,
	.ioctl=		my_ioctl,
	.owner=		THIS_MODULE,
};

struct file_operations fops1 = {		//Alon: FOPS for minor=1
	.open=		my_open,
	.release=	my_release,
	.read=		my_read_1,
	.write=		my_write_1,
	.llseek=	my_llseek,
	.ioctl=		my_ioctl,
	.owner=		THIS_MODULE,
};
//Note how the function names themselves are irrelevant, so long as I bind them to the correct place in the file_ops struct
//(Their signatures are important, though)

//Module cleanup example
void cleanup_module(void)
{
	int ret = unregister_chrdev(major, MY_MODULE);
	if(ret < 0)
	{
		printk(KERN_ALERT "Error in unregister_chrdev: %d\n", ret);
	}
	return;
}


//Module initialization example:
int init_module(void)
{
	major = register_chrdev(0, MY_MODULE, &fops0);	//Dynamic allocation, module major number saved here for future reference
	//fops0 chosen as a default for this module, can and will be overloaded in the open function

	if (major < 0)
	{
		printk(KERN_ALERT "Registering char device failed with %d\n", major);
		return major;
	}

	num_of_0 = 0;
	num_of_1 = 0;
	spin_lock_init(&counters_lock);
	init_waitqueue_head(&wq_0);
	init_waitqueue_head(&wq_1);
	sema_init(&read_lock, 1);
	sema_init(&write_lock, 1);
	sema_init(&index_lock, 1);

	return 0;
}
