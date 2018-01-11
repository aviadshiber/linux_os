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
#define MAX_LOGGING_NUM 1024
#define SYSCALL_INDEX 128
#define store_idt(addr) \
	do { \
		__asm__ __volatile__ ( "sidt %0 \n" \
			: "=m" (addr) \
			: : "memory" ); \
    } while (0)


#define SAVE_ALL \
	"pushl %es;\t\n" \
	"pushl %ds;\t\n" \
	"pushl %ebp;\t\n" \
	"pushl %edi;\t\n" \
	"pushl %esi;\t\n" \
	"pushl %edx;\t\n" \
	"pushl %ecx;\t\n" \
	"pushl %ebx;\t\n" 

#define RESTORE_ALL \
	"popl %ebx;\t\n" \
	"popl %ecx;\t\n" \
	"popl %edx;\t\n" \
	"popl %esi;\t\n" \
	"popl %edi;\t\n" \
	"popl %ebp;\t\n" \
	"popl %ds;\t\n" \
	"popl %es;\t\n" 


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

uint32_t get_address_from_idt(idtGate* idt_gate){
	uint32_t higher_addr= idt_gate->offset_2;
	uint32_t lower_addr= idt_gate->offset_1;
	higher_addr<<=16;
	return  higher_addr | lower_addr;
}

void patched_system_call();
void add_log(int syscall_number);

asm (".text \n\t"
    "patched_system_call: \n\t"
	"pushl %ebp;\n\t"
	"movl %esp,%ebp;\n\t"
	// "pushl %esi;\n\t" // not necessary (unused register)
	// "pushl %edi;\n\t" // not necessary (unused register)
	"pushl %eax;\n\t" //#passing param1- syscall number
	"call add_log;\n\t"
	"popl %eax;\n\t"
	// "popl %edi;\n\t"
	// "popl %esi\n\t"
	"popl %ebp\n\t"
	"jmp *orig_syscall_addr;\n\t"
	"ret;\n\t"
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
	unsigned int size;
	unsigned int i;
	logger logs[MAX_LOGGING_NUM];
	list_t list;
} proc_list;

/**
 * GLOBAL VARS
 * */

int major;
struct file_operations fops0;
uint32_t orig_syscall_addr;
struct _descr idt_adress;
idtGate* syscalls_interrupt;
bool log_enabled;
struct list_head head;

void update_idt_offset(idtGate* idt_gate,uint32_t addr){
	idt_gate->offset_2= get_higher_address(addr);
	idt_gate->offset_1= get_lower_address(addr);
}

idtGate* hook_on(idtGate * interrupt_table,int interrupt_index,uint32_t func_to_be_hooked_addr){
	idtGate* gate=interrupt_table+interrupt_index;
	orig_syscall_addr=  get_address_from_idt(gate);
	update_idt_offset(gate,func_to_be_hooked_addr);
	return gate;
}
proc_list* find_proc(pid_t pid){
	list_t *pos;
	list_for_each(pos,&head){
		if(list_entry(pos, proc_list,list)->pid==pid){
			return ((proc_list *)list_entry(pos, proc_list,list));
		}
	}
	return NULL;
}


void add_log(int syscall_number){
	//add log logic will be here
	//find the process in linked list
	proc_list* proc=find_proc(current->pid);
	if(!proc) return;
	if(MAX_LOGGING_NUM-1==proc->size) return;

	logger new_log = { .syscall_num=syscall_number, .jiffies=jiffies , .time_slice=current->time_slice  }
	proc->logger[proc->size++]=new_log;

}

int init_module(void) {
	idtGate* interrupt_table;
	major = register_chrdev(0, MY_MODULE, &fops0); 
	if (major < 0){
		printk(KERN_ALERT "Registering char device failed with %d\n", major);
		return major;
	}

	LIST_HEAD_INIT(head.list);

	store_idt(idt_adress);
	interrupt_table=(idtGate*)idt_adress.base;
	syscalls_interrupt=hook_on(interrupt_table,SYSCALL_INDEX,(uint32_t)&patched_system_call);
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
	update_idt_offset(syscalls_interrupt,orig_syscall_addr);

}

int my_open(struct inode* inode, struct file* filp)
{
	//TODO: SYNC THIS METHOD
	printk("my_open\n");
	proc_list* proc=find_proc(current->pid);
	if(NULL==proc){ //we could not find thr process, so we need to create it
		proc=kmalloc(sizeof(*proc),GFP_KERNEL);
		if(!proc) return -1;
		proc->i=0;
		proc->size=0;
		list_add(&proc->list,&head);
	}else{
		return -1; //what should we return if exist
	}

	return 0; 
}

int my_release(struct inode* inode, struct file* filp)
{
	printk("my_release\n");
	 proc_list* proc=find_proc(current->pid)
	if(!proc) return -1; //TODO: ASK WHAT SHOULD WE RETURN HERE?

	list_del_entry(&proc->list);
	kfree(proc);
	return 0;
}

ssize_t my_read_0(struct file *filp, char *buf, size_t count, loff_t *f_pos) {
	proc_list* proc;
	ssize_t read_size;
	unsigned int num_bytes_to_read;
	printk("my_read_0\n");
	// TODO: ASK- should we use proc->i or f_pos?
	if(!buf) return -1;
	if(count>size-proc->i){
		count=size-proc->i;
	}
	num_bytes_to_read=count*sizeof(logger);
	//TODO: ASK ABOUT LOGGER STRUCT SIZE
	proc=find_proc(current->pid);
	if(!proc) return -1; //should be 0 or -1?
	read_size= copy_to_user(buf,proc->logger[proc->i],num_bytes_to_read);
	if(read_size==num_bytes_to_read){ //TODO: ASK- do we need to proceed only if we read everything or by read_size?
		proc->i+=count;
	}
	return read_size;
}



ssize_t my_write_0(struct file *filp, const char *buf, size_t count, loff_t *f_pos) {
	printk("my_write_0\n");
	return -ENOSYS;
}


loff_t my_llseek(struct file *filp, loff_t a, int num) {
	printk("my_llseek\n");
	return -ENOSYS;
}

int my_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg) {
	printk("my_ioctl\n");
	return -ENOSYS;
}

