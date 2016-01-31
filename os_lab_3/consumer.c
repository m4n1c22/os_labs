/**
	\file	:	consumer.c
	\author	: 	Team Mango
	\brief	:	Consumer LKM
*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/workqueue.h>

MODULE_AUTHOR("Team Mango");
MODULE_DESCRIPTION("Lab Solution Task 2 Consumer LKM");
MODULE_LICENSE("GPL");

/** Parameter passed into the module*/
static int rate= 3;

static void consume_item(void);
struct workqueue_struct *consumer_wq;
static DECLARE_DELAYED_WORK(consumer, consume_item);
static int flag=0;
extern ssize_t fifo_read(char *buf, size_t count, loff_t *ppos);

static int __init consumer_init_module(void) {
	bool q_status=false;
  printk(KERN_ALERT "Consumer loaded with rate:%d\n", rate);
	consumer_wq=alloc_workqueue("cs-wq",WQ_UNBOUND,1);
	if (consumer_wq == NULL) {
		printk(KERN_ERR "Consumer Workqueue couldn't allocated\n");
		return -ENOMEM;
	}
	else{
//  	printk(KERN_ALERT "reading before starting workqueue\n");
		consume_item();
		q_status= queue_delayed_work(consumer_wq,&consumer,HZ/rate);
//	  printk(KERN_ALERT "Workqueue scheduled, status:\t%d\n",q_status);
	}
	printk(KERN_ALERT "Scheduler closed, we are exiting now.\n");
  return 0;
}
static void consume_item(void){
	size_t size=100;
	bool q_status=false;
	ssize_t read=0;
	char buffer [100] = {0} ;
	char *buff;
	loff_t offset;
	loff_t * off;
	offset=0;
	buff = buffer;
	off = &offset;
//	printk(KERN_ALERT "read call accessed\n");
	//size=strlen(MSG);
	//do {
//		printk(KERN_ALERT "we are at read call\n");
		read = fifo_read(buff,size,off);
		if (read == 0){
			printk(KERN_ALERT "we couldn't read\n");
		}
		else printk(KERN_ALERT "we have read %d bytes\n",(int)read);
		off +=read;
	//}while (read!=0 && buff[read]!='\0');
	printk(KERN_ALERT "consumed one item: \t\t\t\t%s\n",buffer);
	if (flag==0){
		q_status=queue_delayed_work(consumer_wq,&consumer,HZ/rate);
	//	printk(KERN_ALERT "Schduling next consumer\n");
	}
	else printk(KERN_ALERT "consumer is unloading\n");
	return 0;
}

static void __exit consumer_exit_module(void) {
	flag=1;
	cancel_delayed_work(&consumer);
	flush_workqueue(consumer_wq);
	destroy_workqueue(consumer_wq);
	printk(KERN_ALERT "workqueue destroyed and consumer module unloaded\n");
}

/** Initializing the kernel module init with custom init method */
module_init(consumer_init_module);
/** Initializing the kernel module exit with custom cleanup method */
module_exit(consumer_exit_module);
/** Macro which deals with setting up of parameter passing to the module */
module_param(rate,int,0);
