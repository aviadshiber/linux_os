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
#include "template.h"
MODULE_LICENSE("GPL");

#define BUF_SIZE (4*1024) 	/* The buffer size, and also the longest message */
#define MAX_MINOR 2			//This module allows for 2 different kinds of devices, distinguished by different minor values
#define MY_MODULE "template"

int major;

struct file_operations fops0;
struct file_operations fops1;

int my_open(struct inode* inode, struct file* filp)
{
	printk("my_open\n");
	int minor = MINOR(inode->i_rdev);;
	if (minor == 0) {
		filp->f_op = &fops0;
	} else if (minor == 1) {
		filp->f_op = &fops1;
	}
	return 0;
}

int my_release(struct inode* inode, struct file* filp)
{
	printk("my_release\n");
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
	printk("my_llseek\n");
	return -ENOSYS;
}

int my_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg) {
	printk("my_ioctl\n");
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

//Module cleanup example
void cleanup_module(void)
{
	printk("cleanup_module\n");
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
	printk("init_module\n");
	int major = register_chrdev(0, MY_MODULE, &fops0); 
	if (major < 0)
	{
		printk(KERN_ALERT "Registering char device failed with %d\n", major);
		return major;
	}


	return 0;
}
