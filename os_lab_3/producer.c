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

static char * item = "default";
struct workqueue_struct *producer_wq;


static void produce_item(void);
static DECLARE_DELAYED_WORK(producer, produce_item);
static int flag = 0;
//struct timer_list pr_timer;

extern ssize_t fifo_write(const char *, size_t , loff_t *);
/*static void init_timer() {
	init_timer(pr_timer);
	pr_timer.function=repeater_function;
	
}*/
static int __init producer_init_module(void) {
	bool q_status=false;
  printk(KERN_ALERT "Producer loaded with rate:%d\n", rate);
  printk(KERN_ALERT "Producer loaded with item:%s\n", item);
	/*if (strlen(item)>9){
		printk(KERN_ERR "initialiser string size exceeds limit\n");
		return -EINVAL;
	}*/
  //printk(KERN_ALERT "timer is started\n");
	producer_wq = alloc_workqueue("pr-wq", WQ_UNBOUND,5);
	if (producer_wq== NULL){
		printk(KERN_ERR "workqueue couldn't allocated\n");
		return -ENOMEM;
	}
	else {
//		printk(KERN_ALERT "we will create item first\n");
		produce_item();
//		printk(KERN_ALERT "item created now scheduling next item\n");
		q_status = queue_delayed_work(producer_wq, &producer, HZ/rate);
//		printk(KERN_ALERT "item scheduled q_status is: %d\n",q_status);
	}
  printk(KERN_ALERT "exported call returned in producer\n");
  return 0;
}
static void produce_item(void){
	size_t size=0;
	ssize_t written=0;
	bool q_status=false;
	loff_t offset=0;
	loff_t * off;
	off = &offset;
//	printk(KERN_ALERT "Write call accessed\n");
	printk(KERN_ALERT "writing \t\t\t\t\t\t%s\n",item);
	size=strlen(item);
	//while (size > *off){
	//	printk(KERN_ALERT "we are at write call\n");
		written = fifo_write(item,size,off);
		if (written==0){
			printk(KERN_ALERT "we couldn't write\n");
		}
		else printk(KERN_ALERT "we have written %d bytes of %s",(int)written,item);
		off +=written;
	//}
	if (flag == 0){
		q_status = queue_delayed_work(producer_wq, &producer, HZ/rate);
//		printk(KERN_ALERT "another producer sched. q_status is: %d\n",q_status);
	}
	else
		printk(KERN_ALERT "flag caught, we are suppose to die now: \n");
}

static void __exit producer_exit_module(void) {
	flag = 1;		// no more scheduling events now
	cancel_delayed_work(&producer);
	flush_workqueue(producer_wq);
	destroy_workqueue(producer_wq);
  printk(KERN_ALERT "Workqueue Destroyed\n");
  printk(KERN_ALERT "Producer module unloaded\n");
}
/** Initializing the kernel module init with custom init method */
module_init(producer_init_module);
/** Initializing the kernel module exit with custom cleanup method */
module_exit(producer_exit_module);
/** Macro which deals with setting up of parameter passing to the module */
module_param(rate,int,0);
module_param(item,charp,0);
