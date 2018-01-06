#include <linux/module.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <asm-i386/current.h>
#include <linux/vmalloc.h>
#include <linux/list.h>
#include <asm-i386/uaccess.h>

#include <linux/fs.h> //for struct file_operations
#include <asm/uaccess.h> //for copy_from_user()
#include <linux/kernel.h> //for printk()
#include <linux/sched.h>
#include <linux/errno.h>

MODULE_LICENSE("GPL");

#define EXAMPLE_MAGIC '4'
#define EXAMPLE_SET_KEY _IOW(EXAMPLE_MAGIC,0,int)

#define MY_MODULE "syscalls_logger"

#define store_idt(addr) \
	do { \
		__asm__ __volatile__ ( "sidt %0 \n" \
			: "=m" (addr) \
			: : "memory" ); \
    } while (0)


// http://wiki.osdev.org/Interrupt_Descriptor_Table
struct _descr { 
   uint16_t limit; // offset bits 0..15
   uint32_t address; // a code segment selector in GDT or LDT

} __attribute__((__packed__));

typedef struct idtGate {
    
} idtGate;

asm (
    "patched_system_call: \n\t"
);

int my_open(struct inode* inode, struct file* filp);
int my_release(struct inode* inode, struct file* filp);
ssize_t my_read_0(struct file *filp, char *buf, size_t count, loff_t *f_pos);
ssize_t my_write_0(struct file *filp, const char *buf, size_t count, loff_t *f_pos);
loff_t my_llseek(struct file *filp, loff_t a, int num);
int my_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);


struct file_operations fops0 = {		//Alon: FOPS for minor=0
	.open=		my_open,
	.release=	my_release,
	.read=		my_read_0,
	.write=		my_write_0,
	.llseek=	my_llseek,
	.ioctl=		my_ioctl,
	.owner=		THIS_MODULE,
};

int major;
struct file_operations fops0;


void patched_system_call();

struct _descr *desc;

int init_module(void) {
	desc=kmalloc(sizeof(*desc) ,GFP_KERNEL);
	if(!desc){
		printk(KERN_ALERT "Registering char device failed with allocating desc");
		return -1;
	}
	major = register_chrdev(0, MY_MODULE, &fops0); 
	if (major < 0){
		printk(KERN_ALERT "Registering char device failed with %d\n", major);
		return major;
	}
	store_idt(desc);

	//printk("limit=%d ,base=%d, \n",(int)desc->limit,(int)desc->address);

	//printk("offset_1=%d ,selector=%d, zero=%d,type_attr=%d,offest2=%d \n",desc->offset_1,desc->selector,desc->zero,desc->type_attr,desc->offset_2);

    return 0;
}
void cleanup_module(void) {
	kfree(desc);
	int ret = unregister_chrdev(major, MY_MODULE);
	if(ret < 0)
	{
		printk(KERN_ALERT "Error in unregister_chrdev: %d\n", ret);
	}
}

int my_open(struct inode* inode, struct file* filp)
{
	printk("my_open\n");

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



ssize_t my_write_0(struct file *filp, const char *buf, size_t count, loff_t *f_pos) {
	printk("my_write_0\n");
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
