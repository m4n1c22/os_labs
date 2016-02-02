/**
	\file	:	fifo.c
	\author	: 	Team Mango
	\brief	:	OS Lab-3 related to FIFO Queue implementation.
				The file contains a FIFO Queue implementation which can
				be accessed via driver at the location /dev/deeds_fifo
				in User Land and can be accessed across other LKMs via
				respective read and write methods. The FIFO queue stats
				can be verified via /proc/deeds_fifo_stats file.
*/

/** HEADER FILES*/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/time.h>

/** Module Author */
MODULE_AUTHOR("Team Mango");
/** Module Description */
MODULE_DESCRIPTION("FIFO Implementation");
/** Module License */
MODULE_LICENSE("GPL");

/** STANDARD MACROS */

#define BASE_10             10
#define END_OF_BUFF         '\0'

/** MEMORY ALLOCATION RELATED MACROS */

#define DEFAULT_MEM_SIZE    32

/** DEVICE RELATED MACROS */

#define FIFO_DEVICE        "deeds_fifo"
#define FIFO_DEVICE_NAME   "deeds_fifo"
#define FIFO_STATS         "deeds_fifo_stats"
#define MAJOR_NUM          250
#define MINOR_NUM_FIFO     0
#define CLASS_NAME         "fifo_class"

/** Proc FS File Object */
static struct proc_dir_entry *fifo_stats_file_entry;

/** Device Class Object */
static struct class*  fifoClass  = NULL;

/** Device Object */
static struct device* fifo = NULL;

/** MEMORY RELATED VARIABLES */
static int mem_alloc_size;

/** Flags */
static int finished_stats;
static int finished_fifo;

/** User Defined Data Item Structure */
struct data_item {
	unsigned int qid; /** Queue ID*/
	unsigned long long time; /** Creation time */
	char *msg; /** Custom message stored in the Queue Item */
};


/** FIFO_QUEUE */
static struct data_item *queue;

/** FIFO QUEUE Pointers*/
static int head;
static int tail;

/** Push and Pop counters*/
static int push;
static int pop;

/** Producer Consumer counters */
static int producer_ctr;
static int consumer_ctr;

/** User Level Producer Consumer counters */
static int user_level_producer_ctr;
static int user_level_consumer_ctr;

/** FIFO Statistics Variables */
static int num_items;
static int num_empty_slots;
static int fill_percentage;

/** Parameters passed to Module */
static int fifo_size;

/** Semaphores to producer & consumer problem */
static struct semaphore mutex;
static struct semaphore empty;
static struct semaphore full;

/** Semaphores for producer-consumer counters */
static struct semaphore producer_mutex;
static struct semaphore consumer_mutex;

/** Semaphores for user level producer-consumer counters */
static struct semaphore user_level_producer_mutex;
static struct semaphore user_level_consumer_mutex;

/** 
	Safe Incremental/Decremental Functions for all producers 
	and consumers counters.
*/
int producerInc(void);
int producerDec(void);
int consumerInc(void);
int consumerDec(void);

/** 
	Safe Incremental/Decremental Functions for user-level producers 
	and consumers counters.
*/
int userLevelProducerInc(void);
int userLevelProducerDec(void);
int userLevelConsumerInc(void);
int userLevelConsumerDec(void);

/** Basic FIFO Method Prototypes */
static ssize_t fifo_read(char *buf, size_t count, loff_t *ppos);
static ssize_t fifo_write(const char *buf, size_t count, loff_t *ppos);

/** Exporting Functions to be accessed in other LKMs */
EXPORT_SYMBOL_GPL(fifo_read);
EXPORT_SYMBOL_GPL(fifo_write);


/**
	Function Name : fifo_module_read
	Function Type : Kernel Callback Method
	Description   : Method is invoked whenever the fifo device files are
					read. This callback method is triggered when a read
					operation performed on the devices register to this
					file operation object. FIFO Devices are allocated.
*/
static ssize_t fifo_module_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
	/** return value storage variables */
	
	int ret; /** return value dealing with safe operation methods */
	int ret_fiforead; /** return value*/

	/** Performing a safe Increment of User Level Consumer counter */
	ret=userLevelConsumerInc();
	
	/** Verifying if Mutual exclusion inhibits the propagation.*/
	if (ret!=0) {
		/** Issue a restart of syscall which was supposed to be executed.*/
		return -ERESTARTSYS;
	}	
	
	/** Condition to check if the EOF is reached.*/
	if(finished_fifo) {
		/** Performing a safe Decrement of User Level Consumer counter*/
		ret=userLevelConsumerDec();
		/** Verifying if Mutual exclusion inhibits the propagation.*/
		if (ret!=0) {
			/** Issue a restart of syscall which was supposed to be executed.*/
			return -ERESTARTSYS;
		}
		/** Successful execution of read callback with EOF reached.*/
		return 0;
	}
	
	/** Flag set to Completed marking EOF.*/
	finished_fifo = 1;
	
	/** Calling the internal fifo_read() method call.*/
	ret_fiforead=fifo_read(buf,count,ppos);
	
	/** Performing a safe Decrement of User Level Consumer counter*/
	ret=userLevelConsumerDec();
	
	/** Verifying if Mutual exclusion inhibits the propagation.*/
	if (ret!=0) {
		/** Issue a restart of syscall which was supposed to be executed.*/
		return -ERESTARTSYS;
	}
	/** Successful execution of read callback with some bytes*/
	return ret_fiforead;
}

