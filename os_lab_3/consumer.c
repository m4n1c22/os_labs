/**
	\file	:	consumer.c
	\author	: 	Team Mango
	\brief	:	Consumer LKM
				The kernel module which acts as a consumer in the 
				producer-consumer problem. The module loads in with a 
				rate of outflow and instance name.				

*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/workqueue.h>

MODULE_AUTHOR("Team Mango");
MODULE_DESCRIPTION("Lab Solution Task 2 Consumer LKM");
MODULE_LICENSE("GPL");

/** Parameters passed into the module*/

/** Rate of Inflow into the queue */
static int rate= 3;
/** Instance name of the producer in use of the given module */
static char * instance = "consumer_inst";

/** WorkQueue Object */
struct workqueue_struct *consumer_wq;

/** Internal Method for Consumption */
static void consume_item(void);

/** Creating a delayed_work object with the provided function handler.*/
static DECLARE_DELAYED_WORK(consumer, consume_item);

/** Flag for consumer unload operation*/
static int flag=0;

/** 
	Declaring the external module method fifo_read for performing 
	an internal call from consume_item()
*/
extern ssize_t fifo_read(char *buf, size_t count, loff_t *ppos);


/**
	Function Name : consume_item
	Function Type : Internal Method
	Description   : Method which is invoked to consume an item or remove
					an item from the FIFO Queue. Removal is performed
					by using the InterModule Communication with the 
					call of fifo_read(). Resultant string is passed as
					call by reference to the fifo_read() method call. 
					Number of bytes read is returned as a response.
*/
static void consume_item(void){
	
	/** Size of read buffer */
	size_t size=100;	
	/** Boolean status of the queue.*/
	bool q_status=false;	
	/** Number of the read bytes */
	ssize_t bytes_read=0;	
	/** Read Buffer*/
	char buffer [100] = {0} ;
	/** Pointer to read buffer */
	char *buff;
	/** Offset for the memory */
	loff_t offset;
	/** Pointer to offset */
	loff_t * off;
	
	/** Setting offset to starting point */
	offset=0;
	/** Buffer pointer set to Read Buffer */
	buff = buffer;
	/** offset pointer referenced with offset variable */
	off = &offset;
	
	/** 
		Performing the InterModule Communication of calling the 
		fifo_read(). bytes_read stores number of bytes read. buff is the
		read buffer which contains the read buffer from FIFO.
	*/
	bytes_read = fifo_read(buff,size,off);
	
	/** Condition check if bytes read is zero.*/
	if (bytes_read == 0)
		printk(KERN_ALERT "Consumer instance %s:we couldn't read\n",instance);
	else {		
		printk(KERN_ALERT "Consumer instance %s:we have read %d bytes\n",instance,(int)bytes_read);
		/** Outputing read item in the kernel log. Viewed by dmesg cmd.*/
		printk(KERN_ALERT "Consumer instance %s:consumed one item: %s\n",instance,buffer);
	}
	/** Condition check for consumer unloading flag set or not.*/
	if (flag==0){
		/** Setting the delayed work execution for the provided rate */
		q_status=queue_delayed_work(consumer_wq,&consumer,HZ/rate);
	}
	else 
		printk(KERN_ALERT "Consumer instance %s:consumer is unloading\n",instance);
}


/**
	Function Name : consumer_init_module
	Function Type : Module INIT
	Description   : Initialization method of the Kernel module. The
					method gets invoked when the kernel module is being
					inserted using the command insmod.
*/
static int __init consumer_init_module(void) {
	
	/** Boolean status of the queue.*/
	bool q_status=false;
	
	/** Logging the rate,message passed into the module.*/
	printk(KERN_ALERT "Consumer instance %s:Consumer loaded with rate:%d\n", instance,rate);
	
	/**
		Allocating the workqueue under the name consumer-wq and max 1
		active consumers.
	*/
	consumer_wq=alloc_workqueue("consumer-wq",WQ_UNBOUND,1);
	
	/** Condition check if the workqueue allocation failed */
	if (consumer_wq == NULL) {
		
		printk(KERN_ERR "Consumer instance %s ERROR:Consumer Workqueue couldn't allocated\n",instance);
		/** Memory Allocation Problem */
		return -ENOMEM;
	}
	else{
		/** Performing an internal call for consuming item */
		consume_item();
		/** Setting the delayed work execution for the provided rate */
		q_status= queue_delayed_work(consumer_wq,&consumer,HZ/rate);
	}
	printk(KERN_ALERT "Consumer instance %s:Scheduler closed, we are exiting now.\n",instance);
	
	/** Successfully executed the init module*/
	return 0;
}
/**
	Function Name : consumer_exit_module
	Function Type : Module EXIT
	Description   : Cleanup method of the Kernel module. The
                	method gets invoked when the kernel module is being
                 	removed using the command rmmod.
*/
static void __exit consumer_exit_module(void) {
	
	/** Signalling the producer module unloading */
	flag=1;
	/** Cancelling pending jobs in the Work Queue.*/
	cancel_delayed_work(&consumer);
	/** Removing all the pending jobs from the Work Queue*/
	flush_workqueue(consumer_wq);
	/** Deallocating the Work Queue */
	destroy_workqueue(consumer_wq);
	printk(KERN_ALERT "Consumer instance %s :Workqueue Destroyed\n",instance);
	printk(KERN_ALERT "Consumer instance %s :Producer module unloaded\n",instance);
}

/** Initializing the kernel module init with custom init method */
module_init(consumer_init_module);
/** Initializing the kernel module exit with custom cleanup method */
module_exit(consumer_exit_module);
/** Macro which deals with setting up of parameter passing to the module */
module_param(rate,int,0); /** Setting rate as a module param */
module_param(instance,charp,0); /** Setting instance as a module param */

