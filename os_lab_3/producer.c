/**
	\file	:	producer.c
	\author	: 	Team Mango
	\brief	:	Producer LKM
*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/workqueue.h>

MODULE_AUTHOR("Team Mango");
MODULE_DESCRIPTION("Lab Solution Task 2 Producer LKM");
MODULE_LICENSE("GPL");

/** Parameter passed into the module*/
static int rate= 5;

static char * msg = "default";

static char * instance = "producer_inst";

struct workqueue_struct *producer_wq;


static void produce_item(void);
static DECLARE_DELAYED_WORK(producer, produce_item);
static int flag = 0;


extern ssize_t fifo_write(const char *, size_t , loff_t *);

static int __init producer_init_module(void) {
	
	bool q_status=false;
	printk(KERN_ALERT "Producer instance %s:loaded with rate:%d\n", instance, rate);
	printk(KERN_ALERT "Producer instance %s:loaded with msg:%s\n", instance, msg);
	
	
	producer_wq = alloc_workqueue("pr-wq", WQ_UNBOUND,5);
	if (producer_wq== NULL){
		printk(KERN_ERR "Producer instance %s ERROR:Workqueue couldn't be allocated\n",instance);
		return -ENOMEM;
	}
	else {
		produce_item();
		q_status = queue_delayed_work(producer_wq, &producer, HZ/rate);
	}
	printk(KERN_ALERT "Producer instance %s:exported call returned in producer\n",instance);
	return 0;
}
static void produce_item(void){
	
	size_t size=0;
	ssize_t written=0;
	bool q_status=false;
	loff_t offset=0;
	loff_t * off;
	off = &offset;
	
	printk(KERN_ALERT "Producer instance %s:writing \t%s\n",instance,msg);
	size=strlen(msg);
	written = fifo_write(msg,size,off);
	if (written==0){
		printk(KERN_ALERT "Producer instance %s ERROR:we couldn't write\n",instance);
	}
	else printk(KERN_ALERT "Producer instance %s:we have written %d bytes of %s",instance,(int)written,msg);
		off +=written;
	
	if (flag == 0){
		q_status = queue_delayed_work(producer_wq, &producer, HZ/rate);
	}
	else
		printk(KERN_ALERT "Producer instance %s:flag caught, we are suppose to die now: \n",instance);
}

static void __exit producer_exit_module(void) {
	flag = 1;		// no more scheduling events now
	cancel_delayed_work(&producer);
	flush_workqueue(producer_wq);
	destroy_workqueue(producer_wq);
  printk(KERN_ALERT "Producer instance %s :Workqueue Destroyed\n",instance);
  printk(KERN_ALERT "Producer instance %s :Producer module unloaded\n",instance);
}
/** Initializing the kernel module init with custom init method */
module_init(producer_init_module);
/** Initializing the kernel module exit with custom cleanup method */
module_exit(producer_exit_module);
/** Macro which deals with setting up of parameter passing to the module */
module_param(rate,int,0);
module_param(msg,charp,0);
module_param(instance,charp,0);