/**
	Function Name : fifo_read()
	Function Type : Kernel Internal Module Method
	Description   : Method is invoked internally by the 
					fifo_module_read() and externally from other kernel
					modules such as consumer modules loaded.
*/
static ssize_t fifo_read(char *buf, size_t count, loff_t *ppos)
{
	/** return value storage variables */
	
	int ret; /** return value dealing with safe operation methods */
	int ret_buf; /** return value after performing a write on read buffer. */

	printk(KERN_INFO "FIFO:Fifo module is being read.\n");

	/** Performing a safe Increment of Consumer counter*/		
	ret=consumerInc();

	/** Verifying if Mutual exclusion inhibits the propagation.*/
	if (ret!=0) {
		/** Issue a restart of syscall which was supposed to be executed.*/
		return -ERESTARTSYS;
	}
	
	/** 
		Condition to verify the down operation on the counting semaphore
		empty. Execution of a successful down operation in empty
		semaphore indicates the queue is safe from Underflow error.
	*/
	if (down_interruptible(&empty)){
		
		printk(KERN_ALERT "FIFO ERROR:Fifo Read access failed with Underflow.");
		/** Performing a safe Increment of Consumer counter*/		
		ret=consumerDec();
		/** Verifying if Mutual exclusion inhibits the propagation.*/
		if (ret!=0) {
			/** Issue a restart of syscall which was supposed to be executed.*/
			return -ERESTARTSYS;
		}
		/** Issue a restart of syscall which was supposed to be executed.*/
		return -ERESTARTSYS;
	}
	else {
		
		/** 
			Condition to verify the down operation on the binary semaphore
			mutex. Entry into a Mutually exclusive block is granted by
			having a successful lock with the mentioned semaphore.
			mutex semaphore provides a safe access to the following
			critical section.
		*/
		if(down_interruptible(&mutex)){
			
			printk(KERN_ALERT "FIFO ERROR:Mutual Exclusive position access failed from read module");
			/** 
				Performing an up operation on counting semaphore empty.
				Reseting the empty semaphore to original state. Thereby,
				performing transaction rollback.
			*/
			up(&empty);
			/** Performing a safe Increment of Consumer counter*/		
			ret=consumerDec();
			/** Verifying if Mutual exclusion inhibits the propagation.*/
			if (ret!=0) {
				/** Issue a restart of syscall which was supposed to be executed.*/
				return -ERESTARTSYS;
			}
			/** Issue a restart of syscall which was supposed to be executed.*/
			return -ERESTARTSYS;
		}
		else {
			
			/** 
				Loading the read buffer with data item contained in the
				head index. 
			*/
			ret_buf = sprintf(buf,"%d, %lld, %s",queue[head].qid, queue[head].time, queue[head].msg);
			/** Verifying if the read buffer is successfully loaded.*/
			if(ret_buf <0) {
				/** Memory allocation problem */
				return -ENOMEM;
			}
			
			/** 
				Executing memory deallocation of message string from
				the queue at the head index.
			*/
			kfree(queue[head].msg);
			/**
				Condition check for head pointing to maximum index
				location in the queue. If it is the case, change the
				head to point to the first allocation index indicating
				a circular queue operation.
			*/
			if(head==(mem_alloc_size-1)) {
				/** 
					head points to first physical index in circular
					queue.
				*/
				head = 0;
			}
			/** 
				Verifying if the head and tail pointers are in same
				index location. Invalidate the values therefore, making
				the queue empty.
			*/
			else if(head==tail) {
				/** Invalidating head index. */
				head = -1;
				/** Invalidating tail index. */
				tail = -1;
			}
			else {
				/** 
					Incrementing the head pointer to next location in
					the queue.
				*/
				head = head+1;
			}
			/** Incrementing the number of items popped or deleted. */
			pop = pop + 1;
			/** 
				Performing an up operation on mutex. Such an operation
				indicates the critical section is released for other
				processes/threads.
			*/
			up(&mutex);
			/** 
				Performing an up operation on counting semaphore full.
				Such an operation denotes a increase in empty slots in
				the queue.
			*/
			up(&full);
			
			/** Performing a safe Increment of Consumer counter*/		
			ret=consumerDec();
			/** Verifying if Mutual exclusion inhibits the propagation.*/
			if (ret!=0) {
				/** Issue a restart of syscall which was supposed to be executed.*/
				return -ERESTARTSYS;
			}

			/** Successful execution of read callback with some bytes*/
			return ret_buf;
		}
	}
}

