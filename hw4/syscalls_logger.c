#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <asm-i386/current.h>
#include <linux/vmalloc.h>
#include <linux/list.h>
#include <asm-i386/uaccess.h>

MODULE_LICENSE("GPL");

#define MY_MODULE "syscalls_logger"

#define store_idt(addr) \
	do { \
		__asm__ __volatile__ ( "sidt %0 \n" \
			: "=m" (addr) \
			: : "memory" ); \
    } while (0)

struct _descr { 
     
} __attribute__((__packed__));

typedef struct idtGate {
    
} idtGate;

asm (
    "patched_system_call: \n\t"
);
void patched_system_call();

int init_module(void) {
    return 0;
}
void cleanup_module(void) {
}
