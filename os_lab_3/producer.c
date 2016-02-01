/**
	\file	:	producer.c
	\author	: 	Team Mango
	\brief	:	Producer LKM. 
				The kernel module which acts as a producer in the 
				producer-consumer problem. The module loads in with a 
				rate of inflow, message and instance name.				
*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/workqueue.h>

MODULE_AUTHOR("Team Mango");
MODULE_DESCRIPTION("Lab Solution Task 2 Producer LKM");
MODULE_LICENSE("GPL");

/** Parameters passed into the module*/

/** Rate of Inflow into the queue */
static int rate= 5;
/** Message stored in the queue*/
static char * msg = "default";
/** Instance name of the producer in use of the given module */
static char * instance = "producer_inst";

/** WorkQueue Object */
struct workqueue_struct *producer_wq;

/** Internal Method for Production */
static void produce_item(void);

/** Creating a delayed_work object with the provided function handler.*/
static DECLARE_DELAYED_WORK(producer, produce_item);

/** Flag for production operation*/
static int flag = 0;

/** 
	Declaring the external module method fifo_write for performing 
	an internal call from produce_item()
*/
extern ssize_t fifo_write(const char *, size_t , loff_t *);

/**
	Function Name : produce_item
	Function Type : Internal Method
	Description   : Method which is invoked to produce an item or insert
					an item into the FIFO Queue. Insertion is performed
					by using the InterModule Communication with the 
					call of fifo_write(). Msg string is received as
					module input with size and offset are passed other
					possible params to fifo_write(). 
*/
static void produce_item(void){
	
	/** Size of write buffer */
	size_t size=0;
	/** Boolean status of the queue.*/
	bool q_status=false;
	/** Number of the bytes written */
	ssize_t bytes_written=0;
	/** Offset for the memory */
	loff_t offset=0;
	/** Pointer to offset */
	loff_t * off;
	
	/** offset pointer referenced with offset variable */
	off = &offset;
	
	printk(KERN_ALERT "Producer instance %s:writing \t%s\n",instance,msg);
	
	/** Size of bytes to be written as the passed message string.*/
	size=strlen(msg);
	
	/** 
		Performing the InterModule Communication of calling the 
		fifo_write(). bytes_written stores number of bytes written. msg
		contains the message which needs to be written into the FIFO.
	*/
	bytes_written = fifo_write(msg,size,off);
	
	/** Condition check if bytes read is zero.*/
	if (bytes_written==0){
		printk(KERN_ALERT "Producer instance %s ERROR:we couldn't write\n",instance);
	}
	else 
		printk(KERN_ALERT "Producer instance %s:we have written %d bytes of %s",instance,(int)bytes_written,msg);
	
	/** Condition check for producer unloading flag set or not.*/
	if (flag == 0){
		/** Setting the delayed work execution for the provided rate */
		q_status = queue_delayed_work(producer_wq, &producer, HZ/rate);
	}
	else
		printk(KERN_ALERT "Producer instance %s:producer is unloading\n",instance);
}

/**
	Function Name : producer_init_module
	Function Type : Module INIT
	Description   : Initialization method of the Kernel module. The
					method gets invoked when the kernel module is being
					inserted using the command insmod.
*/
static int __init producer_init_module(void) {
	
	/** Boolean status of the queue.*/
	bool q_status=false;
	
	/** Logging the rate,message passed into the module.*/
	printk(KERN_ALERT "Producer instance %s:loaded with rate:%d\n", instance, rate);
	printk(KERN_ALERT "Producer instance %s:loaded with msg:%s\n", instance, msg);
	
	/**
		Allocating the workqueue under the name producer-wq and max 5
		active producers.
	*/
	producer_wq = alloc_workqueue("producer-wq", WQ_UNBOUND,5);

	/** Condition check if the workqueue allocation failed */
	if (producer_wq== NULL){
		
		printk(KERN_ERR "Producer instance %s ERROR:Workqueue couldn't be allocated\n",instance);
		/** Memory Allocation Problem */
		return -ENOMEM;
	}
	else {
		/** Performing an internal call for producing item */
		produce_item();
		/** Setting the delayed work execution for the provided rate */
		q_status = queue_delayed_work(producer_wq, &producer, HZ/rate);
	}
	printk(KERN_ALERT "Producer instance %s:exported call returned in producer\n",instance);
	
	/** Successfully executed the init module*/
	return 0;
}


/**
	Function Name : producer_exit_module
	Function Type : Module EXIT
	Description   : Cleanup method of the Kernel module. The
                	method gets invoked when the kernel module is being
                 	removed using the command rmmod.
*/
static void __exit producer_exit_module(void) {
	
	/** Signalling the producer module unloading */
	flag = 1;
	/** Cancelling pending jobs in the Work Queue.*/
	cancel_delayed_work(&producer);
	/** Removing all the pending jobs from the Work Queue*/
	flush_workqueue(producer_wq);
	/** Deallocating the Work Queue */
	destroy_workqueue(producer_wq);
	printk(KERN_ALERT "Producer instance %s :Workqueue Destroyed\n",instance);
	printk(KERN_ALERT "Producer instance %s :Producer module unloaded\n",instance);
}

/** Initializing the kernel module init with custom init method */
module_init(producer_init_module);
/** Initializing the kernel module exit with custom cleanup method */
module_exit(producer_exit_module);
/** Macro which deals with setting up of parameter passing to the module */
module_param(rate,int,0); /** Setting the rate as a module param */
module_param(msg,charp,0); /** Setting the msg as a module param */
module_param(instance,charp,0);/** Setting the instance as a module param */