/**
	Function Name : fifo_module_write
	Function Type : Kernel Callback Method
	Description   : Method is invoked whenever the fifo device files are
					written. This callback method is triggered when a
					write operation performed on the devices register to
					this file operation object.
*/
static ssize_t fifo_module_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{	
	/** return value storage variables */
	int ret; /** return value dealing with safe operation methods */
	int ret_fifowrite; /** return value*/

	/** Performing a safe Increment of User Level Producer counter */
	ret=userLevelProducerInc();
	
	/** Verifying if Mutual exclusion inhibits the propagation.*/
	if (ret!=0) {
		/** Issue a restart of syscall which was supposed to be executed.*/
		return -ERESTARTSYS;
	}
	
	/** Calling the internal fifo_write() method call.*/
	ret_fifowrite= fifo_write(buf,count,ppos);

	/** Performing a safe Decrement of User Level Producer counter*/
	ret=userLevelProducerDec();
	/** Verifying if Mutual exclusion inhibits the propagation.*/
	if (ret!=0) {
		/** Issue a restart of syscall which was supposed to be executed.*/
		return -ERESTARTSYS;
	}
	
	/** Successful execution of write callback*/
	return ret_fifowrite;
}

/**
	Function Name : fifo_write()
	Function Type : Kernel Internal Module Method
	Description   : Method is invoked internally by the 
					fifo_module_write() and externally from other kernel
					modules such as producer modules loaded.
*/
static ssize_t fifo_write(const char *buf, size_t count, loff_t *ppos)
{
	/** return value storage variables */
	int ret; /** return value dealing with safe operation methods */
	
	/** Timeval object for finding the current time */
	struct timeval timeval_obj;

	/** Performing a safe Increment of Producer counter */
	ret=producerInc();
	
	/** Verifying if Mutual exclusion inhibits the propagation.*/
	if (ret!=0) {
		/** Issue a restart of syscall which was supposed to be executed.*/
		return -ERESTARTSYS;
	}
	
	/** Checking if the Write buffer is empty or not. */
	if (( buf == NULL) ||  (*buf == 0)) {
		/** Performing a safe Decrement of Producer counter */
		ret=producerDec();
		/** Verifying if Mutual exclusion inhibits the propagation.*/
		if (ret!=0) {
			/** Issue a restart of syscall which was supposed to be executed.*/
			return -ERESTARTSYS;
		}
		/** Memory Allocation Problem */
	  	return -ENOMEM;
	}
	
	/** 
		Condition to verify the down operation on the counting semaphore
		full. Execution of a successful down operation in full
		semaphore indicates the queue is safe from Overflow error.
	*/
	if (down_interruptible(&full)){
		
		printk(KERN_ALERT "FIFO ERROR:Write access failed with Overflow error.");
		/** Performing a safe Decrement of Producer counter */
		ret=producerDec();
		/** Verifying if Mutual exclusion inhibits the propagation.*/
		if (ret!=0) {
			/** Issue a restart of syscall which was supposed to be executed.*/
			return -ERESTARTSYS;
		}
		/** Issue a restart of syscall which was supposed to be executed.*/
		return -ERESTARTSYS;
	}
	else {
		/** 
			Condition to verify the down operation on the binary semaphore
			mutex. Entry into a Mutually exclusive block is granted by
			having a successful lock with the mentioned semaphore.
			mutex semaphore provides a safe access to the following
			critical section.
		*/
		if (down_interruptible(&mutex)){
		
			printk(KERN_ALERT "FIFO ERROR: Mutual Exclusive access failed from write module");
	
			/** 
				Performing an up operation on counting semaphore full.
				Reseting the full semaphore to original state. Thereby,
				performing transaction rollback.
			*/
			up(&full);
			/** Performing a safe Decrement of Producer counter */
			ret=producerDec();
			/** Verifying if Mutual exclusion inhibits the propagation.*/
			if (ret!=0) {
				/** Issue a restart of syscall which was supposed to be executed.*/
				return -ERESTARTSYS;
			}
			/** Issue a restart of syscall which was supposed to be executed.*/
			return -ERESTARTSYS;
		}
		else {

			/** 
				Verifying if the head and tail pointers are invalid. 
				Indicates the presence of an empty queue. Set the head
				to first index location.
			*/
			if((head == -1)&&(tail==-1)) {
				/** Setting head point to first index.*/
				head = 0;
			}
			/**
				Condition check for tail pointing to maximum index
				location in the queue. If it is the case, change the
				tail to point to the invalid index. Since tail 
				will be incremented later on indicating a circular queue 
				operation.
			*/
			else if(tail==mem_alloc_size-1) {
				
				/** Invalidating the tail pointer */
				tail=-1;
			}
			
			/** Increment tail pointer to next indexed location */
			tail = tail+1;
			
			printk(KERN_INFO "FIFO:head = %d, tail = %d", head,tail);
			/** 
				Finding the current time using the system call
				gettimeofday()
			*/
			do_gettimeofday(&timeval_obj);
			
			/**
				Initializing the data item in the queue at the indexed 
				location tail. 
			*/
			/** Setting Queue ID*/
			queue[tail].qid = push; 
			/** Setting Creation Time as seconds */
			queue[tail].time = timeval_obj.tv_sec; 
			
			/** 
				Allocating memory for the message string with the 
				write buffer size. 
			*/
			queue[tail].msg = kmalloc(strlen(buf),GFP_KERNEL);
			/** Setting queue message with the incoming write buffer. */
			ret=sprintf(queue[tail].msg,buf);
			/** Verifying if the read buffer is successfully loaded.*/
			if(ret<0) {
				/** 
					Performing an up operation on mutex. Such an operation
					indicates the critical section is released for other
					processes/threads.
				*/	
				up(&mutex);
				/** 
					Performing an up operation on counting semaphore full.
					Reseting the full semaphore to original state. Thereby,
					performing transaction rollback.
				*/
				up(&full);
				
				/** Performing a safe Decrement of Producer counter */
				ret=producerDec();
				/** Verifying if Mutual exclusion inhibits the propagation.*/
				if (ret!=0) {
					/** Issue a restart of syscall which was supposed to be executed.*/
					return -ERESTARTSYS;
				}
				/** Erroneous return value */
				return ret;
			}
			printk(KERN_INFO "FIFO:Fifo module is being written.\n");

			/** 
				Incrementing the push variable for determining the
				total number of insertions in the FIFO Queue.
			*/
			push = push + 1;
		
			/** 
				Performing an up operation on mutex. Such an operation
				indicates the critical section is released for other
				processes/threads.
			*/
			up(&mutex);
			/** 
				Performing an up operation on counting semaphore empty.
				Such an operation denotes an decrease in empty slots in
				the queue.
			*/
			up(&empty);
	
			/** Performing a safe Decrement of Producer counter */
			ret=producerDec();
			/** Verifying if Mutual exclusion inhibits the propagation.*/
			if (ret!=0) {
				/** Issue a restart of syscall which was supposed to be executed.*/
				return -ERESTARTSYS;
			}
			/** Successful execution of write callback with buffer count.*/
			return count;
		}
	}
}


