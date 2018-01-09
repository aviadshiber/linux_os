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
typedef enum { false, true } bool;

#define EXAMPLE_MAGIC '4'
#define EXAMPLE_SET_KEY _IOW(EXAMPLE_MAGIC,0,int)

#define MY_MODULE "syscalls_logger"
#define get_higher_address(addr) ((addr >> 16)  &  0x0000FFFF)
#define get_lower_address(addr) (addr &  0x0000FFFF)
#define get_idt_entry(base,i)  (((idtGate *)base)[i])
#define set_idt_entry(base,i,data) (get_idt_entry(base,i)=data)
#define SYSCALL_INDEX 128
#define store_idt(addr) \
	do { \
		__asm__ __volatile__ ( "sidt %0 \n" \
			: "=m" (addr) \
			: : "memory" ); \
    } while (0)




// http://wiki.osdev.org/Interrupt_Descriptor_Table
struct _descr { 
   uint16_t size; // offset bits 0..15
   uint32_t base; // a code segment selector in GDT or LDT

} __attribute__((__packed__));

typedef struct idtGate {
   uint16_t offset_1; // offset bits 0..15 (lower part)
   uint16_t selector; // a code segment selector in GDT or LDT
   uint8_t zero;      // unused, set to 0
   uint8_t type_attr; // type and attributes, see below
   uint16_t offset_2; // offset bits 16..31 (higher part)
} idtGate;

uint32_t get_address_from_idt(idtGate idt_gate){
	uint32_t higher_addr= idt_gate.offset_2;
	uint32_t lower_addr= idt_gate.offset_1;
	higher_addr<<=16;
	return  higher_addr | lower_addr;
}

void patched_system_call();
void add_log();

asm (".text \n\t"
    "patched_system_call: \n\t"
	//push syscall result and syscall number
	// "pushl %eax;\n\t" //#passing param1- syscall number
	 "call add_log;\n\t"
	// "popl %eax;\n\t"
	 "jmp *orig_syscall_addr;\n\t"
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
typedef struct logger{
int syscall_num;
unsigned long jiffies;
int time_slice;
}logger;

typedef struct proc{
	pid_t pid;
	int fd;
	int size;
	logger logs[1024];
	list_t list;
} proc_list;

/**
 * GLOBAL VARS
 * */

int major;
struct file_operations fops0;
uint32_t orig_syscall_addr;
struct _descr idt_adress;
idtGate idt_gate;
bool log_enabled;
logger head;

void update_idt_offset(idtGate idt_gate,uint32_t addr){
	idt_gate.offset_2= get_higher_address(addr);
	idt_gate.offset_1= get_lower_address(addr);
}

void add_log(int syscall_number){
	printk("add log was called with syscall ");
	if(!log_enabled) return;
	logger *new_log=kmalloc(sizeof(*new_log),GFP_KERNEL);
	new_log->syscall_num=syscall_number;
	new_log->jiffies=jiffies;
	new_log->time_slice=current->time_slice;
	list_add(new_log,head);


//add log logic will be here
	
	//printk("add log was called with syscall num %d",syscall_number);
}

int init_module(void) {
	
	major = register_chrdev(0, MY_MODULE, &fops0); 
	if (major < 0){
		printk(KERN_ALERT "Registering char device failed with %d\n", major);
		return major;
	}
	log_enabled=0;
	INIT_LIST_HEAD(&head.list);
	printk("limit=%u ,base=%u, \n",(int)idt_adress.size,(unsigned int)idt_adress.base);
	store_idt(idt_adress);

	printk("limit=%u ,base=%u, \n",(int)idt_adress.size,(int)idt_adress.base);

	idt_gate= get_idt_entry(idt_adress.base,SYSCALL_INDEX);
	orig_syscall_addr=  get_address_from_idt(idt_gate);
	printk("orig addr:%u\n",orig_syscall_addr);
	update_idt_offset(idt_gate,(uint32_t)patched_system_call);
	set_idt_entry(idt_adress.base,SYSCALL_INDEX,idt_gate);


	//printk("offset_1=%d ,selector=%d, zero=%d,type_attr=%d,offest2=%d \n",desc->offset_1,desc->selector,desc->zero,desc->type_attr,desc->offset_2);

    return 0;
}
void cleanup_module(void) {
	//kfree(desc);
	int ret = unregister_chrdev(major, MY_MODULE);
	if(ret < 0){
		printk(KERN_ALERT "Error in unregister_chrdev: %d\n", ret);
	}
	//reverting syscall back to original
	update_idt_offset(idt_gate,orig_syscall_addr);
	set_idt_entry(idt_adress.base,SYSCALL_INDEX,idt_gate);

}

int my_open(struct inode* inode, struct file* filp)
{
	//TODO: SYNC THIS METHOD
	printk("my_open\n");
	//TODO: CHECK IF PROCESS PID EXIST IN LIST, IF NOT ADD NEW ONE
	list_t pos;
	list_t* l=head;
	int i=0;
	list_for_each(pos,l){
		if(list_entry(pos,proc_list,list)->pid == current->pid){
			return 0; //TODO: RETURN FD 
		}
		i++;
	}
	proc_list *new_node=kmalloc(sizeof(*new_node),GFP_KERNEL);
	new_node->pid=current->pid;
	new_node->size=0;
	list_add_tail(new_node,l);
	log_enabled=1;
	return 0; //TODO RETURN FD
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

