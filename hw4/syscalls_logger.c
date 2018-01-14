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
#define get_higher_address(addr) ((addr >> 16)  &  0x0000FFFF)
#define get_lower_address(addr) (addr &  0x0000FFFF)
#define MAX_LOGGING_NUM (1024)
#define SYSCALL_INDEX (128)
#define store_idt(addr) \
	do { \
		__asm__ __volatile__ ( "sidt %0 \n" \
			: "=m" (addr) \
			: : "memory" ); \
    } while (0)

#define SAVE_ALL \
	"pushl %es;										\t\n" \
	"pushl %ds;										\t\n" \
	"pushl %ebp;									\t\n" \
	"pushl %edi;									\t\n" \
	"pushl %esi;									\t\n" \
	"pushl %edx;									\t\n" \
	"pushl %ecx;									\t\n" \
	"pushl %ebx;									\t\n" 

#define RESTORE_ALL \
	"popl %ebx;										\t\n" \
	"popl %ecx;										\t\n" \
	"popl %edx;										\t\n" \
	"popl %esi;										\t\n" \
	"popl %edi;										\t\n" \
	"popl %ebp;										\t\n" \
	"popl %ds;										\t\n" \
	"popl %es;										\t\n" 


struct _descr { 
   uint16_t size; // offset bits 0..15
   uint32_t base; // a code segment selector in GDT or LDT

} __attribute__((__packed__));
// http://wiki.osdev.org/Interrupt_Descriptor_Table
struct idtGate {
   uint16_t offset_1; // offset bits 0..15 (lower part)
   uint16_t selector; // a code segment selector in GDT or LDT
   uint8_t zero;      // unused, set to 0
   uint8_t type_attr; // type and attributes, see below
   uint16_t offset_2; // offset bits 16..31 (higher part)
} __attribute__((__packed__));

typedef struct idtGate idtGate;
uint32_t get_address_from_idt(idtGate* idt_gate){
	uint32_t higher_addr= idt_gate->offset_2;
	uint32_t lower_addr= idt_gate->offset_1;
	higher_addr<<=16;
	return  higher_addr | lower_addr;
}

void patched_system_call();
void add_log(int syscall_number);

#define CLEAR_INTERRUPTS "cli;\n\t"
#define SET_INTERRUPTS "sti;\n\t"


asm (".text\n\t"
    "patched_system_call:\n\t"
	CLEAR_INTERRUPTS
	SAVE_ALL
	"pushl %eax;\n\t" //#passing param1- syscall number
	"call add_log;\n\t" 
	"popl %eax;\n\t"
	RESTORE_ALL
	SET_INTERRUPTS
	"jmp *orig_syscall_addr;\n\t"
	
);

int my_open(struct inode* inode, struct file* filp);
int my_release(struct inode* inode, struct file* filp);
ssize_t my_read_0(struct file *filp, char *buf, size_t count, loff_t *f_pos);
ssize_t my_write_0(struct file *filp, const char *buf, size_t count, loff_t *f_pos);
loff_t my_llseek(struct file *filp, loff_t a, int num);
int my_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);


struct file_operations fops0 = {
	.open=		my_open,
	.release=	my_release,
	.read=		my_read_0,
	.write=		my_write_0,
	.llseek=	my_llseek,
	.ioctl=		my_ioctl,
	.owner=		THIS_MODULE,
};
typedef struct logger{
unsigned long syscall_num;
unsigned long jiffies;
unsigned long time_slice;
}logger;

typedef struct proc{
	pid_t pid;
	unsigned int size;
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
struct list_head head;
spinlock_t idt_lock;


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
void kfree_list(){
	list_t *pos;
	list_for_each(pos,&head){
		proc_list * proc=list_entry(pos, proc_list,list);
		list_del(&proc->list);
		kfree (proc);
	}
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
	proc_list* proc=find_proc(current->pid);
	if(!proc) return;
	if(MAX_LOGGING_NUM==proc->size) return;

	logger new_log;
	new_log.syscall_num=syscall_number;
	new_log.jiffies=jiffies;
	new_log.time_slice=current->time_slice;
	proc->logs[proc->size++]=new_log;
}

int init_module(void) {
	idtGate* interrupt_table;
	major = register_chrdev(0, MY_MODULE, &fops0); 
	if (major < 0){
		printk(KERN_ALERT "Registering char device failed with %d\n", major);
		return major;
	}
	spin_lock_init(&idt_lock);
	INIT_LIST_HEAD(&head);
	spin_lock(idt_lock);
			store_idt(idt_adress);
			interrupt_table=(idtGate*)idt_adress.base;
			syscalls_interrupt=hook_on(interrupt_table,SYSCALL_INDEX,(uint32_t)&patched_system_call);
	spin_unlock(idt_lock);
    return 0;
}
void cleanup_module(void) {
	int ret = unregister_chrdev(major, MY_MODULE);
	if(ret < 0){
		printk(KERN_ALERT "Error in unregister_chrdev: %d\n", ret);
	}
	//reverting syscall back to original
	spin_lock(idt_lock);
	update_idt_offset(syscalls_interrupt,orig_syscall_addr);
	spin_unlock(idt_lock);
	kfree_list();
}

int my_open(struct inode* inode, struct file* filp)
{

	proc_list* proc=find_proc(current->pid);
	if(NULL==proc){ //we could not find thr process, so we need to create it
		proc=kmalloc(sizeof(*proc),GFP_ATOMIC); //we need this allocation to be atomic
		if(!proc) return -ENOMEM;
		//proc->i=0;
		proc->size=0;
		proc->pid=current->pid;
		list_add(&(proc->list),&head);
	}else{
		return -EEXIST;
	}
	return 0; 
}

int my_release(struct inode* inode, struct file* filp){
	proc_list* proc=find_proc(current->pid);
	if(!proc) { //should never happen
		return -ESRCH;
	}
	list_del(&proc->list);
	kfree(proc);
	return 0;
}

ssize_t my_read_0(struct file *filp, char *buf, size_t count, loff_t *f_pos) {
	proc_list* proc;
	logger* logs;
	ssize_t unable_to_read_size=0;
	unsigned long read_size=0;
	unsigned long num_bytes_to_read;
	unsigned long remaining;
	if(!buf) return -ENOBUFS;
	proc=find_proc(current->pid);
	if(!proc) return -ESRCH;
	// remaining= (proc->size) - (proc->i);
	remaining = proc->size;
	if( count > remaining  ){
		count=remaining;
	}
	num_bytes_to_read=count*sizeof(logger);
	//logs=&proc->logs[proc->i]
	logs=proc->logs;
	unable_to_read_size=copy_to_user(buf,logs,num_bytes_to_read);
	read_size=(num_bytes_to_read-unable_to_read_size)/sizeof(logger);
	if(!read_size) return -EAGAIN;
	//(proc->i) += read_size; //TODO: MAYBE THIS IS NOT NDEEDED- WAIT FOR ANSWER
	return read_size;
}



ssize_t my_write_0(struct file *filp, const char *buf, size_t count, loff_t *f_pos) {
	return -ENOSYS;
}


loff_t my_llseek(struct file *filp, loff_t a, int num) {
	return -ENOSYS;
}

int my_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg) {
	return -ENOSYS;
}