/**
	Function Name : fifo_module_open
	Function Type : Kernel Callback Method
	Description   : Method is invoked whenever the fifo device files are
					opened. This callback method is triggered when an
					open operation performed on the devices register to
					this file operation object. The open system call is
					invoked whenever an operation of read/write is
					performed on the device.
*/
static int fifo_module_open(struct inode * inode, struct file * file)
{
	/** Finished flag set to false indicating file is just opened.*/
	finished_fifo = 0;

	/** Successful execution of open callback */
	return 0;
}

/**
	Function Name : fifo_module_release
	Function Type : Kernel Callback Method
	Description   : Method is invoked whenever the fifo device files are
        	        closed. This callback method is triggered when a
                	close operation performed on the devices register to
					this file operation object.
*/
static int fifo_module_release(struct inode * inode, struct file * file)
{
	printk(KERN_INFO "FIFO:Fifo module is being released.\n");

	/** Successful execution of release callback */
	return 0;
}


/**
	Function Name : fifo_config_module_read
	Function Type : Kernel Callback Method
	Description   : Method is invoked whenever the fifo_config file is
					read. This callback method is triggered when a read
					operation performed on the /proc/fifo_config.
					The /proc/fifo_config contains the information about
					the memory info	of the FIFO Queue. Such as allocated
					size, free size and total size.	The fifo_config is a
					RDONLY file.
*/
static ssize_t fifo_stats_module_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
	/** return value storage variables */
	int ret;
	
	int latest_index,poppable_index;
	printk(KERN_INFO "FIFO:Fifo config module is being read.\n");


    /** Condition to check if EOF is reached. */
	if(!finished_stats){
		/** Flag set to Completed marking EOF.*/
		  finished_stats = 1;
		/** 
			Check if the head and tail indexes invalidated.
		*/
		if((head==-1) && (tail==-1)) {
			/** Set number of items as zero.*/
			num_items=0;
			/** Number of empty slots set as memory allocation size*/
			num_empty_slots=mem_alloc_size;
			/** Set fill percentage as zero */
			fill_percentage=0;
			
			/** Setting poppable and latest index */
			poppable_index=-1;
			latest_index=-1;
			
		}

		else {
			/** Setting number of items */
			num_items = push - pop;
			/** Number of empty slots */
			num_empty_slots = mem_alloc_size - num_items;
			/** Setting fill percentage */
			fill_percentage = (num_items*100)/mem_alloc_size;
			
			/** Setting poppable and latest index */
			poppable_index = queue[head].qid;
			latest_index = queue[tail].qid;
		}
		/** Setting the read buffer of the fifo_config module read*/
		ret = sprintf(buf,"Allocated Size: %d\nNumber of items stored: %d\nNumber of empty slots: %d\nPercentage of filled slots: %d \nNumber of insertions performed: %d\nNumber of removals performed: %d\nActive No.of Producers: %d\nActive No.of Consumers: %d\nActive User level Producers: %d\nActive User level Consumers: %d\nFirst Data Item Sequence No:%d\nLatest Data Item Sequence No:%d\n", mem_alloc_size,num_items,num_empty_slots,fill_percentage,push,pop,producer_ctr,consumer_ctr,user_level_producer_ctr,user_level_consumer_ctr,poppable_index,latest_index);

		/** Verifying if the load was successful or not.*/
		if(ret < 0) {
			/** Memory allocation problem */
			return -ENOMEM;
		}
		/** Successful execution of read callback with some bytes*/
		return ret;
	}
	/** Successful execution of read callback with EOF reached.*/
	return 0;
}

/**
	Function Name : fifo_config_module_open
	Function Type : Kernel Callback Method
	Description   : Method is invoked whenever the fifo_config file is
					opened. This callback method is triggered when an
					open operation performed on the	/proc/deeds_fifo_stats.
					This can be triggered on calls for read operations 
					on /proc/deeds_fifo_stats file.					
					The /proc/deeds_fifo_stats contains the information 
					about the memory info of the FIFO Queue. Such as 
					allocated size, free size and total size. The 
					deeds_fifo_stats is a RDONLY file.
*/
static int fifo_stats_module_open(struct inode * inode, struct file * file)
{
	printk(KERN_INFO "FIFO:Fifo config module is being opened.\n");

	/** Finished flag set to false indicating file is just opened.*/
	finished_stats = 0;
	/** Successful execution of open callback. */
	return 0;
}

/**
	Function Name : fifo_config_module_release
	Function Type : Kernel Callback Method
	Description   : Method is invoked whenever the fifo_config file is
					closed. This callback method is triggered when a
					close operation performed on the 
					/proc/deeds_fifo_stats. This can be triggered on 
					calls for RDONLY operations on 
					/proc/deeds_fifo_stats file.
					The /proc/deeds_fifo_stats contains the information 
					about the memory info of the FIFO Queue. Such as 
					allocated size, free size and total size.	
					The deeds_fifo_stats is a RDONLY file.
*/
static int fifo_stats_module_release(struct inode * inode, struct file * file)
{
	printk(KERN_INFO "FIFO:Fifo config module is being released.\n");
	/** Successful execution of release callback */
	return 0;
}

/**
    File Operations for handling the /proc/deeds_fifo_stats file accesses.
*/
static struct file_operations fifo_stats_module_fops = {
    .owner   =	THIS_MODULE,
    .read    =	fifo_stats_module_read,
    .open    =	fifo_stats_module_open,
    .release =	fifo_stats_module_release,
};

/**
    File Operations for handling the fifo device file accesses.
*/
static struct file_operations fifo_module_fops = {
    .owner   =	THIS_MODULE,
    .read    =	fifo_module_read,
    .write   =	fifo_module_write,
    .open    =	fifo_module_open,
    .release =	fifo_module_release,
};

/**
	Function Name : fifo_module_init
	Function Type : Module INIT
	Description   : Initialization method of the Kernel module. The
					method gets invoked when the kernel module is being
					inserted using the command insmod.
*/
static int __init fifo_module_init(void)
{
	int ret;
	printk(KERN_INFO "FIFO:FIFO module is being loaded.\n");

	/**
		Proc FS is created with RD ONLY permissions with name 
		deeds_fifo_stats
	*/
	fifo_stats_file_entry = proc_create(FIFO_STATS,0644,NULL,&fifo_stats_module_fops);

	/** Condition to verify if fifo_config creation was successful*/
	if(fifo_stats_file_entry == NULL) {
		printk(KERN_ALERT "FIFO ERROR: Could not initialize /proc/%s\n",FIFO_STATS);
		/** FILE CREATION PROBLEM */
		return -ENOMEM;
	}

	/**
	    Registering the Device with a major number as 250 and
	    configuring the file operations associated with it.
	*/
	ret = register_chrdev(MAJOR_NUM, FIFO_DEVICE, &fifo_module_fops);

	/** Condition code to check if the registration was successful.*/
	if (ret < 0) {
		printk(KERN_ALERT "FIFO ERROR: %s failed with %d\n",
		       "Sorry, registering the character device ", MAJOR_NUM);
		/** Registration error.*/
		return ret;
	}
	printk(KERN_INFO "FIFO:registered correctly with major number %d\n", MAJOR_NUM);

    /** Registering device class and associating devices with it.*/
    fifoClass = class_create(THIS_MODULE, CLASS_NAME);

	/** Condition check if the class creation was successful. */
	if (IS_ERR(fifoClass)){
		/** Unregister the device due to failed class creation. */
		unregister_chrdev(MAJOR_NUM, FIFO_DEVICE);
		printk(KERN_ALERT "FIFO ERROR:Failed to register device class\n");

		/** Class creation error.*/
		return PTR_ERR(fifoClass);
	}
	printk(KERN_INFO "FIFO: device class registered correctly\n");

	/**
		Registering the device driver for the provided device class. The
		device driver is associated with fifo0 with minor number as 0.
	*/
	fifo = device_create(fifoClass, NULL, MKDEV(MAJOR_NUM, MINOR_NUM_FIFO), NULL, FIFO_DEVICE_NAME);

	/** Condition for error verification during driver creation.*/
	if (IS_ERR(fifo)){

		/** Class destroyed associated with the device drivers.*/
		class_destroy(fifoClass);
		/** Unregister the device due to failed driver creation. */
		unregister_chrdev(MAJOR_NUM, FIFO_DEVICE);
		printk(KERN_ALERT "FIFO ERROR:Failed to create the device\n");
		/** Driver creation error.*/
		return PTR_ERR(fifo);
	}
	printk(KERN_INFO "FIFO:device class created correctly\n");

	/** 
		Setting queue memory allocation size variable to module param 
		FIFO size 
	*/
	mem_alloc_size = fifo_size;

	/** 
		Queue Allocated with the data item struct with a memory size of
		inputted FIFO size
	*/
	queue = (struct data_item*)kmalloc(mem_alloc_size*sizeof(struct data_item),GFP_KERNEL);

	/** Condition to check if the memory allocation was successful.*/
	if(!queue) {
		printk(KERN_ERR "FIFO ERROR:Memory allocation problem.\n");
		/** Memory allocation problem.*/
		return -ENOMEM;
	}

	/** Invalidating head and tail pointers of the queue */
	head = -1;
	tail = -1;

	/** Initializing push and pop counters*/
	push = 0;
	pop = 0;

	/** Initializing the semaphores */
	/** 
		Setting Mutex used for critical section inside fifo modules
		as 1. Indicates the critical section is free from use.
	*/
	sema_init(&mutex,1); 
	/**
		Initializing the counting semaphore empty to 0. Indicates FIFO
		Queue is empty. 
	*/
	sema_init(&empty,0);
	/**
		Initializing the counting semaphore empty to queue size. 
		Indicates FIFO Queue is empty. 
	*/
	sema_init(&full,mem_alloc_size);
	/** 
		Setting Mutex used for critical section with producer counters
		as 1. Indicates the critical section is free from use.
	*/	
	sema_init(&producer_mutex,1);
	/** 
		Setting Mutex used for critical section with consumer counters
		as 1. Indicates the critical section is free from use.
	*/
	sema_init(&consumer_mutex,1);
	/** 
		Setting Mutex used for critical section with user level producer
		counters as 1. Indicates the critical section is free from use.
	*/
	sema_init(&user_level_producer_mutex,1);
	/** 
		Setting Mutex used for critical section with user level consumer
		counters as 1. Indicates the critical section is free from use.
	*/
	sema_init(&user_level_consumer_mutex,1);
	
	/** Initialize producer consumer counters */
	producer_ctr=0;
	consumer_ctr=0;
	
	/** Initialize user level producer consumer counters */
	user_level_producer_ctr=0;
	user_level_consumer_ctr=0;

	/** Initializing FIFO stats variables */
	/** Setting number of items in queue as 0 */
	num_items = 0; 
	/** Initializing empty slots as FIFO size*/
	num_empty_slots = fifo_size; 
	/** Set percentage fill slots in queue as 0*/
	fill_percentage = 0;

	/** Successful execution of initialization method. */
	return 0;
}

/**
	Function Name : fifo_module_cleanup
	Function Type : Module EXIT
	Description   : Cleanup method of the Kernel module. The
                	method gets invoked when the kernel module is being
                 	removed using the command rmmod.
*/
static void __exit fifo_module_cleanup(void)
{

	printk(KERN_INFO "FIFO:FIFO module is being unloaded.\n");

	/** Removing the Proc FS entry. */
	proc_remove(fifo_stats_file_entry);
	/** Removing the device with minor number 0 => FIFO0 */
	device_destroy(fifoClass, MKDEV(MAJOR_NUM, MINOR_NUM_FIFO));
	/** Deregistering the class FIFO*/
	class_unregister(fifoClass);
	/** Deallocating the class FIFO*/
	class_destroy(fifoClass);
	/** Deregistering the character Device FIFO*/
	unregister_chrdev(MAJOR_NUM, FIFO_DEVICE);
	/** Deallocating the Queue */
	kfree(queue);
}


/**
	Function Name   :   queueAlloc
	Function Type   :   Custom
	Description     :   Method used to allocate/re-allocate the queue
                        from various module calls.
	Parameter       :   mem_size is used to allocate the memory for the
                        queue provided.
*/
int queueAlloc(int mem_size) {

	/** Condition check to see if the queue is already allocated.*/
	if(queue) {
		/** deallocating the queue.*/
		kfree(queue);
	}
	/** Allocating queue with the new mem_size.*/
	queue = (struct data_item*) kmalloc(mem_size*sizeof(struct data_item),GFP_KERNEL);
	/** Condition to see if the allocation was successful or not.*/
	if(!queue) {
		printk(KERN_ERR "FIFO ERROR:Memory allocation problem.\n");

		/** Memory allocation problem.*/
		return -ENOMEM;
	}
	/** Setting the memory allocation size */
	mem_alloc_size = mem_size;

	/** FIFO head & tail Set to invalidated locations. */
	head = -1;
	tail = -1;

	/** Successful execution of Queue Allocation method*/
	return 0;
}

/**
	Function Name   :   producerInc
	Function Type   :   Safe Increment
	Description     :   Method used to increment the producer counter
	 					variable but, in a safe way with the use of
						Mutual Exclusion property. Internally the method
						use the semaphore producer_mutex as a simple 
						mutual exclusion lock.
	Return	        :   IF the execution is successful -> 0
						ELSE-> -ERESTARTSYS
*/

int producerInc(void) {

	/** 
		Check if a safe down operation can be performed on the 
		producer_mutex. Indicates the critical section can be acquired
		by the accessing thread for performing the increment operation 
		on the producer counter.
	*/
	if (down_interruptible(&producer_mutex)){
		
		printk(KERN_ALERT "FIFO ERROR:Producer counter Increment Mutex Failed");
		/** Issue a restart of syscall which was supposed to be executed.*/
		return -ERESTARTSYS;
	}
	/** Increment the producer counter.*/
	producer_ctr++;
	/** 
		Perform an up operation on the producer_mutex. Indicates the
		critical section is being released for other threads to access.
	*/
	up(&producer_mutex);
	/** Successful execution of the producer increment operation.*/
	return 0;
}


/**
	Function Name   :   producerDec
	Function Type   :   Safe Decrement
	Description     :   Method used to decrement the producer counter
	 					variable but, in a safe way with the use of
						Mutual Exclusion property. Internally the method
						use the semaphore producer_mutex as a simple 
						mutual exclusion lock.
	Return	        :   IF the execution is successful -> 0
						ELSE-> -ERESTARTSYS
*/
int producerDec(void) {

	/** 
		Check if a safe down operation can be performed on the 
		producer_mutex. Indicates the critical section can be acquired
		by the accessing thread for performing the decrement operation 
		on the producer counter.
	*/
	if (down_interruptible(&producer_mutex)){
		
		printk(KERN_ALERT "FIFO ERROR:Producer counter Decrement Mutex Failed");
		/** Issue a restart of syscall which was supposed to be executed.*/
		return -ERESTARTSYS;
	}
	/** Decrement the producer counter.*/
	producer_ctr--;
	/** 
		Perform an up operation on the producer_mutex. Indicates the
		critical section is being released for other threads to access.
	*/
	up(&producer_mutex);
	/** Successful execution of the producer decrement operation.*/
	return 0;
}

/**
	Function Name   :   consumerInc
	Function Type   :   Safe Increment
	Description     :   Method used to increment the consumer counter
	 					variable but, in a safe way with the use of
						Mutual Exclusion property. Internally the method
						use the semaphore consumer_mutex as a simple 
						mutual exclusion lock.
	Return	        :   IF the execution is successful -> 0
						ELSE-> -ERESTARTSYS
*/
int consumerInc(void) {

	/** 
		Check if a safe down operation can be performed on the 
		consumer_mutex. Indicates the critical section can be acquired
		by the accessing thread for performing the increment operation 
		on the consumer counter.
	*/
	if (down_interruptible(&consumer_mutex)){
		
		printk(KERN_ALERT "FIFO ERROR:Consumer counter Increment Mutex Failed");
		/** Issue a restart of syscall which was supposed to be executed.*/
		return -ERESTARTSYS;
	}
	/** Increment the consumer counter.*/
	consumer_ctr++;
	/** 
		Perform an up operation on the consumer_mutex. Indicates the
		critical section is being released for other threads to access.
	*/
	up(&consumer_mutex);
	/** Successful execution of the consumer increment operation.*/
	return 0;
}

/**
	Function Name   :   consumerDec
	Function Type   :   Safe Decrement
	Description     :   Method used to decrement the consumer counter
	 					variable but, in a safe way with the use of
						Mutual Exclusion property. Internally the method
						use the semaphore consumer_mutex as a simple 
						mutual exclusion lock.
	Return	        :   IF the execution is successful -> 0
						ELSE-> -ERESTARTSYS
*/
int consumerDec(void) {
	
	/** 
		Check if a safe down operation can be performed on the 
		consumer_mutex. Indicates the critical section can be acquired
		by the accessing thread for performing the decrement operation 
		on the consumer counter.
	*/
	if (down_interruptible(&consumer_mutex)){
		
		printk(KERN_ALERT "FIFO ERROR:Consumer counter Decrement Mutex Failed");
		/** Issue a restart of syscall which was supposed to be executed.*/
		return -ERESTARTSYS;
	}
	
	/** Decrement the consumer counter.*/
	consumer_ctr--;
	/** 
		Perform an up operation on the consumer_mutex. Indicates the
		critical section is being released for other threads to access.
	*/
	up(&consumer_mutex);
	/** Successful execution of the consumer decrement operation.*/
	return 0;
}

/**
	Function Name   :   userLevelProducerInc
	Function Type   :   Safe Increment
	Description     :   Method used to increment the user level producer
						counter	variable but, in a safe way with the use
						of Mutual Exclusion property. Internally the 
						method use the semaphore 
						user_level_producer_mutex as a simple mutual 
						exclusion lock.
	Return	        :   IF the execution is successful -> 0
						ELSE-> -ERESTARTSYS
*/
int userLevelProducerInc(void) {

	/** 
		Check if a safe down operation can be performed on the 
		user_level_producer_mutex. Indicates the critical section can be 
		acquired by the accessing thread for performing the increment 
		operation on the user level producer counter.
	*/
	if (down_interruptible(&user_level_producer_mutex)){
		
		printk(KERN_ALERT "FIFO ERROR:Producer counter Increment Mutex Failed");
		/** Issue a restart of syscall which was supposed to be executed.*/
		return -ERESTARTSYS;
	}
	
	/** Increment the user level producer counter.*/
	user_level_producer_ctr++;
	/** 
		Perform an up operation on the user_level_producer_mutex. 
		Indicates the critical section is being released for other
		threads to access.
	*/
	up(&user_level_producer_mutex);
	/** 
		Successful execution of the user level producer increment 
		operation.
	*/
	return 0;
}

/**
	Function Name   :   userLevelProducerDec
	Function Type   :   Safe Decrement
	Description     :   Method used to decrement the user level producer
						counter	variable but, in a safe way with the use
						of Mutual Exclusion property. Internally the 
						method use the semaphore 
						user_level_producer_mutex as a simple mutual 
						exclusion lock.
	Return	        :   IF the execution is successful -> 0
						ELSE-> -ERESTARTSYS
*/
int userLevelProducerDec(void) {

	/** 
		Check if a safe down operation can be performed on the 
		user_level_producer_mutex. Indicates the critical section can be 
		acquired by the accessing thread for performing the decrement 
		operation on the user level producer counter.
	*/
	if (down_interruptible(&user_level_producer_mutex)){
		
		printk(KERN_ALERT "FIFO ERROR:Producer counter Decrement Mutex Failed");
		/** Issue a restart of syscall which was supposed to be executed.*/
		return -ERESTARTSYS;
	}
	
	/** Decrement the user level producer counter.*/
	user_level_producer_ctr--;

	/** 
		Perform an up operation on the user_level_producer_mutex. 
		Indicates the critical section is being released for other
		threads to access.
	*/
	up(&user_level_producer_mutex);
	/** 
		Successful execution of the user level producer decrement 
		operation.
	*/
	return 0;
}

/**
	Function Name   :   userLevelConsumerInc
	Function Type   :   Safe Increment
	Description     :   Method used to increment the user level consumer
						counter	variable but, in a safe way with the use
						of Mutual Exclusion property. Internally the 
						method use the semaphore 
						user_level_consumer_mutex as a simple mutual 
						exclusion lock.
	Return	        :   IF the execution is successful -> 0
						ELSE-> -ERESTARTSYS
*/
int userLevelConsumerInc(void) {

	/** 
		Check if a safe down operation can be performed on the 
		user_level_consumer_mutex. Indicates the critical section can be 
		acquired by the accessing thread for performing the increment 
		operation on the user level consumer counter.
	*/
	if (down_interruptible(&user_level_consumer_mutex)){
		
		printk(KERN_ALERT "FIFO ERROR:Consumer counter Increment Mutex Failed");
		/** Issue a restart of syscall which was supposed to be executed.*/
		return -ERESTARTSYS;
	}

	/** Increment the user level consumer counter.*/
	user_level_consumer_ctr++;

	/** 
		Perform an up operation on the user_level_consumer_mutex. 
		Indicates the critical section is being released for other
		threads to access.
	*/
	up(&user_level_consumer_mutex);
	/** 
		Successful execution of the user level consumer increment 
		operation.
	*/
	return 0;
}

/**
	Function Name   :   userLevelConsumerInc
	Function Type   :   Safe Increment
	Description     :   Method used to decrement the user level consumer
						counter	variable but, in a safe way with the use
						of Mutual Exclusion property. Internally the 
						method use the semaphore 
						user_level_consumer_mutex as a simple mutual 
						exclusion lock.
	Return	        :   IF the execution is successful -> 0
						ELSE-> -ERESTARTSYS
*/
int userLevelConsumerDec(void) {

	/** 
		Check if a safe down operation can be performed on the 
		user_level_consumer_mutex. Indicates the critical section can be 
		acquired by the accessing thread for performing the decrement 
		operation on the user level consumer counter.
	*/
	if (down_interruptible(&user_level_consumer_mutex)){
		
		printk(KERN_ALERT "FIFO ERROR:Consumer counter Decrement Mutex Failed");
		/** Issue a restart of syscall which was supposed to be executed.*/
		return -ERESTARTSYS;
	}

	/** Decrement the user level consumer counter.*/
	user_level_consumer_ctr--;

	/** 
		Perform an up operation on the user_level_consumer_mutex. 
		Indicates the critical section is being released for other
		threads to access.
	*/
	up(&user_level_consumer_mutex);
	/** 
		Successful execution of the user level consumer decrement 
		operation.
	*/
	return 0;
}
/** Initializing the kernel module init with custom init method */
module_init(fifo_module_init);
/** Initializing the kernel module exit with custom cleanup method */
module_exit(fifo_module_cleanup);
/** Macro which deals with setting up of parameter passing to the module */
module_param(fifo_size,int,0);
